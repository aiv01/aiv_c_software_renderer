#include "aiv_obj_parser.h"

static int is_valid_float(char symbol)
{
    if (symbol == '.' || symbol == '-' || (symbol >= '0' && symbol <= '9'))
        return 1;
    return 0;
}

void obj_parse(Context_t *ctx, char *data, size_t data_len)
{
    size_t i;
    ObjParser_t parser;
    parser.state = NOP;
    parser.token_start = NULL;

    for (i = 0; i < data_len; i++)
    {

        char *symbol_ptr = &data[i];
        char symbol = *symbol_ptr;

        switch (parser.state)
        {
        case NOP:
            if (symbol == 'v')
            {
                parser.state = DATA;
                break;
            }
            if (symbol == 'f')
            {
                parser.state = FACE;
                break;
            }
            break;
        case DATA:
            if (symbol == ' ')
            {
                parser.state = VERTEX1;
                parser.token_start = NULL;
                break;
            }
            else if (symbol == 'n')
            {
                //parser.state = NORMAL1;
                parser.token_start = NULL;
                break;
            }
            parser.state = NOP;
            break;
        case FACE:
            if (symbol == ' ')
            {
                parser.state = FACE1;
                parser.token_start = NULL;
                break;
            }
            parser.state = NOP;
            break;
        case FACE1:
            if (!parser.token_start)
            {
                parser.token_start = symbol_ptr;
            }
            if (symbol == '/')
            {
                *symbol_ptr = 0;
                break;
            }
            if (symbol == '\n' || symbol == '\r')
            {
                parser.state = NOP;
                break;
            }
            if (!is_valid_float(symbol))
            {
                *symbol_ptr = 0;
                parser.current_triangle.a = Vertex_new(ctx->vertices[atoi(parser.token_start) - 1]);
                parser.state = FACE2;
                parser.token_start = NULL;
            }
            break;

        case FACE2:
            if (!parser.token_start)
            {
                parser.token_start = symbol_ptr;
            }
            if (symbol == '/')
            {
                *symbol_ptr = 0;
                break;
            }
            if (symbol == '\n' || symbol == '\r')
            {
                parser.state = NOP;
                break;
            }
            if (!is_valid_float(symbol))
            {
                *symbol_ptr = 0;
                parser.current_triangle.b = Vertex_new(ctx->vertices[atoi(parser.token_start) - 1]);
                parser.state = FACE3;
                parser.token_start = NULL;
            }
            break;
        case FACE3:
            if (!parser.token_start)
            {
                parser.token_start = symbol_ptr;
            }
            if (symbol == '/')
            {
                *symbol_ptr = 0;
                break;
            }
            if (symbol == '\n' || symbol == '\r' || !is_valid_float(symbol))
            {
                *symbol_ptr = 0;
                parser.current_triangle.c = Vertex_new(ctx->vertices[atoi(parser.token_start) - 1]);
                parser.state = NOP;
                context_add_face(ctx, parser.current_triangle);
            }
            break;
        case VERTEX1:
            if (!parser.token_start)
            {
                parser.token_start = symbol_ptr;
            }
            if (symbol == '\n' || symbol == '\r')
            {
                parser.state = NOP;
                break;
            }
            if (!is_valid_float(symbol))
            {
                *symbol_ptr = 0;
                parser.current_vertex.x = atof(parser.token_start);
                parser.state = VERTEX2;
                parser.token_start = NULL;
            }
            break;
        case VERTEX2:
            if (!parser.token_start)
            {
                parser.token_start = symbol_ptr;
            }
            if (symbol == '\n' || symbol == '\r')
            {
                parser.state = NOP;
                break;
            }
            if (!is_valid_float(symbol))
            {
                *symbol_ptr = 0;
                parser.current_vertex.y = atof(parser.token_start);
                parser.state = VERTEX3;
                parser.token_start = NULL;
            }
            break;
        case VERTEX3:
            if (!parser.token_start)
            {
                parser.token_start = symbol_ptr;
            }
            if (symbol == '\n' || symbol == '\r' || !is_valid_float(symbol))
            {
                *symbol_ptr = 0;
                parser.current_vertex.z = atof(parser.token_start) * -1;
                parser.state = NOP;
                context_add_point(ctx, parser.current_vertex);
            }
            break;
        }
    }
}