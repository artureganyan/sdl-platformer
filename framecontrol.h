/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#ifndef FRAMECONTROL_H
#define FRAMECONTROL_H

#ifdef _MSC_VER
#include <windows.h>
#else
#include <time.h>
#endif

typedef struct
{
#ifdef _MSC_VER
    double cpuFrequency;    // Ticks per second divided by 1000
    LONGLONG startTime;     // Ticks
#else
    double startTime;       // Milliseconds
#endif
    double prevFrameTime;   // Milliseconds
    double framePeriod;     // Milliseconds
    unsigned long frameCount;
} FrameControl;

int FrameControl_start( FrameControl* c, int fps );
void FrameControl_waitNextFrame( FrameControl* c );
double FrameControl_getElapsedTime( FrameControl* c ); // Milliseconds
double FrameControl_getRealFps( FrameControl* c );

#endif
