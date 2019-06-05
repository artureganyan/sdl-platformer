/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#ifndef FRAMECONTROL_H
#define FRAMECONTROL_H

int startFrameControl( int fps );
void waitForNextFrame();
double getElapsedTime(); // Milliseconds
double getCurrentFps();

#endif
