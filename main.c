#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include "graphics.h"
#include "time.h"

int main(int argc, char* argv[]) {

	//============================================================================
	// INITIALIZE GAME ENGINE:

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() == -1) {
        fprintf(stderr, "TTF_Init Error: %s\n", TTF_GetError());
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        fprintf(stderr, "IMG_Init Error: %s\n", IMG_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "SDL Background Example",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        320,
        200,
        SDL_WINDOW_SHOWN);
                
    if (!window) {
        fprintf(stderr, "CreateWindow Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "CreateRenderer Error: %s\n", SDL_GetError());
        return 1;
    }

    TTF_Font* font = TTF_OpenFont("fonts/amiga4ever pro2.ttf", 8);
    if (!font) {
        fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
        return 1;
    }

    // Start in fullscreen mode
    ToggleFullscreen(window);

	// Set FPS
	const int FPS = 24;
	const int frameDelay = 1000 / FPS;

	//============================================================================
	// INITIALIZE SCENE 1

    TimeCounter sceneTimer;
    TimeCounter_Init(&sceneTimer);
    TimeCounter_Start(&sceneTimer);

	// Load assets	
    SDL_Texture* background = LoadBackground("assets/room_000.png", renderer);
    if (!background) return 1;

	// Scene content
    void executeScene1() {
		static float backgroundY = -75;
		const int rainGroundYMin = 180;
        const int rainGroundYMax = 240;

		double seconds = TimeCounter_GetElapsed(&sceneTimer);

		if (seconds < 28) {
        	RenderBackground(background, renderer, 320, 0, (int)backgroundY);
        	RenderRain(renderer, 320, rainGroundYMin, rainGroundYMax);
        }
        
		if (seconds > 0) { RenderTopBarText(renderer, font, 
						   "It was a rainy night in Delta Sector...", 320, 15); }
		if (seconds > 10) { RenderTopBarText(renderer, font, 
						   "I had not felt so alone for a long time.", 320, 15); }
		if (seconds > 20) { RenderTopBarText(renderer, font, 
						   "Sometimes I wondered why I even took this job.", 320, 15); }
		    
    }

    //============================================================================
    // GAME LOOP
    
    bool running = true;
    SDL_Event e;
    while (running) {
        Uint32 frameStart = SDL_GetTicks();
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
        }

        SDL_RenderClear(renderer);
		//=======================================================================
		// GAME SCENE CONTENT HERE
		
        executeScene1();

		//=======================================================================
        SDL_RenderPresent(renderer);
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < (Uint32)frameDelay) {
            SDL_Delay(frameDelay - frameTime);
        }
    }

	//============================================================================
	// CLEAN EXIT - Free Memories etc.

	TimeCounter_Stop(&sceneTimer);

    SDL_DestroyTexture(background);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
