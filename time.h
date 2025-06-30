#ifndef TIME_H
#define TIME_H

#include <stdbool.h>
#include <SDL2/SDL.h>

typedef struct {
    Uint64 start_ticks;
    Uint64 elapsed_ticks;
    bool running;
} TimeCounter;

// Initialize a TimeCounter structure
void TimeCounter_Init(TimeCounter *counter);

// Start measuring elapsed time
void TimeCounter_Start(TimeCounter *counter);

// Stop measuring and accumulate elapsed time
void TimeCounter_Stop(TimeCounter *counter);

// Reset the counter to zero. If running, restart from current time
void TimeCounter_Reset(TimeCounter *counter);

// Get the elapsed time in seconds
// If the counter is running, it includes the time since last start
double TimeCounter_GetElapsed(const TimeCounter *counter);

#endif // TIME_H
