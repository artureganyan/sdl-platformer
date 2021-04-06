/******************************************************************************
 * Copyright (c) Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#ifndef HELPERS_H
#define HELPERS_H

#include "types.h"

int isCellValid( int r, int c );
int isSolid( int r, int c, int flags );
int isLadder( int r, int c );
int isSolidLadder( int r, int c );
int isWater( int r, int c );
int cellContains( int r, int c, ObjectTypeId generalType );
int hitTest( Object* object1, Object* object2 );

void getObjectCell( Object* object, int* r, int* c );
void getObjectBody( Object* object, Borders* body );
void getObjectPos( Object* object, int* r, int* c, Borders* cell, Borders* body );

int findNearDoor( int* r, int* c );
Object* findNearItem( int r, int c );
Object* findObject( Level* level, ObjectTypeId typeId );

double limitAbs(double value, double max);
void ensure(int condition, const char* message);

#endif
