/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#ifndef HELPERS_H
#define HELPERS_H

#include "types.h"

int isCellValid( int r, int c );
int isSolid( int r, int c, int flags );
int isLadder( int r, int c );
int isSolidOrLadder( int r, int c );
int isSolidLadder( int r, int c );
int isWater( int r, int c );
int cellContains( int r, int c, ObjectTypeId generalType );

void getObjectCell( Object* object, int* r, int* c );
void getObjectPos( Object* object, int* r, int* c, Borders* cell, Borders* body );

int findNearDoor( int* r, int* c );
Object* findNearItem( int r, int c );
Object* findObject( Level* level, ObjectTypeId typeId );

#endif
