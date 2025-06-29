#include "graphics.h"
#include <SDL_image.h>
#include <stdio.h>

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
