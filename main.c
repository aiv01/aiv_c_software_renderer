#include "aiv_renderer.h"
#include <stdlib.h>
#include <SDL2/SDL.h>

#define triangle(x0, y0, z0, x1, y1, z1, x2, y2, z2) \
    Triangle_new(                                    \
        Vertex_new(Vector3_new(x0, y0, z0)),         \
        Vertex_new(Vector3_new(x1, y1, z1)),         \
        Vertex_new(Vector3_new(x2, y2, z2)))

int main(int argc, char **argv)
{
    Context_t ctx;
    ctx.width = 600;
    ctx.height = 600;

    ctx.framebuffer = NULL;

    Triangle_t triangle = triangle(0, 0.5, 0, -0.5, 0, 0, 0.5, 0, 0);

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 600, 600, 0);
    if (!window)
        return -1;

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
        return -1;

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 600, 600);
    if (!texture)
        return -1;

    for (;;)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                return 0;
        }

        int pitch;
        SDL_LockTexture(texture, NULL, (void **)&ctx.framebuffer, &pitch);

        rasterize(&ctx, &triangle);

        SDL_UnlockTexture(texture);

        SDL_RenderCopy(renderer, texture, NULL, NULL);

        SDL_RenderPresent(renderer);
    }

    return 0;
}