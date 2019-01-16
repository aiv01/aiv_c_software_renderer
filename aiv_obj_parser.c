#include "aiv_obj_parser.h"

static int is_valid_float(char symbol)
{
    if (symbol == '.' || symbol == '-' || (symbol >= '0' && symbol <= '9'))
        return 1;
    return 0;
}

static int is_valid_uint_or_slash(char symbol)
{
    if (symbol == '/' || (symbol >= '0' && symbol <= '9'))
        return 1;
    return 0;
}

static int is_valid_uint(char symbol)
{
    if ((symbol >= '0' && symbol <= '9'))
        return 1;
    return 0;
}

static void obj_parse_vertex(Context_t *ctx, char *token)
{
    Vector3_t v;
    size_t line_length = strlen(token);

    char *base = NULL;
    float *component[3] = {&v.x, &v.y, &v.z};
    int component_index = 0;
    size_t i;
    for (i = 0; i < line_length; i++)
    {
        int valid = is_valid_float(token[i]);

        if (valid && !base)
        {
            base = &token[i];
        }
        else if (!valid && base)
        {
            token[i] = 0;
            *component[component_index++] = atof(base);
            if (component_index == 3)
                break;
            base = NULL;
        }
    }

    if (component_index == 2)
    {
        *component[component_index] = atof(base);
    }

    v.z *= -1;
    context_add_point(ctx, v);
}

static void obj_parse_uv(Context_t *ctx, char *token)
{
}

static void obj_parse_normal(Context_t *ctx, char *token)
{
    Vector3_t v;
    size_t line_length = strlen(token);

    char *base = NULL;
    float *component[3] = {&v.x, &v.y, &v.z};
    int component_index = 0;
    size_t i;
    for (i = 0; i < line_length; i++)
    {
        int valid = is_valid_float(token[i]);

        if (valid && !base)
        {
            base = &token[i];
        }
        else if (!valid && base)
        {
            token[i] = 0;
            *component[component_index++] = atof(base);
            if (component_index == 3)
                break;
            base = NULL;
        }
    }

    if (component_index == 2)
    {
        *component[component_index] = atof(base);
    }

    v.z *= -1;
    context_add_normal(ctx, v);
}

static Vertex_t obj_parse_face_part(Context_t *ctx, char *token)
{
    Vertex_t vertex;
    memset(&vertex, 0, sizeof(vertex));
    size_t line_length = strlen(token);

    char *base = NULL;
    int component[3] = {0, 0, 0};
    int component_index = 0;
    size_t i;
    for (i = 0; i < line_length; i++)
    {
        int valid = is_valid_uint(token[i]);

        if (valid && !base)
        {
            base = &token[i];
        }
        else if (!valid && base)
        {
            token[i] = 0;
            component[component_index++] = atoi(base) - 1;
            if (component_index == 3)
                break;
            base = NULL;
        }
    }

    if (component_index == 2)
    {
        component[component_index] = atoi(base) - 1;
    }

    vertex.position = ctx->vertices[component[0]];
    vertex.normal = ctx->vertices[component[2]];
    return vertex;
}

static void obj_parse_face(Context_t *ctx, char *token)
{
    Triangle_t triangle;
    size_t line_length = strlen(token);

    char *base = NULL;
    Vertex_t *component[3] = {&triangle.a, &triangle.b, &triangle.c};
    int component_index = 0;
    size_t i;
    for (i = 0; i < line_length; i++)
    {
        int valid = is_valid_uint_or_slash(token[i]);

        if (valid && !base)
        {
            base = &token[i];
        }
        else if (!valid && base)
        {
            token[i] = 0;
            *component[component_index++] = obj_parse_face_part(ctx, base);
            if (component_index == 3)
                break;
            base = NULL;
        }
    }

    if (component_index == 2)
    {
        *component[component_index] = obj_parse_face_part(ctx, base);
    }

    context_add_face(ctx, triangle);
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
    size_t i;

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