/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#ifndef TYPES_H
#define TYPES_H

#include "SDL.h"

typedef enum
{
    SPRITE_SIZE = 16,
    LEVEL_WIDTH = 640,
    LEVEL_HEIGHT = 480,
    CELL_SIZE = SPRITE_SIZE * 2,
    CELL_HALF = CELL_SIZE / 2,
    ROW_COUNT = (LEVEL_HEIGHT + CELL_SIZE - 1) / CELL_SIZE,
    COLUMN_COUNT = (LEVEL_WIDTH + CELL_SIZE - 1) / CELL_SIZE,
    CELL_COUNT = ROW_COUNT * COLUMN_COUNT,
    FRAME_PERIOD = 1000 / 48,
    PLAYER_WIDTH = CELL_SIZE - 12,
    PLAYER_HEIGHT = CELL_SIZE - 10,
    SYSTEM_TIMER_PERIOD = 1 // In ms, used to update frames
} Constant;

typedef enum
{
    TYPE_NONE = 0,
    TYPE_PLAYER,
    TYPE_GHOST,
    TYPE_SCORPION,
    TYPE_BAT,
    TYPE_FIREBALL,
    TYPE_WALL_TOP,
    TYPE_WALL,
    TYPE_PILLAR,
    TYPE_PILLAR_TOP,
    TYPE_PILLAR_BOTTOM,
    TYPE_DOOR,
    TYPE_LADDER,
    TYPE_KEY,
    TYPE_COIN,
    TYPE_ICESHOT,
    TYPE_FIRESHOT,
    TYPE_DROP,
    TYPE_DROPSOURCE,
    TYPE_COUNT, // Must always be last

    // General types (objects of these types can not be created)
    TYPE_ENEMY,
    TYPE_BACKGROUND
} ObjectTypeId;

struct Object_s;
typedef struct Object_s Object;
typedef void (*OnInit)( Object* );
typedef void (*OnFrame)( Object* );
typedef void (*OnHit)( Object*, Object* /*target*/ );

typedef struct
{
    ObjectTypeId typeId;
    ObjectTypeId generalTypeId;
    SDL_Rect sprite;
    int solid;
    int speed;
    int width;
    int height;
    OnInit onInit;
    OnFrame onFrame;
    OnHit onHit;
} ObjectType;

typedef struct
{
    int direction;
    int frame;
    int frameStart;
    int frameEnd;
    int frameDelay; // In game frames
    int frameDelayCounter;
} Animation;

typedef struct Object_s
{
    ObjectType* type;
    Animation anim;
    int x, y;
    int vx, vy;
    int removed;
    int attack;
} Object;

typedef struct
{
    ObjectType* type;
    Animation anim;
    int x, y;
    int vx, vy;
    int removed; // Unused
    int attack;
    int inAir;
    int onLadder;
    int lives;
    int coins;
    int keys;
} Player;

typedef struct {
    Object** array;
    int reserved;
    int count;
} ObjectArray;

typedef struct {
    ObjectType* map[ROW_COUNT][COLUMN_COUNT];
    ObjectArray objects;
    SDL_Texture* texture;
    int coins;
    int r;
    int c;
} Level;


void initArray( ObjectArray* objects );
void appendArray( ObjectArray* objects, Object* obj );
void freeArray( ObjectArray* objects );
void cleanArray( ObjectArray* objects );

void createObjectInMap( Level* level, ObjectTypeId typeId, int r, int c );
Object* createObject( Level* level, ObjectTypeId typeId, int r, int c );
void initTypes();

extern ObjectType objectTypes[TYPE_COUNT];
extern Player player;
extern Level* level;

#endif
