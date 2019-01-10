#include "aiv_math.h"
#include "aiv_rasterizer.h"

typedef struct Vertex
{
    Vector3_t position;
    Vector3_t normal;
    Vector3_t color;

    int raster_x;
    int raster_y;
} Vertex_t;

typedef struct Triangle
{
    Vertex_t a;
    Vertex_t b;
    Vertex_t c;
} Triangle_t;