#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL.h>

// Loads a background texture from file
SDL_Texture* LoadBackground(const char* filepath, SDL_Renderer* renderer);

// Renders the background texture to fill the screen
void RenderBackground(SDL_Texture* background, SDL_Renderer* renderer);

#endif
