#include "time.h"

void TimeCounter_Init(TimeCounter *counter) {
    if (!counter) return;
    counter->start_ticks = 0;
    counter->elapsed_ticks = 0;
    counter->running = false;
}

void TimeCounter_Start(TimeCounter *counter) {
    if (!counter) return;
    if (!counter->running) {
        counter->start_ticks = SDL_GetPerformanceCounter();
        counter->running = true;
    }
}

void TimeCounter_Stop(TimeCounter *counter) {
    if (!counter) return;
    if (counter->running) {
        Uint64 now = SDL_GetPerformanceCounter();
        counter->elapsed_ticks += now - counter->start_ticks;
        counter->running = false;
    }
}

void TimeCounter_Reset(TimeCounter *counter) {
    if (!counter) return;
    counter->elapsed_ticks = 0;
    if (counter->running) {
        counter->start_ticks = SDL_GetPerformanceCounter();
    } else {
        counter->start_ticks = 0;
    }
}

double TimeCounter_GetElapsed(const TimeCounter *counter) {
    if (!counter) return 0.0;
    Uint64 elapsed = counter->elapsed_ticks;
    if (counter->running) {
        Uint64 now = SDL_GetPerformanceCounter();
        elapsed += now - counter->start_ticks;
    }
    double freq = (double)SDL_GetPerformanceFrequency();
    return (double)elapsed / freq;
}
