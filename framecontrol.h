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
    double cpuFrequency;
    double framePeriod;
    double prevFrameTime;
#ifdef _MSC_VER
    LONGLONG startTime; // Ticks
#else
    double startTime;   // Seconds
#endif
    unsigned long frameCount;
} FrameControl;

int FrameControl_start( FrameControl*, int fps );
void FrameControl_waitNextFrame( FrameControl* );
double FrameControl_getElapsedTime( FrameControl* ); // Milliseconds
double FrameControl_getRealFps( FrameControl* );

#endif
