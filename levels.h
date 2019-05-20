/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#ifndef LEVELS_H
#define LEVELS_H

#include "types.h"

enum
{
    LEVEL_XCOUNT = 2,
    LEVEL_YCOUNT = 2
};

typedef enum
{
    ACTION_TAKE,
    ACTION_USE
} Action;

void initLevel( Level* );
void initLevels();
void setLevel( int r, int c );

extern Level levels[LEVEL_YCOUNT][LEVEL_XCOUNT];
extern Level* level;

#endif
