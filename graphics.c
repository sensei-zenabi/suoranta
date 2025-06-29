#include "graphics.h"
#include <SDL_image.h>
#include <stdbool.h>
#include <stdio.h>

void ToggleFullscreen(SDL_Window* window) {
    if (!window) return;
    Uint32 fullscreenFlag = SDL_WINDOW_FULLSCREEN_DESKTOP;
    bool isFullscreen = SDL_GetWindowFlags(window) & fullscreenFlag;
    if (isFullscreen) {
        SDL_SetWindowFullscreen(window, 0);
    } else {
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

void RenderBackground(SDL_Texture* background, SDL_Renderer* renderer) {
    if (!background) return;
    SDL_RenderCopy(renderer, background, NULL, NULL);
}
