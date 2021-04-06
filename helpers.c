/******************************************************************************
 * Copyright (c) Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#include "helpers.h"
#include "game.h"
#include "levels.h"


int isCellValid( int r, int c )
{
    return r >= 0 && r < ROW_COUNT && c >= 0 && c < COLUMN_COUNT;
}

int isSolid( int r, int c, int flags )
{
    return isCellValid(r, c) ? (level->cells[r][c]->solid & flags) == flags : 0;
}

int isLadder( int r, int c )
{
    return isCellValid(r, c) ? level->cells[r][c]->generalTypeId == TYPE_LADDER : 0;
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

int hitTest( Object* object1, Object* object2 )
{
    const SDL_Rect o1 = object1->type->body;
    const SDL_Rect o2 = object2->type->body;
    if (fabs((object1->x + o1.x + o1.w / 2.0) - (object2->x + o2.x + o2.w / 2.0)) < (o1.w + o2.w) / 2.0 &&
        fabs((object1->y + o1.y + o1.h / 2.0) - (object2->y + o2.y + o2.h / 2.0)) < (o1.h + o2.h) / 2.0) {
        return 1;
    }
    return 0;
}

void getObjectCell( Object* object, int* r, int* c )
{
    const SDL_Rect body = object->type->body;
    *r = (object->y + body.y + body.h / 2.0) / CELL_SIZE;
    *c = (object->x + body.x + body.w / 2.0) / CELL_SIZE;
}

void getObjectBody( Object* object, Borders* borders )
{
    const SDL_Rect body = object->type->body;
    borders->left = object->x + body.x;
    borders->right = borders->left + body.w;
    borders->top = object->y + body.y;
    borders->bottom = borders->top + body.h;
}

void getObjectPos( Object* object, int* r, int* c, Borders* cell, Borders* body )
{
    getObjectCell(object, r, c);
    getObjectBody(object, body);

    cell->left = CELL_SIZE * (*c);
    cell->right = cell->left + CELL_SIZE;
    cell->top = CELL_SIZE * (*r);
    cell->bottom = cell->top + CELL_SIZE;
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

double limitAbs(double value, double max)
{
    return value >  max ?  max :
           value < -max ? -max :
           value;
}

void ensure(int condition, const char* message)
{
    if (!condition) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", message, NULL);
        exit(EXIT_FAILURE);
    }
}
