/******************************************************************************
 * Copyright (c) Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#include "framecontrol.h"
#include "helpers.h"
#include "SDL.h"
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
typedef LONGLONG Time;                   // Ticks
static const Time TIME_UNDEFINED = -1;
static const int SYSTEM_TIMER_PERIOD = 1 // Milliseconds
#else
#include <time.h>
typedef long long Time;                  // Nanoseconds
static const Time TIME_UNDEFINED = -1;
#endif

static struct
{
    int started;
    Time startTime;
    Time prevFrameTime;
    Time elapsedFrameTime;
    Time framePeriod;
    unsigned long frameCount;
    double maxDeltaTime;
    double timePerMs;
} control = {0};


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
    ensure(QueryPerformanceCounter(&i), "getCurrentTime(): QueryPerformanceCounter() failed");
    return i.QuadPart;
#else
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * (Time)1000000000 + t.tv_nsec;
#endif
}

// If fps <= 0, new frame will be ready right after the previous one is handled,
// i.e. there will be no fps limit.
// 
// The maxDeltaTime is the maximum time increment which can be correctly handled,
// in ms. If it's <= 0, there will be no increment limit.
//
// The maxDeltaTime is used because, if the frame takes too long, the game may
// change more than we can handle: e.g. some object may move too far, across the
// walls. To solve this, the long frame can be handled as multiple shorter frames,
// each <= maxDeltaTime. Or, if it's not required to sync the game time with the
// real time, the long frame can be simply truncated to maxDeltaTime. This is done
// in getElapsedFrameTime(), so that each long frame will be treated as a shorter
// one (the game will slow down at these moments). For more information, see
// https://gafferongames.com/post/fix_your_timestep/
void startFrameControl( int fps, double maxDeltaTime )
{
#ifdef _WIN32
    LARGE_INTEGER i;
    ensure(QueryPerformanceFrequency(&i), "startFrameControl(): QueryPerformanceFrequency() failed");
    control.timePerMs = i.QuadPart / 1000.0;
#else
    control.timePerMs = 1000000;
#endif
    control.startTime = getCurrentTime();
    ensure(control.startTime != TIME_UNDEFINED, "startFrameControl(): Can't get current time");
    control.prevFrameTime = control.startTime;
    control.elapsedFrameTime = 0;
    control.framePeriod = fps > 0 ? msToTime(1000.0 / fps) : 0;
    control.frameCount = 0;
    control.maxDeltaTime = maxDeltaTime;
#ifdef _WIN32
    // Request accuracy of the system timer. NOTE: This call must
    // be matched with timeEndPeriod(), with the same parameter.
    ensure(timeBeginPeriod(SYSTEM_TIMER_PERIOD) == TIMERR_NOERROR, "timeBeginPeriod() failed");
#endif
    control.started = 1;
}

void stopFrameControl()
{
    if (!control.started) {
        return;
    }
#ifdef _WIN32
    timeEndPeriod(SYSTEM_TIMER_PERIOD);
#endif
}

void waitForNextFrame()
{
    const Time nextFrameTime = control.prevFrameTime + control.framePeriod;
    Time currentTime = getCurrentTime();

    while (currentTime < nextFrameTime) {
        if (nextFrameTime - currentTime > control.timePerMs) {
            SDL_Delay(1);
        } else {
            // Just spin in the loop, because this can be more precise
            // than system delay
        }
        currentTime = getCurrentTime();
    }

    control.elapsedFrameTime = control.prevFrameTime ? currentTime - control.prevFrameTime : 0;
    control.prevFrameTime = currentTime;
    control.frameCount += 1;
}

double getElapsedFrameTime()
{
    const double elapsed = timeToMs(control.elapsedFrameTime);
    if (control.maxDeltaTime > 0 && elapsed > control.maxDeltaTime) {
        return control.maxDeltaTime;
    }
    return elapsed;
}

double getElapsedTime()
{
    return timeToMs(getCurrentTime() - control.startTime);
}

double getCurrentFps()
{
    return control.frameCount / (timeToMs(control.prevFrameTime - control.startTime) / 1000.0);
}
