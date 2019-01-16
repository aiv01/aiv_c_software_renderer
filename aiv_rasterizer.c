#include "aiv_rasterizer.h"
#include <string.h>
#include <stdlib.h>

void context_add_point(Context_t *ctx, Vector3_t v)
{
    ctx->vertices_count++;
    Vector3_t *new_vertices = realloc(ctx->vertices, sizeof(Vector3_t) * ctx->vertices_count);
    if (!new_vertices)
    {
        // TODO explode
        return;
    }
    ctx->vertices = new_vertices;
    ctx->vertices[ctx->vertices_count - 1] = v;
}

void context_add_normal(Context_t *ctx, Vector3_t v)
{
    ctx->normals_count++;
    Vector3_t *new_normals = realloc(ctx->normals, sizeof(Vector3_t) * ctx->normals_count);
    if (!new_normals)
    {
        // TODO explode
        return;
    }
    ctx->normals = new_normals;
    ctx->normals[ctx->normals_count - 1] = v;
}

void context_add_face(Context_t *ctx, Triangle_t t)
{
    ctx->faces_count++;
    Triangle_t *new_faces = realloc(ctx->faces, sizeof(Triangle_t) * ctx->faces_count);
    if (!new_faces)
    {
        // TODO explode
        return;
    }
    ctx->faces = new_faces;
    ctx->faces[ctx->faces_count - 1] = t;
}

Vertex_t Vertex_new(Vector3_t position)
{
    Vertex_t vertex;
    memset(&vertex, 0, sizeof(Vertex_t));
    vertex.position = position;
    return vertex;
}

Triangle_t Triangle_new(Vertex_t a, Vertex_t b, Vertex_t c)
{
    Triangle_t triangle = {.a = a, .b = b, .c = c};
    return triangle;
}

static void put_pixel(Context_t *ctx, int x, int y, float z, Vector3_t color)
{
    if (x < 0 || y < 0 || x >= ctx->width || y >= ctx->height)
        return;

    if (z < -1 || z > 1)
        return;

    unsigned char r = color.x * 255;
    unsigned char g = color.y * 255;
    unsigned char b = color.z * 255;

    int offset = ((y * ctx->width) + x) * 4;

    int z_offset = y * ctx->width + x;
    if (z > ctx->depth_buffer[z_offset])
        return;

    ctx->framebuffer[offset++] = r;
    ctx->framebuffer[offset++] = g;
    ctx->framebuffer[offset++] = b;
    ctx->framebuffer[offset] = 255;
    ctx->depth_buffer[z_offset] = z;

    ctx->put_pixel_counter++;
}

static void view_to_raster(Context_t *ctx, Vertex_t *vertex)
{
    float fov = (60.0 / 2) * (M_PI / 180.0);
    float znear = 0.01;
    float zfar = 20;
    float camera_distance = tan(fov);

    vertex->z = linear_convert(vertex->view_position.z, znear, zfar, -1, 1);

    float z = vertex->view_position.z;
    if (z == 0)
        z = znear;

    float projected_x = vertex->view_position.x / (camera_distance * z);
    float projected_y = vertex->view_position.y / (camera_distance * z);

    vertex->raster_x = (projected_x + 1) * (ctx->width * 0.5);
    vertex->raster_y = ctx->height - ((projected_y + 1) * (ctx->height * 0.5));
}

static void point_to_view(Context_t *ctx, Vertex_t *vertex)
{
    vertex->view_position = Vector3_sub(vertex->position, ctx->camera_position);
}

static void scanline(Context_t *ctx, int y, Vertex_t *left[2], Vertex_t *right[2])
{
    float gradientLeft = 1;
    if (left[0]->raster_y != left[1]->raster_y)
        gradientLeft = (float)(y - left[0]->raster_y) / (float)(left[1]->raster_y - left[0]->raster_y);

    float gradientRight = 1;
    if (right[0]->raster_y != right[1]->raster_y)
        gradientRight = (float)(y - right[0]->raster_y) / (float)(right[1]->raster_y - right[0]->raster_y);

    int start_x = lerp(left[0]->raster_x, left[1]->raster_x, gradientLeft);
    int end_x = lerp(right[0]->raster_x, right[1]->raster_x, gradientRight);
    int x;

    Vector3_t left_color = lerp3(left[0]->position, left[1]->position, gradientLeft);
    Vector3_t right_color = lerp3(right[0]->position, right[1]->position, gradientRight);

    Vector3_t left_normal = lerp3(left[0]->normal, left[1]->normal, gradientLeft);
    Vector3_t right_normal = lerp3(right[0]->normal, right[1]->normal, gradientRight);

    Vector3_t left_vertex = lerp3(left[0]->position, left[1]->position, gradientLeft);
    Vector3_t right_vertex = lerp3(right[0]->position, right[1]->position, gradientRight);

    float start_z = lerp(left[0]->z, left[1]->z, gradientLeft);
    float end_z = lerp(right[0]->z, right[1]->z, gradientRight);

    int clamped_start_x = clamp(start_x, -1, ctx->width);
    int clamped_end_x = clamp(end_x, clamped_start_x, ctx->width);
    for (x = clamped_start_x; x <= clamped_end_x; x++)
    {
        float gradient = 1;
        if (start_x != end_x)
            gradient = (float)(x - start_x) / (end_x - start_x);
        //Vector3_t color = lerp3(left_color, right_color, gradient);
        Vector3_t color = Vector3_new(1, 1, 1);
        float z = lerp(start_z, end_z, gradient);
        Vector3_t normal = Vector3_normalized(lerp3(left_normal, right_normal, gradient));
        Vector3_t vertex = lerp3(left_vertex, right_vertex, gradient);
        Vector3_t light_vertex = Vector3_normalized(Vector3_sub(ctx->light_position, vertex));
        float lambert = clampf(Vector3_dot(light_vertex, normal), 0, 1);
        put_pixel(ctx, x, y, z, Vector3_mul(Vector3_add(color, Vector3_new(0.1, 0.1, 0.1)), lambert));
    }
}

