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
	const int topBarHeight = 20;

	//============================================================================
	// INITIALIZE SCENE 1

    TimeCounter sceneTimer;
    TimeCounter_Init(&sceneTimer);
    TimeCounter_Start(&sceneTimer);

	// Scene content
    void executeScene1(double seconds) {

		if (seconds < 28) {
		    SDL_Texture* background = LoadBackground("assets/room_000.png", renderer);
        	RenderBackground(background, renderer, 320, 0, -75);
        	RenderRain(renderer, 320, 180, 240);
		    SDL_DestroyTexture(background);
        }
        
		if (seconds > 0) { RenderTopBarText(renderer, font, 
						   "It was a rainy night in Delta Sector...", 320, topBarHeight); }
		if (seconds > 10) { RenderTopBarText(renderer, font, 
						   "World has finally reached it's breaking point.", 320, topBarHeight); }
		if (seconds > 20) { RenderTopBarText(renderer, font, 
						   "Decline starts to be hard to hide.\nNobody cares anymore...", 320, topBarHeight); }
    }

    void executeScene2(double seconds) {
		static int yPos = -100;
		
		if (seconds >= 30 && seconds < 35) {
			SDL_Texture* background = LoadBackground("assets/room_001.png", renderer);
        	RenderBackground(background, renderer, 320, 0, yPos);
        	RenderRain(renderer, 320, 220, 240);
        	SDL_DestroyTexture(background);
        	yPos = yPos + 0.00001;
        }

		if (seconds > 30) { RenderTopBarText(renderer, font, 
						    "AI is like cancer.\nSlowly eating people's brains...", 320, topBarHeight); }		    
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

		double seconds = TimeCounter_GetElapsed(&sceneTimer);		
        executeScene1(seconds);
        executeScene2(seconds);

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

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
