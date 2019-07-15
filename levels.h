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
    LEVEL_XCOUNT = 2,
    LEVEL_YCOUNT = 2
};

extern Level levels[LEVEL_YCOUNT][LEVEL_XCOUNT];

void initLevels();

#endif
