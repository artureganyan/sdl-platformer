/******************************************************************************
 * Copyright (c) Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#ifndef FRAMECONTROL_H
#define FRAMECONTROL_H

void startFrameControl( int fps );
void waitForNextFrame();
double getElapsedFrameTime();   // ms
double getElapsedTime();        // ms
double getCurrentFps();

#endif
