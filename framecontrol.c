/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#include "framecontrol.h"
#include "SDL.h"
#include <stdio.h>

int FrameControl_start( FrameControl* c, int fps )
{
    c->framePeriod = 1000.0 / fps;
    c->prevFrameTime = 0;
    c->frameCount = 0;
#ifdef _MSC_VER
    LARGE_INTEGER i;
    if (!QueryPerformanceFrequency(&i)) {
        fprintf(stderr, "FrameControl_start(): QueryPerformanceFrequency() failed\n");
        return 0;
    }
    c->cpuFrequency = i.QuadPart / 1000.0;
    if (!QueryPerformanceCounter(&i)) {
        fprintf(stderr, "FrameControl_start(): QueryPerformanceCounter() failed\n");
        return 0;
    }
    c->startTime = i.QuadPart;
#else
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    c->startTime = (now.tv_sec + now.tv_nsec / 1000000000.0) * 1000.0;
#endif
    return 1;
}

void FrameControl_waitNextFrame( FrameControl* c )
{
    const double nextFrameTime = c->prevFrameTime + c->framePeriod;
    double remainedTime;
    while ((remainedTime = nextFrameTime - FrameControl_getElapsedTime(c)) > 0) {
        if (remainedTime > 1) {
            SDL_Delay(1);
        } else {
            // Just spin in the loop, because this can be more precise
            // than system delay
        }
    }
    c->prevFrameTime = FrameControl_getElapsedTime(c);
    c->frameCount += 1;
}

double FrameControl_getElapsedTime( FrameControl* c )
{
#ifdef _MSC_VER
    LARGE_INTEGER i;
    if (!QueryPerformanceCounter(&i)) {
        fprintf(stderr, "FrameControl_getElapsedTime(): QueryPerformanceCounter() failed\n");
        return 0;
    }
    return (i.QuadPart - c->startTime) / c->cpuFrequency;
#else
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (now.tv_sec + now.tv_nsec / 1000000000.0) * 1000.0 - c->startTime;
#endif
}

double FrameControl_getRealFps( FrameControl* c )
{
    return c->frameCount / (c->prevFrameTime / 1000.0);
}
