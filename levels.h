/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#ifndef LEVELS_H
#define LEVELS_H

#include "types.h"

enum {
    LEVEL_XCOUNT = 4,
    LEVEL_YCOUNT = 3
};

extern Level levels[LEVEL_YCOUNT][LEVEL_XCOUNT];
extern Level* level;

void initLevel( Level* level );
void initLevels();
void setLevel( int r, int c );

#endif
