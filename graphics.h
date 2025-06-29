#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL2/SDL.h>

// Loads a background texture from file
SDL_Texture* LoadBackground(const char* filepath, SDL_Renderer* renderer);

// Toggles the given window between windowed and fullscreen mode
void ToggleFullscreen(SDL_Window* window);

// Renders the background texture scaled to the given width at the
// specified (x, y) position. Height is adjusted to maintain the
// texture's aspect ratio.
void RenderBackground(SDL_Texture* background, SDL_Renderer* renderer,
                      int width, int x, int y);
#endif
