/******************************************************************************
 * Copyright (c) Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#ifndef GAME_H
#define GAME_H

#include "types.h"

extern Level* level;
extern Player player;

void initGame();
void gameLoop();
void setLevel( int r, int c );
void completeLevel();
void damagePlayer( int damage );
void killPlayer();
void showMessage( const char* message );

#endif
