#include "aiv_math.h"


typedef struct Vertex
{
    Vector3_t position;
    Vector3_t normal;
    Vector3_t color;

    Vector3_t view_position;

    int raster_x;
    int raster_y;
    float z;
} Vertex_t;

Vertex_t Vertex_new(Vector3_t position);

typedef struct Triangle
{
    Vertex_t a;
    Vertex_t b;
    Vertex_t c;
} Triangle_t;

Triangle_t Triangle_new(Vertex_t a, Vertex_t b, Vertex_t c);


typedef struct Context
{
    int width;
    int height;

    Vector3_t camera_position;

    unsigned char *framebuffer;

    float *depth_buffer;

    Vector3_t *vertices;
    size_t vertices_count;

    Vector3_t *normals;
    size_t normals_count;

    Triangle_t *faces;
    size_t faces_count;

    Vector3_t light_position;

    unsigned long long put_pixel_counter;
    unsigned long long triangle_processed;
    unsigned long long triangle_culled;

} Context_t;

void rasterize(Context_t *ctx, Triangle_t *triangle);

void context_add_point(Context_t *ctx, Vector3_t v);
void context_add_normal(Context_t *ctx, Vector3_t v);
void context_add_face(Context_t *ctx, Triangle_t t);