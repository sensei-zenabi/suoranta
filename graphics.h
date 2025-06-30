#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// Loads a background texture from file
SDL_Texture* LoadBackground(const char* filepath, SDL_Renderer* renderer);

// Toggles the given window between windowed and fullscreen mode
void ToggleFullscreen(SDL_Window* window);

// Renders the background texture scaled to the given width at the
// specified (x, y) position. Height is adjusted to maintain the
// texture's aspect ratio.
void RenderBackground(SDL_Texture* background, SDL_Renderer* renderer,
                      int width, int x, int y);

// Draws a text string centered inside a black bar at the top of the screen
// Parameters:
//   renderer    The renderer to draw with
//   font        Loaded TTF font used to render the text
//   text        UTF-8 string to display
//   windowWidth Width of the window/bar in pixels
//   barHeight   Height of the bar in pixels
void RenderTopBarText(SDL_Renderer* renderer, TTF_Font* font, const char* text,
                      int windowWidth, int barHeight);

#endif
