
#include "graphics.h"
#include <SDL_image.h>
#include <stdbool.h>
#include <stdio.h>
#include <SDL_ttf.h>

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

void RenderBackground(SDL_Texture* background, SDL_Renderer* renderer,
                      int width, int x, int y) {
    int texW = 0, texH = 0;
    SDL_QueryTexture(background, NULL, NULL, &texW, &texH);

    SDL_Rect dst = {x, y, texW, texH};
    if (width > 0) {
        dst.w = width;
        dst.h = (int)((double)texH * width / texW);
    }

    SDL_RenderCopy(renderer, background, NULL, &dst);
}

void RenderTopBarText(SDL_Renderer* renderer, TTF_Font* font, const char* text,
                      int windowWidth, int barHeight) {
    if (!renderer || !font || !text) return;

    SDL_Rect bar = {0, 0, windowWidth, barHeight};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &bar);

    SDL_Color color = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    int texW = surface->w;
    int texH = surface->h;
    SDL_FreeSurface(surface);

    if (!texture) return;

    SDL_Rect dst = {(windowWidth - texW) / 2, (barHeight - texH) / 2, texW, texH};
    SDL_RenderCopy(renderer, texture, NULL, &dst);
    SDL_DestroyTexture(texture);
}
