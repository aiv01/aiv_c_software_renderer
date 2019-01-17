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

void context_add_uv(Context_t *ctx, Vector3_t v)
{
    ctx->uvs_count++;
    Vector3_t *new_uvs = realloc(ctx->uvs, sizeof(Vector3_t) * ctx->uvs_count);
    if (!new_uvs)
    {
        // TODO explode
        return;
    }
    ctx->uvs = new_uvs;
    ctx->uvs[ctx->uvs_count - 1] = v;
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

    if (z < 0 || z > 1)
        return;

    unsigned char r = (unsigned char)(clampf(color.x, 0, 1) * 255.0);
    unsigned char g = (unsigned char)(clampf(color.y, 0, 1) * 255.0);
    unsigned char b = (unsigned char)(clampf(color.z, 0, 1) * 255.0);

    int offset = ((y * ctx->width) + x) * 4;

    int depth_offset = y * ctx->width + x;

    if (ctx->depth_buffer[depth_offset] < z)
    {
        return;
    }

    ctx->framebuffer[offset++] = r;
    ctx->framebuffer[offset++] = g;
    ctx->framebuffer[offset++] = b;
    ctx->framebuffer[offset] = 255;

    ctx->depth_buffer[depth_offset] = z;

    ctx->put_pixel_counter++;
}

static void view_to_raster(Context_t *ctx, Vertex_t *vertex)
{
    float fov = (60.0 / 2) * (M_PI / 180.0);
    float znear = 0.01;
    float zfar = 10;
    float camera_distance = tan(fov);

    float z = vertex->view_position.z;
    if (z == 0)
        z = znear;

    vertex->z = linear_convert(z, znear, zfar, 0, 1);

    float projected_x = vertex->view_position.x / (camera_distance * z);
    float projected_y = vertex->view_position.y / (camera_distance * z);

    vertex->raster_x = (projected_x + 1) * (ctx->width * 0.5);
    vertex->raster_y = ctx->height - ((projected_y + 1) * (ctx->height * 0.5));
}

static void point_to_view(Context_t *ctx, Vertex_t *vertex)
{
    vertex->world_position = Vector3_roty(vertex->position, ctx->roty);
    vertex->world_normal = Vector3_roty(vertex->normal, ctx->roty);
    vertex->view_position = Vector3_sub(vertex->world_position, ctx->camera_position);
}

static Vector3_t get_texel(Context_t *ctx, Vector3_t uv)
{
    int x = (int)linear_convert(uv.x, 0, 1, 0, (ctx->texture_width - 1)) % (ctx->texture_width - 1);
    int y = (int)linear_convert(uv.y, 0, 1, (ctx->texture_height - 1), 0) % (ctx->texture_height - 1);

    int offset = (y * ctx->texture_width + x) * 3;

    unsigned char r = ctx->texture[offset];
    unsigned char g = ctx->texture[offset + 1];
    unsigned char b = ctx->texture[offset + 2];

    return Vector3_new(((float)r) / 255.0, ((float)g) / 255.0, ((float)b) / 255.0);
}

