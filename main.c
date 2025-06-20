// main.c
#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>

//=============================================================================
// MATH / UTILITY:

static double degToRad(double angleDeg) {
    return angleDeg * M_PI / 180.0;
}

//=============================================================================
// VECTOR & ROCKET STRUCTS:

typedef struct {
    float x, y;
} Vec2;

typedef struct {
    // Kinematic state
    Vec2    position;            // px
    Vec2    velocity;            // px/sec
    Vec2    acceleration;        // px/sec²
    float   angle;               // degrees
    float   angularVelocity;     // deg/sec
    float   angularAcceleration; // deg/sec²

    // Physical parameters
    float   mass;                // kg
    float   maxThrust;           // N
    float   thrustLevel;         // 0–1
    float   dragCoefficient;     // dimensionless
    float   frontalArea;         // m²
    float   fuel;                // kg
    float   fuelConsumption;     // kg/sec @ full throttle

    // Rendering
    int     size;                // px (line length)
} Rocket;

typedef struct {
    float gravity;
} World;

//=============================================================================
// DRAWING:

static void DrawRocket(SDL_Renderer *ren,
                       int x0, int y0,
                       double angleDeg,
                       int size)
{
    const double wingSpread = 33.0;
    const double halfSize  = size * 0.5;

    // Bisector of the V
    double bisectDeg = angleDeg + wingSpread * 0.5;
    double bisectRad = degToRad(bisectDeg);

    // Compute tail point (old origin) halfSize back along bisector
    double ox = x0 - cos(bisectRad) * halfSize;
    double oy = y0 - sin(bisectRad) * halfSize;
    int ix0 = (int)round(ox), iy0 = (int)round(oy);

    // Compute the two tips
    double rad1 = degToRad(angleDeg);
    double rad2 = degToRad(angleDeg + wingSpread);
    int x1 = ix0 + (int)round(cos(rad1) * size);
    int y1 = iy0 + (int)round(sin(rad1) * size);
    int x2 = ix0 + (int)round(cos(rad2) * size);
    int y2 = iy0 + (int)round(sin(rad2) * size);

    SDL_RenderDrawLine(ren, ix0, iy0, x1, y1);
    SDL_RenderDrawLine(ren, ix0, iy0, x2, y2);
}

//=============================================================================
// MAIN / GAME LOOP:

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow(
        "Rocket Physics",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        0, 0,
        SDL_WINDOW_FULLSCREEN_DESKTOP
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

    // Retro 320×240 scaling
    SDL_RenderSetLogicalSize(ren, 320, 240);

    // Initialize world
    World world = {
        .gravity            = 9.82f
    };

    // Initialize rocket1
    Rocket rocket1 = {
        .position           = {160.0f, 120.0f},
        .velocity           = {  0.0f,   0.0f},
        .acceleration       = {  0.0f,   0.0f},
        .angle              =   0.0f,
        .angularVelocity    =   0.0f,
        .angularAcceleration=   0.0f,
        .mass               =   1.0f,
        .maxThrust          = 100.0f,
        .thrustLevel        =   0.0f,
        .dragCoefficient    =   0.05f,
        .frontalArea        =   0.01f,
        .fuel               =  20.0f,
        .fuelConsumption    =   1.0f,
        .size               =  10
    };

    const float dt = 1.0f / 60.0f;   // fixed timestep ~60 FPS
    const float rotVelocity = 120.0f;   // deg/sec² when ◀️/▶️ held

    int running = 1;
    SDL_Event ev;

    while (running) {
        // 1) Events
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                running = 0;
            }
            else if (ev.type == SDL_KEYDOWN &&
                     ev.key.keysym.sym == SDLK_ESCAPE)
            {
                running = 0;
            }
        }

        // 2) Input & control
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        // Angular control
        if (keys[SDL_SCANCODE_LEFT]) {
            rocket1.angularVelocity = -rotVelocity;
        }
        else if (keys[SDL_SCANCODE_RIGHT]) {
            rocket1.angularVelocity = rotVelocity;
        }
        else {
            rocket1.angularVelocity = 0.0f;
        }
        // Throttle
        if (keys[SDL_SCANCODE_UP] && rocket1.fuel > 0.0f) {
            rocket1.thrustLevel = -1.0f;
        } else {
            rocket1.thrustLevel = 0.0f;
        }

        // 3) Physics integration
        // Angular acceleration is bypassed
        // rocket1.angularVelocity    += rocket1.angularAcceleration * dt;
        // rocket1.angle              += rocket1.angularVelocity    * dt;
        rocket1.angle                 += rocket1.angularVelocity * dt;

        // Linear: thrust
        float thrustForce = rocket1.maxThrust * rocket1.thrustLevel;
        double rad = degToRad(rocket1.angle);
        rocket1.acceleration.x = cos(rad) * thrustForce / rocket1.mass;
        rocket1.acceleration.y = sin(rad) * thrustForce / rocket1.mass;

        // Simple quadratic drag: F_drag ≈ –Cd·v·|v|
        rocket1.acceleration.x -= rocket1.dragCoefficient
                                * rocket1.velocity.x
                                * fabsf(rocket1.velocity.x);
        rocket1.acceleration.y -= rocket1.dragCoefficient
                                * rocket1.velocity.y
                                * fabsf(rocket1.velocity.y);
        rocket1.acceleration.y += world.gravity;

        // Integrate velocity & position
        rocket1.velocity.x   += rocket1.acceleration.x * dt;
        rocket1.velocity.y   += rocket1.acceleration.y * dt;
        rocket1.position.x   += rocket1.velocity.x    * dt;
        rocket1.position.y   += rocket1.velocity.y    * dt;

        // Fuel burn
        rocket1.fuel -= rocket1.fuelConsumption
                      * rocket1.thrustLevel
                      * dt;
        if (rocket1.fuel < 0.0f) {
            rocket1.fuel = 0.0f;
            rocket1.thrustLevel = 0.0f;
        }

        // 4) Render
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
        DrawRocket(ren,
                   (int)roundf(rocket1.position.x),
                   (int)roundf(rocket1.position.y),
                   rocket1.angle,
                   rocket1.size);

        SDL_RenderPresent(ren);

        // 5) Frame cap
        SDL_Delay((Uint32)(dt * 1000));
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
