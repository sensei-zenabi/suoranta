
#include "graphics.h"
#include <SDL_image.h>
#include <stdbool.h>
#include <stdio.h>

void ToggleFullscreen(SDL_Window* window) {
    if (!window) return;
    Uint32 fullscreenFlag = SDL_WINDOW_FULLSCREEN;
    bool isFullscreen = SDL_GetWindowFlags(window) & fullscreenFlag;
    if (isFullscreen) {
        SDL_SetWindowFullscreen(window, 0);
    } else {
        SDL_DisplayMode mode = {0};
        mode.w = 320;
        mode.h = 200;
        SDL_SetWindowDisplayMode(window, &mode);
        SDL_SetWindowFullscreen(window, fullscreenFlag);
    }
}

SDL_Texture* LoadBackground(const char* filepath, SDL_Renderer* renderer) {
    SDL_Texture* background = IMG_LoadTexture(renderer, filepath);
    if (!background) {
        fprintf(stderr, "Failed to load background image: %s\n", IMG_GetError());
        return NULL;
    }
    return background;
}

void RenderBackground(SDL_Texture* background, SDL_Renderer* renderer, int width) {       
    int texW = 0, texH = 0;
    SDL_QueryTexture(background, NULL, NULL, &texW, &texH);

    SDL_Rect dst = {0, 0, texW, texH};
    if (width > 0) {
        dst.w = width;
        dst.h = (int)((double)texH * width / texW);
    }

    SDL_RenderCopy(renderer, background, NULL, &dst);}
