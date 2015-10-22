/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#include "helpers.h"


int isCellValid( int r, int c )
{
    return c >= 0 && c < COLUMN_COUNT && r >= 0 && r < ROW_COUNT;
}

int isSolid( int r, int c, int flags )
{
    return isCellValid(r, c) ? (level->map[r][c]->solid & flags) == flags : 0;
}

int isLadder( int r, int c )
{
    return isCellValid(r, c) ? level->map[r][c]->generalTypeId == TYPE_LADDER : 0;
}

// Returns 1 if there is a ladder or top-solid block at (r, c)
int isSolidOrLadder( int r, int c )
{
    return isCellValid(r, c) ? (level->map[r][c]->solid & SOLID_TOP) ||
            (level->map[r][c]->generalTypeId == TYPE_LADDER) : 0;
}

// Returns 1 if there is a ladder at (r, c) and player can stay on it
int isSolidLadder( int r, int c )
{
    return isLadder(r, c) && (isSolid(r, c - 1, SOLID_TOP) || isSolid(r, c + 1, SOLID_TOP) ||
           !isLadder(r - 1, c));
}

int isWater( int r, int c )
{
    return isCellValid(r, c) ? level->map[r][c]->generalTypeId == TYPE_WATER : 0;
}

int cellContains( int r, int c, ObjectTypeId generalType )
{
    return isCellValid(r, c) ? level->map[r][c]->generalTypeId == generalType : 0;
}

void getObjectCell( Object* object, int* r, int* c )
{
    *r = (object->y + CELL_HALF) / CELL_SIZE;
    *c = (object->x + CELL_HALF) / CELL_SIZE;
}

// cellBorders/bodyBorders = [left, right, top, bottom], for object cell/body
void getObjectPos( Object* object, int* r, int* c, int cellBorders[4], int bodyBorders[4] )
{
    const int dw = (CELL_SIZE - object->type->width) / 2;
    const int dh = (CELL_SIZE - object->type->height) / 2;

    getObjectCell(object, r, c);

    cellBorders[0] = CELL_SIZE * (*c);
    cellBorders[1] = CELL_SIZE * (*c + 1);
    cellBorders[2] = CELL_SIZE * (*r);
    cellBorders[3] = CELL_SIZE * (*r + 1);

    bodyBorders[0] = object->x + dw;
    bodyBorders[1] = object->x + CELL_SIZE - dw;
    bodyBorders[2] = object->y + dh;
    bodyBorders[3] = object->y + CELL_SIZE - dh;
}

int findNearDoor( int* r, int* c )
{
    const int r0 = *r;
    const int c0 = *c;
    if (cellContains(r0, c0, TYPE_DOOR)) {
        return 1;
    }
    if (cellContains(r0, c0 - 1, TYPE_DOOR)) {
        *c = c0 - 1;
        return 1;
    }
    if (cellContains(r0, c0 + 1, TYPE_DOOR)) {
        *c = c0 + 1;
        return 1;
    }
    return 0;
}

Object* findNearItem( int r, int c )
{
    for (int i = level->objects.count - 1; i >= 0; -- i) {
        Object* object = level->objects.array[i];
        if (object->type->generalTypeId == TYPE_ITEM && !object->removed) {
            int or, oc;
            getObjectCell(object, &or, &oc);
            if (or == r && oc == c) {
                return object;
            }
        }
    }
    return NULL;
}

Object* findObject( Level* level, ObjectTypeId typeId )
{
    for (int i = 0; i < level->objects.count; ++ i) {
        Object* object = level->objects.array[i];
        if (object->type->typeId == typeId) {
            return object;
        }
    }
    return NULL;
}
