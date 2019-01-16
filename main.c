#include "aiv_obj_parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>

#define triangle(x0, y0, z0, x1, y1, z1, x2, y2, z2) \
    Triangle_new(                                    \
        Vertex_new(Vector3_new(x0, y0, z0)),         \
        Vertex_new(Vector3_new(x1, y1, z1)),         \
        Vertex_new(Vector3_new(x2, y2, z2)))

int main(int argc, char **argv)
{
    Context_t ctx;
    memset(&ctx, 0, sizeof(Context_t));
    ctx.width = 600;
    ctx.height = 600;

    ctx.camera_position = Vector3_new(0, 1.5, -5);

    ctx.light_position = Vector3_new(0, 4, -10);

    ctx.framebuffer = NULL;

    ctx.depth_buffer = malloc(ctx.width * ctx.height * sizeof(float));
    memset(ctx.depth_buffer, 0, ctx.width * ctx.height * sizeof(float));

    FILE *obj = fopen(argv[1], "rb");
    if (!obj)
        return -1;

    fseek(obj, 0, SEEK_END);
    long obj_size = ftell(obj);
    fseek(obj, 0, SEEK_SET);

    char *obj_data = malloc(obj_size);
    if (!obj_data)
        return -1;

    fread(obj_data, 1, obj_size, obj);

    obj_parse(&ctx, obj_data, obj_size);

    size_t i;
    for (i = 0; i < ctx.vertices_count; i++)
    {
        //Vector3_print(ctx.vertices[i]);
    }

    Triangle_t triangle = triangle(
        0, 1, 0,
        0.8, 0, 0,
        -1, -0.5, 0);

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 600, 600, 0);
    if (!window)
        return -1;

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
        return -1;

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 600, 600);
    if (!texture)
        return -1;

    for (;;)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                return 0;
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_w)
                {
                    ctx.camera_position = Vector3_add(ctx.camera_position, Vector3_new(0, 0, 0.1));
                }
                else if (event.key.keysym.sym == SDLK_s)
                {
                    ctx.camera_position = Vector3_add(ctx.camera_position, Vector3_new(0, 0, -0.1));
                }
            }
        }

        int pitch;
        SDL_LockTexture(texture, NULL, (void **)&ctx.framebuffer, &pitch);

        memset(ctx.framebuffer, 0, ctx.width * ctx.height * 4);
        memset(ctx.depth_buffer, MAXFLOAT, ctx.width * ctx.height * sizeof(float));

        //rasterize(&ctx, &triangle);

        ctx.put_pixel_counter = 0;
        ctx.triangle_processed = 0;
        ctx.triangle_culled = 0;

        

        
        for(i=0;i<ctx.faces_count;i++)
        {
            rasterize(&ctx, &ctx.faces[i]);
        }
        

        printf("%llu %llu %llu %f\n", ctx.put_pixel_counter, ctx.triangle_processed, ctx.triangle_culled, ctx.camera_position.z);

        SDL_UnlockTexture(texture);

        SDL_RenderCopy(renderer, texture, NULL, NULL);

        SDL_RenderPresent(renderer);
    }

    return 0;
}