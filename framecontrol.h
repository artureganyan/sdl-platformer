/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#ifndef FRAMECONTROL_H
#define FRAMECONTROL_H

#include <windows.h>

typedef struct
{
    double cpuFrequency;
    double framePeriod;
    double prevFrameTime;
    LONGLONG startTime;
    unsigned long frameCount;
} FrameControl;

int FrameControl_start( FrameControl*, int fps );
void FrameControl_waitNextFrame( FrameControl* );
double FrameControl_getElapsedTime( FrameControl* );
double FrameControl_getRealFps( FrameControl* );

#endif
