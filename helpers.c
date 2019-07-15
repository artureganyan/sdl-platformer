/******************************************************************************
 * Copyright (c) Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#include "helpers.h"
#include "game.h"


int isCellValid( int r, int c )
{
    return c >= 0 && c < COLUMN_COUNT && r >= 0 && r < ROW_COUNT;
}

int isSolid( int r, int c, int flags )
{
    return isCellValid(r, c) ? (level->cells[r][c]->solid & flags) == flags : 0;
}

int isLadder( int r, int c )
{
    return isCellValid(r, c) ? level->cells[r][c]->generalTypeId == TYPE_LADDER : 0;
}

// Returns 1 if there is a ladder or top-solid block at (r, c)
int isSolidOrLadder( int r, int c )
{
    return isCellValid(r, c) ? (level->cells[r][c]->solid & SOLID_TOP) ||
            (level->cells[r][c]->generalTypeId == TYPE_LADDER) : 0;
}

// Returns 1 if there is a ladder at (r, c) and player can stay on it
int isSolidLadder( int r, int c )
{
    return isLadder(r, c) && (isSolid(r, c - 1, SOLID_TOP) || isSolid(r, c + 1, SOLID_TOP) ||
           !isLadder(r - 1, c));
}

int isWater( int r, int c )
{
    return isCellValid(r, c) ? level->cells[r][c]->generalTypeId == TYPE_WATER : 0;
}

int cellContains( int r, int c, ObjectTypeId generalType )
{
    return isCellValid(r, c) ? level->cells[r][c]->generalTypeId == generalType : 0;
}

void getObjectCell( Object* object, int* r, int* c )
{
    *r = (object->y + object->type->sprite.h * SIZE_FACTOR / 2) / CELL_SIZE;
    *c = (object->x + object->type->sprite.w * SIZE_FACTOR / 2) / CELL_SIZE;
}

void getObjectPos( Object* object, int* r, int* c, Borders* cell, Borders* body )
{
    const int dw = (CELL_SIZE - object->type->width) / 2;
    const int dh = (CELL_SIZE - object->type->height) / 2;

    getObjectCell(object, r, c);

    cell->left = CELL_SIZE * (*c);
    cell->right = cell->left + CELL_SIZE;
    cell->top = CELL_SIZE * (*r);
    cell->bottom = cell->top + CELL_SIZE;

    body->left = object->x + dw;
    body->right = object->x + CELL_SIZE - dw;
    body->top = object->y + dh;
    body->bottom = object->y + CELL_SIZE - dh;
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
