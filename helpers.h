/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#ifndef HELPERS_H
#define HELPERS_H

#include "types.h"

int isCellValid( int r, int c );
int isSolid( int r, int c );
int isLadder( int r, int c );
int isSolidOrLadder( int r, int c );
int isSolidLadder( int r, int c );
int isWater( int r, int c );
int cellContains( int r, int c, ObjectTypeId generalType );

void getObjectCell( Object* obj, int* r, int* c );
void getObjectPos( Object* obj, int* r, int* c, int cellBorders[4], int bodyBorders[4] );
int findNearDoor( int* r, int* c );

#endif
