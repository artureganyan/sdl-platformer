/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#ifndef GAME_H
#define GAME_H

#include "types.h"

void processPlayer();
void processObjects();
void gameLoop();
void damagePlayer( int damage );
void killPlayer();

#endif
