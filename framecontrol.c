/******************************************************************************
 * Copyright (c) Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#include "framecontrol.h"
#include "SDL.h"
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
typedef LONGLONG Time;  // Ticks
static const Time TIME_UNDEFINED = -1;
#else
#include <time.h>
typedef long long Time; // Nanoseconds
static const Time TIME_UNDEFINED = -1;
#endif

static struct
{
    Time startTime;
    Time prevFrameTime;
    Time framePeriod;
    unsigned long frameCount;
    double timePerMs;
} control;


static inline double timeToMs( Time time )
{
    return time / control.timePerMs;
}

static inline Time msToTime( double ms )
{
    return ms * control.timePerMs;
}

static Time getCurrentTime()
{
#ifdef _WIN32
    LARGE_INTEGER i;
    if (!QueryPerformanceCounter(&i)) {
        fprintf(stderr, "getCurrentTime(): QueryPerformanceCounter() failed\n");
        return TIME_UNDEFINED;
    }
    return i.QuadPart;
#else
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * (Time)1000000000 + t.tv_nsec;
#endif
}

int startFrameControl( int fps )
{
#ifdef _WIN32
    LARGE_INTEGER i;
    if (!QueryPerformanceFrequency(&i)) {
        fprintf(stderr, "startFrameControl(): QueryPerformanceFrequency() failed\n");
        return 0;
    }
    control.timePerMs = i.QuadPart / 1000.0;
#else
    control.timePerMs = 1000000;
#endif
    control.startTime = getCurrentTime();
    if (control.startTime == TIME_UNDEFINED) {
        fprintf(stderr, "startFrameControl(): Can't get current time\n");
        return 0;
    }
    control.prevFrameTime = 0;
    control.framePeriod = msToTime(1000.0 / fps);
    control.frameCount = 0;
    return 1;
}

void waitForNextFrame()
{
    const Time nextFrameTime = control.prevFrameTime + control.framePeriod;
    while (1) {
        const Time currentTime = getCurrentTime();
        if (currentTime >= nextFrameTime) {
            break;
        }
        if (nextFrameTime - currentTime > control.timePerMs) {
            SDL_Delay(1);
        } else {
            // Just spin in the loop, because this can be more precise
            // than system delay
        }
    }
    control.prevFrameTime = getCurrentTime();
    control.frameCount += 1;
}

double getElapsedTime()
{
    return timeToMs(getCurrentTime() - control.startTime);
}

double getCurrentFps()
{
    return control.frameCount / (timeToMs(control.prevFrameTime - control.startTime) / 1000.0);
}