static void scanline(Context_t *ctx, int y, Vertex_t *left[2], Vertex_t *right[2])
{
    float gradientLeft = 1;
    if (left[0]->raster_y != left[1]->raster_y)
        gradientLeft = (float)(y - left[0]->raster_y) / (float)(left[1]->raster_y - left[0]->raster_y);

    float gradientRight = 1;
    if (right[0]->raster_y != right[1]->raster_y)
        gradientRight = (float)(y - right[0]->raster_y) / (float)(right[1]->raster_y - right[0]->raster_y);

    Vector3_t leftNormal = lerp3(left[0]->world_normal, left[1]->world_normal, gradientLeft);
    Vector3_t rightNormal = lerp3(right[0]->world_normal, right[1]->world_normal, gradientRight);

    Vector3_t leftPixel = lerp3(left[0]->world_position, left[1]->world_position, gradientLeft);
    Vector3_t rightPixel = lerp3(right[0]->world_position, right[1]->world_position, gradientRight);

    Vector3_t leftUV = lerp3(left[0]->uv, left[1]->uv, gradientLeft);
    Vector3_t rightUV = lerp3(right[0]->uv, right[1]->uv, gradientRight);

    float left_z = lerp(left[0]->z, left[1]->z, gradientLeft);
    float right_z = lerp(right[0]->z, right[1]->z, gradientRight);

    int start_x = lerp(left[0]->raster_x, left[1]->raster_x, gradientLeft);
    int end_x = lerp(right[0]->raster_x, right[1]->raster_x, gradientRight);
    int x;

    int clamped_start_x = clamp(start_x, -1, ctx->width);
    int clamped_end_x = clamp(end_x, clamped_start_x, ctx->width);

    for (x = clamped_start_x; x <= clamped_end_x; x++)
    {
        float gradient = 1;
        if (start_x != end_x)
            gradient = (float)(x - start_x) / (float)(end_x - start_x);

        Vector3_t uv = lerp3(leftUV, rightUV, gradient);

        //Vector3_t color = Vector3_new(0.5, 0.5, 0.5); //lerp3(leftColor, rightColor, gradient);

        Vector3_t color = get_texel(ctx, uv);

        Vector3_t pixel_normal = Vector3_normalized(lerp3(leftNormal, rightNormal, gradient));
        Vector3_t pixel_light_vector = Vector3_new(0, 0, -1);
        float lambert = clampf(Vector3_dot(pixel_normal, pixel_light_vector), 0, 1);

        Vector3_t pixel_position = lerp3(leftPixel, rightPixel, gradient);
        Vector3_t point_light_pixel_position = Vector3_normalized(Vector3_sub(ctx->point_light_position, pixel_position));
        float point_lambert = clampf(Vector3_dot(pixel_normal, point_light_pixel_position), 0, 1);

        float z = lerp(left_z, right_z, gradient);
        Vector3_t diffuse_directional = Vector3_mul(color, lambert);
        Vector3_t diffuse_point_light = Vector3_mul(color, point_lambert);

        //Vector3_t diffuse = Vector3_add(diffuse_directional, diffuse_point_light);
        Vector3_t diffuse = diffuse_point_light;
        Vector3_t ambient = Vector3_new(0.2, 0.2, 0.2);
        Vector3_t final_color = Vector3_add(diffuse, ambient);
        put_pixel(ctx, x, y, z, final_color);
    }
}

static int triangle_in_screen(Context_t *ctx, Triangle_t *triangle)
{
    if (triangle->a.raster_y >= ctx->height && triangle->b.raster_y >= ctx->height && triangle->c.raster_y >= ctx->height)
        return 0;

    if (triangle->a.raster_y < 0 && triangle->b.raster_y < 0 && triangle->c.raster_y < 0)
        return 0;

    if (triangle->a.raster_x >= ctx->width && triangle->b.raster_x >= ctx->width && triangle->c.raster_x >= ctx->width)
        return 0;

    if (triangle->a.raster_x < 0 && triangle->b.raster_x < 0 && triangle->c.raster_x < 0)
        return 0;

    if (triangle->a.z > 1 && triangle->b.z > 1 && triangle->c.z > 1)
        return 0;

    if (triangle->a.z < 0 || triangle->b.z < 0 || triangle->c.z < 0)
        return 0;

    return 1;
}

static int triangle_is_facing_camera(Context_t *ctx, Triangle_t *triangle)
{
    Vector3_t a = triangle->a.world_position;
    Vector3_t b = triangle->b.world_position;
    Vector3_t c = triangle->c.world_position;

    Vector3_t edge0 = Vector3_sub(b, a);
    Vector3_t edge1 = Vector3_sub(c, a);

    Vector3_t fwd = Vector3_normalized(Vector3_cross(edge0, edge1));
    Vector3_t camera_vector = Vector3_normalized(Vector3_sub(a, ctx->camera_position));

    if (Vector3_dot(fwd, camera_vector) >= 0)
        return 1;
    return 0;
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

    if (!triangle_in_screen(ctx, triangle))
        return;

    if (!triangle_is_facing_camera(ctx, triangle))
        return;

    ctx->triangle_counter++;

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