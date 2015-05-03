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

int isSolid( int r, int c )
{
    return isCellValid(r, c) ? level->map[r][c]->solid : 0;
}

int isLadder( int r, int c )
{
    return isCellValid(r, c) ? level->map[r][c]->generalTypeId == TYPE_LADDER : 0;
}

int isSolidOrLadder( int r, int c )
{
    return isCellValid(r, c) ? level->map[r][c]->solid ||
            level->map[r][c]->generalTypeId == TYPE_LADDER : 0;
}

// Returns 1 if ladder is at (r, c) and player can stay on it
int isSolidLadder( int r, int c )
{
    return isLadder(r, c) && (isSolid(r, c - 1) || isSolid(r, c + 1) || !isLadder(r - 1, c));
}

int isWater( int r, int c )
{
    return isCellValid(r, c) ? level->map[r][c]->generalTypeId == TYPE_WATER : 0;
}

int cellContains( int r, int c, ObjectTypeId generalType )
{
    return isCellValid(r, c) ? level->map[r][c]->generalTypeId == generalType : 0;
}

void getObjectCell( Object* obj, int* r, int* c )
{
    *r = (obj->y + CELL_HALF) / CELL_SIZE;
    *c = (obj->x + CELL_HALF) / CELL_SIZE;
}

// cellBorders/bodyBorders = [left, right, top, bottom], for object cell/body
void getObjectPos( Object* obj, int* r, int* c, int cellBorders[4], int bodyBorders[4] )
{
    const int dw = (CELL_SIZE - obj->type->width) / 2;
    const int dh = (CELL_SIZE - obj->type->height) / 2;

    getObjectCell(obj, r, c);

    cellBorders[0] = CELL_SIZE * (*c);
    cellBorders[1] = CELL_SIZE * (*c + 1);
    cellBorders[2] = CELL_SIZE * (*r);
    cellBorders[3] = CELL_SIZE * (*r + 1);

    bodyBorders[0] = obj->x + dw;
    bodyBorders[1] = obj->x + CELL_SIZE - dw;
    bodyBorders[2] = obj->y + dh;
    bodyBorders[3] = obj->y + CELL_SIZE - dh;
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
