// main.c
// Compile with:
// gcc main.c -o peli \
//     $(sdl2-config --cflags --libs) \
//     -lSDL2_gfx -lm

#include <SDL2/SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

//==============================================================================
// CONSTANTS:

#define WING_SPREAD 33.0
#define LANDING_VELOCITY_THRESHOLD 10.0f  // max safe vertical speed for landing
#define MAX_LEVEL_POINTS 64

//==============================================================================
// MATH / UTILITY:

static double degToRad(double angleDeg) {
    return angleDeg * M_PI / 180.0;
}

//==============================================================================
// VECTOR, ROCKET & WORLD STRUCTS:

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
// LEVEL / TERRAIN:

typedef struct {
    int numPoints;
    Vec2 points[MAX_LEVEL_POINTS];
} Level;

static Level levels[] = {
    {5, {{0,200}, {80,200}, {160,160}, {240,200}, {320,200}}},
    {3, {{0,220}, {160,120}, {320,220}}}
};
static const int numLevels = sizeof(levels) / sizeof(levels[0]);

//==============================================================================
// SEGMENT–SEGMENT INTERSECTION:

// returns true if segment AB intersects segment CD
static bool LineIntersect(Vec2 A, Vec2 B, Vec2 C, Vec2 D) {
    // from: https://stackoverflow.com/a/1968345
    float s1_x = B.x - A.x;
    float s1_y = B.y - A.y;
    float s2_x = D.x - C.x;
    float s2_y = D.y - C.y;

    float denom = (-s2_x * s1_y + s1_x * s2_y);
    if (fabsf(denom) < 1e-6f) return false; // parallel or nearly so

    float s = (-s1_y * (A.x - C.x) + s1_x * (A.y - C.y)) / denom;
    float t = ( s2_x * (A.y - C.y) - s2_y * (A.x - C.x)) / denom;

    return (s >= 0 && s <= 1 && t >= 0 && t <= 1);
}

//==============================================================================
// DRAWING & HELPERS:

static void DrawGround(SDL_Renderer *ren, const Level *lvl) {
    SDL_SetRenderDrawColor(ren, 0, 255, 0, 255);
    for (int i = 0; i < lvl->numPoints - 1; i++) {
        SDL_RenderDrawLine(
            ren,
            (int)lvl->points[i].x,   (int)lvl->points[i].y,
            (int)lvl->points[i+1].x, (int)lvl->points[i+1].y
        );
    }
}

static float GroundYAtX(const Level *lvl, float x) {
    for (int i = 0; i < lvl->numPoints - 1; i++) {
        float x0 = lvl->points[i].x, y0 = lvl->points[i].y;
        float x1 = lvl->points[i+1].x, y1 = lvl->points[i+1].y;
        if ((x >= x0 && x <= x1) || (x >= x1 && x <= x0)) {
            float t = (x - x0) / (x1 - x0);
            return y0 + t * (y1 - y0);
        }
    }
    if (x < lvl->points[0].x) return lvl->points[0].y;
    return lvl->points[lvl->numPoints-1].y;
}

