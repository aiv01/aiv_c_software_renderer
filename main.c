#include "aiv_obj_parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(int argc, char **argv)
{
    Context_t ctx;
    memset(&ctx, 0, sizeof(Context_t));
    ctx.width = 600;
    ctx.height = 600;

    ctx.camera_position = Vector3_new(0, 1.5, -5);
    ctx.point_light_position = Vector3_new(0, 0, -2);

    ctx.framebuffer = NULL;

    int comp = 0;
    ctx.texture = stbi_load(argv[2], &ctx.texture_width, &ctx.texture_height, &comp, 3);

    ctx.depth_buffer = malloc(sizeof(float) * ctx.width * ctx.height);

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
        memset(ctx.depth_buffer, 0xff, ctx.width * ctx.height * sizeof(float));

        ctx.put_pixel_counter = 0;
        ctx.triangle_counter = 0;

        ctx.roty += 0.01;

        size_t i;
        for (i = 0; i < ctx.faces_count; i++)
        {
            rasterize(&ctx, &ctx.faces[i]);
        }

        printf("put_pixel: %llu triangles: %llu\n", ctx.put_pixel_counter, ctx.triangle_counter);

        SDL_UnlockTexture(texture);

        SDL_RenderCopy(renderer, texture, NULL, NULL);

        SDL_RenderPresent(renderer);
    }

    return 0;
}