static int vertex_out_of_screen(Context_t *ctx, Vertex_t *vertex)
{
    if (vertex->raster_x < 0 || vertex->raster_x >= ctx->width ||
        vertex->raster_y < 0 || vertex->raster_y >= ctx->height || vertex->z < -1 || vertex->z > 1)
        return 1;
    return 0;
}

static int triangle_facing(Context_t *ctx, Triangle_t *triangle)
{
    Vector3_t edge0 = Vector3_sub(triangle->b.view_position, triangle->a.view_position);
    Vector3_t edge1 = Vector3_sub(triangle->c.view_position, triangle->a.view_position);
    Vector3_t normal = Vector3_cross(edge0, edge1);
    float dot = Vector3_dot(normal, Vector3_new(0, 0, -1));
    if (dot >= 0)
        return 0;
    return 1;
}

void rasterize(Context_t *ctx, Triangle_t *triangle)
{
    // camera space
    point_to_view(ctx, &triangle->a);
    point_to_view(ctx, &triangle->b);
    point_to_view(ctx, &triangle->c);

    // device space
    view_to_raster(ctx, &triangle->a);
    view_to_raster(ctx, &triangle->b);
    view_to_raster(ctx, &triangle->c);

    // out of screen ?
    if (vertex_out_of_screen(ctx, &triangle->a) && vertex_out_of_screen(ctx, &triangle->b) && vertex_out_of_screen(ctx, &triangle->c))
    {
        return;
    }

    if (!triangle_facing(ctx, triangle))
    {
        ctx->triangle_culled++;
        return;
    }

    ctx->triangle_processed++;

    Vertex_t *vertices[3] = {&triangle->a, &triangle->b, &triangle->c};
    if (vertices[0]->raster_y > vertices[1]->raster_y)
    {
        Vertex_t *tmp = vertices[1];
        vertices[1] = vertices[0];
        vertices[0] = tmp;
    }

    if (vertices[1]->raster_y > vertices[2]->raster_y)
    {
        Vertex_t *tmp = vertices[2];
        vertices[2] = vertices[1];
        vertices[1] = tmp;
    }

    if (vertices[0]->raster_y > vertices[1]->raster_y)
    {
        Vertex_t *tmp = vertices[1];
        vertices[1] = vertices[0];
        vertices[0] = tmp;
    }

    float slope_p0p1 = (float)(vertices[1]->raster_x - vertices[0]->raster_x) / (float)(vertices[1]->raster_y - vertices[0]->raster_y);
    float slope_p0p2 = (float)(vertices[2]->raster_x - vertices[0]->raster_x) / (float)(vertices[2]->raster_y - vertices[0]->raster_y);

    // default: p1 on the left
    Vertex_t *top_left[2] = {vertices[0], vertices[1]};
    Vertex_t *top_right[2] = {vertices[0], vertices[2]};

    Vertex_t *bottom_left[2] = {vertices[1], vertices[2]};
    Vertex_t *bottom_right[2] = {vertices[0], vertices[2]};

    // p1 on the right ?
    if (slope_p0p2 < slope_p0p1)
    {
        top_left[1] = vertices[2];
        top_right[1] = vertices[1];

        bottom_left[0] = vertices[0];
        bottom_right[0] = vertices[1];
    }

    int y;
    int start_y = clamp(vertices[0]->raster_y, -1, ctx->height);
    int end_y = clamp(vertices[1]->raster_y, vertices[0]->raster_y, ctx->height);

    for (y = start_y; y < end_y; y++)
    {
        scanline(ctx, y, top_left, top_right);
    }

    start_y = clamp(vertices[1]->raster_y, vertices[0]->raster_y, ctx->height);
    end_y = clamp(vertices[2]->raster_y, vertices[1]->raster_y, ctx->height);

    for (y = start_y; y <= end_y; y++)
    {
        scanline(ctx, y, bottom_left, bottom_right);
    }
}