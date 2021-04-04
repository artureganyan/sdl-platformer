/******************************************************************************
 * Copyright (c) Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#ifndef LEVELS_H
#define LEVELS_H

#include "types.h"

enum
{
    LEVEL_COUNTX = 2,
    LEVEL_COUNTY = 2
};

extern Level levels[LEVEL_COUNTY][LEVEL_COUNTX];

void initLevels();

#endif
