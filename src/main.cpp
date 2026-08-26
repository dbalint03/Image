#include "image.hpp"

#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static int texture_width = 0;
static int texture_height = 0;

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_Surface *surface = NULL;
    // char *png_path = NULL;

    SDL_SetAppMetadata("Example Renderer Textures", "1.0", "com.example.renderer-textures");

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("examples/renderer/textures", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer))
    {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderLogicalPresentation(renderer, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    Image img("E:\\Projects\\cpp\\Image\\kenny2.png");
    surface = SDL_CreateSurfaceFrom(img.width, img.height, SDL_PIXELFORMAT_INDEX8, img.pixels.data(), static_cast<int>(img.get_row_size()));
    if (!surface)
    {
        SDL_Log("Couldn't load png: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Color colors[256];
    SDL_Palette *palette = SDL_CreatePalette(256);
    if (!palette)
    {
        SDL_Log("Couldn't create surface palette: %s", SDL_GetError());
        SDL_DestroySurface(surface);
        return SDL_APP_FAILURE;
    }

    for (int i = 0; i < 256; i++)
    {
        colors[i].r = colors[i].g = colors[i].b = i;
        colors[i].a = SDL_ALPHA_OPAQUE;
    }

    if (!SDL_SetPaletteColors(palette, colors, 0, 256))
    {
        SDL_Log("Couldn't set surface palette: %s", SDL_GetError());
        SDL_DestroyPalette(palette);
        SDL_DestroySurface(surface);
        return SDL_APP_FAILURE;
    }

    if (!SDL_SetSurfacePalette(surface, palette))
    {
        SDL_Log("Couldn't attach surface palette: %s", SDL_GetError());
        SDL_DestroyPalette(palette);
        SDL_DestroySurface(surface);
        return SDL_APP_FAILURE;
    }
    SDL_DestroyPalette(palette);

    texture_width = surface->w;
    texture_height = surface->h;

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture)
    {
        SDL_Log("Couldn't create static texture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_DestroySurface(surface); /* done with this, the texture has a copy of the pixels now. */

    return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT)
    {
        return SDL_APP_SUCCESS; /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    SDL_RenderClear(renderer);

    SDL_RenderTexture(renderer, texture, NULL, NULL);

    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    SDL_DestroyTexture(texture);
    /* SDL will clean up the window/renderer for us. */
}
