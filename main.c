// main.c
// Compile with:
// gcc main.c -o peli \
//     $(sdl2-config --cflags --libs) \
//     -lSDL2_gfx -lm

#include <SDL2/SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <stdio.h>
#include <math.h>

//==============================================================================
// MATH / UTILITY:

static double degToRad(double angleDeg) {
    return angleDeg * M_PI / 180.0;
}

//==============================================================================
// VECTOR & ROCKET STRUCTS:

typedef struct { float x, y; } Vec2;

typedef struct {
    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;
    float angle;
    float angularVelocity;

    float mass;
    float maxThrust;
    float thrustLevel;
    float dragCoefficient;
    float frontalArea;
    float fuel;
    float fuelConsumption;
    int size;
} Rocket;

typedef struct { float gravity; } World;

//==============================================================================
// DRAWING ROCKET:

static void DrawRocket(SDL_Renderer *ren, int x0, int y0,
                       double angleDeg, int size)
{
    const double wingSpread = 33.0;
    const double halfSize  = size * 0.5;
    double bisectDeg = angleDeg + wingSpread * 0.5;
    double bisectRad = degToRad(bisectDeg);

    double ox = x0 - cos(bisectRad) * halfSize;
    double oy = y0 - sin(bisectRad) * halfSize;
    int ix0 = (int)round(ox), iy0 = (int)round(oy);

    double rad1 = degToRad(angleDeg);
    double rad2 = degToRad(angleDeg + wingSpread);
    int x1 = ix0 + (int)round(cos(rad1) * size);
    int y1 = iy0 + (int)round(sin(rad1) * size);
    int x2 = ix0 + (int)round(cos(rad2) * size);
    int y2 = iy0 + (int)round(sin(rad2) * size);

    SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
    SDL_RenderDrawLine(ren, ix0, iy0, x1, y1);
    SDL_RenderDrawLine(ren, ix0, iy0, x2, y2);
}

//==============================================================================
// MAIN / GAME LOOP:

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    SDL_Window *win = SDL_CreateWindow(
        "Rocket Physics",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP
    );
    if (!win) {
        fprintf(stderr, "Window error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *ren = SDL_CreateRenderer(
        win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!ren) {
        fprintf(stderr, "Renderer error: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    SDL_RenderSetLogicalSize(ren, 320, 240);

    World world = { .gravity = 9.82f };
    Rocket rocket1 = {
        .position = {160.0f, 120.0f},
        .velocity = {0.0f, 0.0f},
        .acceleration = {0.0f, 0.0f},
        .angle = 0.0f,
        .angularVelocity = 0.0f,
        .mass = 1.0f,
        .maxThrust = 100.0f,
        .thrustLevel = 0.0f,
        .dragCoefficient = 0.05f,
        .frontalArea = 0.01f,
        .fuel = 20.0f,
        .fuelConsumption = 1.0f,
        .size = 10
    };

    const float dt = 1.0f / 60.0f;
    const float rotVelocity = 120.0f;
    int running = 1;
    SDL_Event ev;

    while (running) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT || 
                (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE)) {
                running = 0;
            }
        }

        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_LEFT])      rocket1.angularVelocity = -rotVelocity;
        else if (keys[SDL_SCANCODE_RIGHT]) rocket1.angularVelocity = rotVelocity;
        else                                rocket1.angularVelocity = 0.0f;

        rocket1.thrustLevel = (keys[SDL_SCANCODE_UP] && rocket1.fuel > 0.0f) ? -1.0f : 0.0f;

        // Physics integration
        rocket1.angle += rocket1.angularVelocity * dt;
        float thrustForce = rocket1.maxThrust * rocket1.thrustLevel;
        double rad = degToRad(rocket1.angle);
        rocket1.acceleration.x = cos(rad) * thrustForce / rocket1.mass;
        rocket1.acceleration.y = sin(rad) * thrustForce / rocket1.mass;
        rocket1.acceleration.x -= rocket1.dragCoefficient * rocket1.velocity.x * fabsf(rocket1.velocity.x);
        rocket1.acceleration.y -= rocket1.dragCoefficient * rocket1.velocity.y * fabsf(rocket1.velocity.y);
        rocket1.acceleration.y += world.gravity;

        rocket1.velocity.x += rocket1.acceleration.x * dt;
        rocket1.velocity.y += rocket1.acceleration.y * dt;
        rocket1.position.x += rocket1.velocity.x * dt;
        rocket1.position.y += rocket1.velocity.y * dt;
        rocket1.fuel -= rocket1.fuelConsumption * rocket1.thrustLevel * dt;
        if (rocket1.fuel < 0.0f) { rocket1.fuel = 0.0f; rocket1.thrustLevel = 0.0f; }

        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        DrawRocket(ren,
                   (int)roundf(rocket1.position.x),
                   (int)roundf(rocket1.position.y),
                   rocket1.angle,
                   rocket1.size);

        // Print rocket1 stats
        char fuelText[32];
        snprintf(fuelText, sizeof(fuelText), "Fuel: %.1f kg", rocket1.fuel);
        stringRGBA(ren, 10, 10, fuelText, 255, 255, 255, 255);

        char thrustText[32];
        snprintf(thrustText, sizeof(thrustText), "Thrust: %.1f", rocket1.thrustLevel);
        stringRGBA(ren, 10, 20, thrustText, 255, 255, 255, 255);


        SDL_RenderPresent(ren);
        SDL_Delay((Uint32)(dt * 1000));
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