static void DrawRocket(SDL_Renderer *ren, int x0, int y0,
                       double angleDeg, int size)
{
    const double wingSpread = WING_SPREAD;
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
// ROCKET RESET:

void ResetRocket(Rocket *r) {
    r->position = (Vec2){160, 120};
    r->velocity = (Vec2){0, 0};
    r->acceleration = (Vec2){0, 0};
    r->angle = 0;
    r->angularVelocity = 0;
    r->fuel = 20;
    r->thrustLevel = 0;
}

//==============================================================================
// MAIN / GAME LOOP:

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    SDL_Window *win = SDL_CreateWindow(
        "Rocket Physics",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP
    );
    SDL_Renderer *ren = SDL_CreateRenderer(
        win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    SDL_RenderSetLogicalSize(ren, 320, 240);

    int currentLevel = 0;
    World world = { .gravity = 9.82f };
    Rocket rocket;
    ResetRocket(&rocket);
    rocket.mass           = 1;
    rocket.maxThrust      = 100;
    rocket.dragCoefficient= 0.05f;
    rocket.frontalArea    = 0.01f;
    rocket.fuelConsumption= 1;
    rocket.size           = 10;

    const float dt     = 1.0f / 60.0f;
    const float rotVel = 120.0f;
    bool gameOver = false;
    SDL_Event ev;

    while (1) {
        // Event handling
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) return 0;
            if (ev.type == SDL_KEYDOWN) {
                if (ev.key.keysym.sym == SDLK_ESCAPE) return 0;
                if (gameOver && ev.key.keysym.sym == SDLK_RETURN) {
                    // restart after crash
                    gameOver = false;
                    currentLevel = 0;
                    ResetRocket(&rocket);
                }
            }
        }

        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if (!gameOver) {
            // controls
            rocket.angularVelocity =
                keys[SDL_SCANCODE_LEFT]  ? -rotVel :
                keys[SDL_SCANCODE_RIGHT] ?  rotVel : 0;
            rocket.thrustLevel = (keys[SDL_SCANCODE_UP] && rocket.fuel > 0.0f)
                                 ? -1.0f : 0.0f;

            // physics
            rocket.angle += rocket.angularVelocity * dt;
            float thrust = rocket.maxThrust * rocket.thrustLevel;
            double rad = degToRad(rocket.angle);

            // drag forces
            float dragX = rocket.dragCoefficient * rocket.velocity.x * fabsf(rocket.velocity.x);
            float dragY = rocket.dragCoefficient * rocket.velocity.y * fabsf(rocket.velocity.y);

            // detect if resting on ground
            float gY = GroundYAtX(&levels[currentLevel], rocket.position.x);
            bool onGround = (rocket.position.y + rocket.size >= gY);

            // horizontal accel always
            rocket.acceleration.x = cos(rad) * thrust / rocket.mass - dragX;

            // vertical accel: zero if on ground with no thrust, else normal
            if (onGround && rocket.thrustLevel == 0.0f) {
                rocket.acceleration.y = 0.0f;
                rocket.velocity.y     = 0.0f;
                rocket.position.y = gY - rocket.size;
            } else {
                rocket.acceleration.y = sin(rad) * thrust / rocket.mass
                                      - dragY
                                      + world.gravity;
            }

            // integrate
            rocket.velocity.x += rocket.acceleration.x * dt;
            rocket.velocity.y += rocket.acceleration.y * dt;
            rocket.position.x += rocket.velocity.x * dt;
            rocket.position.y += rocket.velocity.y * dt;
            rocket.fuel -= rocket.fuelConsumption * rocket.thrustLevel * dt;
            if (rocket.fuel < 0.0f) {
                rocket.fuel = 0.0f;
                rocket.thrustLevel = 0.0f;
            }

            // pixel-perfect collision
            {
                Vec2 R0, R1, R2;
                double halfS = rocket.size * 0.5;
                double bisect = rocket.angle + WING_SPREAD * 0.5;
                double bRad   = degToRad(bisect);
                double aRad   = degToRad(rocket.angle);
                double aRad2  = degToRad(rocket.angle + WING_SPREAD);

                R0.x = rocket.position.x - cos(bRad) * halfS;
                R0.y = rocket.position.y - sin(bRad) * halfS;
                R1.x = R0.x + cos(aRad)  * rocket.size;
                R1.y = R0.y + sin(aRad)  * rocket.size;
                R2.x = R0.x + cos(aRad2) * rocket.size;
                R2.y = R0.y + sin(aRad2) * rocket.size;

                const Level *L = &levels[currentLevel];
                bool collided = false;
                for (int i = 0; i < L->numPoints - 1 && !collided; i++) {
                    Vec2 G0 = L->points[i], G1 = L->points[i+1];
                    if (LineIntersect(R0, R1, G0, G1) ||
                        LineIntersect(R0, R2, G0, G1)) {
                        collided = true;
                    }
                }

                if (collided) {
                    if (fabsf(rocket.velocity.y) > LANDING_VELOCITY_THRESHOLD) {
                        gameOver = true;
                    } else {
                        // safe landing clamp
                        float gY2 = GroundYAtX(L, rocket.position.x);
                        rocket.position.y = gY2 - rocket.size;
                        rocket.velocity.y = 0.0f;
                    }
                }
            }
        }

        // rendering
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        DrawGround(ren, &levels[currentLevel]);
        DrawRocket(
            ren,
            (int)roundf(rocket.position.x),
            (int)roundf(rocket.position.y),
            rocket.angle,
            rocket.size
        );

        // HUD
        char buf[64];
        // Fuel
        snprintf(buf, sizeof(buf), "Fuel: %.1f kg", rocket.fuel);
        stringRGBA(ren, 10, 10, buf, 255, 255, 255, 255);
        // Velocity along rocket's facing direction
        {
            double rad = degToRad(rocket.angle);
            float v_proj = rocket.velocity.x * cos(rad)
                         + rocket.velocity.y * sin(rad);
            float speed = fabsf(v_proj);
            snprintf(buf, sizeof(buf), "Speed: %.1f", speed);
            stringRGBA(ren, 10, 20, buf, 255, 255, 255, 255);
        }

        if (gameOver) {
            stringRGBA(ren, 100, 120, "GAME OVER", 255, 0, 0, 255);
            stringRGBA(ren, 80, 140, "Press ENTER to restart", 255, 255, 255, 255);
        }

        SDL_RenderPresent(ren);
        SDL_Delay((Uint32)(dt * 1000));
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
