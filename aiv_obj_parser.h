#include "aiv_rasterizer.h"
#include <stdlib.h>

enum ObjParserState
{
    NOP,
    DATA,
    VERTEX1,
    VERTEX2,
    VERTEX3,
    NORMAL1,
    NORMAL2,
    NORMAL3,
    FACE,
    FACE1,
    FACE2,
    FACE3
};

typedef struct ObjParser
{
    enum ObjParserState state;
    char *token_start;
    Vector3_t current_vertex;
    Triangle_t current_triangle;
} ObjParser_t;

void obj_parse(Context_t *ctx, char *data, size_t data_len);