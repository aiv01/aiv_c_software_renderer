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

static void put_pixel(Context_t *ctx, int x, int y, Vector3_t color)
{
    if (x < 0 || y < 0 || x >= ctx->width || y >= ctx->height)
        return;

    unsigned char r = (unsigned char)(color.x * 255.0);
    unsigned char g = (unsigned char)(color.y * 255.0);
    unsigned char b = (unsigned char)(color.z * 255.0);

    int offset = ((y * ctx->width) + x) * 4;

    ctx->framebuffer[offset++] = r;
    ctx->framebuffer[offset++] = g;
    ctx->framebuffer[offset++] = b;
    ctx->framebuffer[offset] = 255;
}

static void view_to_raster(Context_t *ctx, Vertex_t *vertex)
{
    float fov = (60.0 / 2) * (M_PI / 180.0);
    float znear = 0.01;
    float zfar = 20;
    float camera_distance = tan(fov);

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

    for (x = start_x; x <= end_x; x++)
    {
        put_pixel(ctx, x, y, Vector3_new(1, 0, 0));
    }
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