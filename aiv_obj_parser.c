#include "aiv_obj_parser.h"

static int is_valid_float(char symbol)
{
    if (symbol == '.' || symbol == '-' || (symbol >= '0' && symbol <= '9'))
        return 1;
    return 0;
}

static void obj_parse_vertex(Context_t *ctx, char *token)
{

}

static void obj_parse_uv(Context_t *ctx, char *token)
{

}

static void obj_parse_normal(Context_t *ctx, char *token)
{

}

static void obj_parse_line(Context_t *ctx, char *token)
{
    size_t line_length = strlen(token);
    if (line_length < 2)
        return;

    if (token[0] == 'v' && token[1] == ' ')
        obj_parse_vertex(ctx, token + 2);

    else if (token[0] == 'v' && token[1] == 't')
        obj_parse_uv(ctx, token + 2);

    else if (token[0] == 'v' && token[1] == 'n')
        obj_parse_normal(ctx, token + 2);

    else if (token[0] == 'f' && token[1] == ' ')
        obj_parse_face(ctx, token + 2);
}

void obj_parse(Context_t *ctx, char *data, size_t data_len)
{
    ObjParser_t parser;
    int i;

    char *token = data;

    for (i = 0; i < data_len; i++)
    {
        char symbol = data[i];
        if (symbol == '\r' || symbol == '\n')
        {
            data[i] = 0;
            obj_parse_line(ctx, token);
            token = NULL;
            continue;
        }

        if (!token)
            token = &data[i];
    }
}