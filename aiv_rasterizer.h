#include "aiv_math.h"

typedef struct Context
{
    int width;
    int height;

    unsigned char *framebuffer;

} Context_t;

typedef struct Vertex
{
    Vector3_t position;
    Vector3_t normal;
    Vector3_t color;

    int raster_x;
    int raster_y;
} Vertex_t;

Vertex_t Vertex_new(Vector3_t position);

typedef struct Triangle
{
    Vertex_t a;
    Vertex_t b;
    Vertex_t c;
} Triangle_t;

Triangle_t Triangle_new(Vertex_t a, Vertex_t b, Vertex_t c);

void rasterize(Context_t *ctx, Triangle_t *triangle);