/******************************************************************************
 * Copyright (c) Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#ifndef TYPES_H
#define TYPES_H

#include "SDL.h"

typedef enum
{
    LEVEL_WIDTH = 320,
    LEVEL_HEIGHT = 240,
    SPRITE_SIZE = 16,
    SIZE_FACTOR = 2,
    CELL_SIZE = SPRITE_SIZE,
    CELL_HALF = CELL_SIZE / 2,
    ROW_COUNT = (LEVEL_HEIGHT + CELL_SIZE - 1) / CELL_SIZE,
    COLUMN_COUNT = (LEVEL_WIDTH + CELL_SIZE - 1) / CELL_SIZE,
    CELL_COUNT = ROW_COUNT * COLUMN_COUNT,
    FRAME_RATE = 48,
    FRAME_PERIOD = 1000 / FRAME_RATE,
    PLAYER_WIDTH = CELL_SIZE - 12,
    PLAYER_HEIGHT = CELL_SIZE,
#ifdef _WIN32
    SYSTEM_TIMER_PERIOD = 1 // In ms, used to update frames
#endif
} Constant;

typedef enum
{
    TYPE_NONE = 0,

    TYPE_GHOST,
    TYPE_SCORPION,
    TYPE_SPIDER,
    TYPE_RAT,
    TYPE_BAT,
    TYPE_BLOB,
    TYPE_FIREBALL,
    TYPE_SKELETON,
    TYPE_ICESHOT,
    TYPE_FIRESHOT,
    TYPE_DROP,
    TYPE_PLATFORM,
    TYPE_CLOUD1,
    TYPE_CLOUD2,
    TYPE_WALL_FAKE,
    TYPE_GROUND_FAKE,
    TYPE_PLAYER,

    TYPE_HEART,
    TYPE_WALL_TOP,
    TYPE_WALL,
    TYPE_WALL_STAIR,
    TYPE_GROUND_TOP,
    TYPE_GROUND,
    TYPE_GROUND_STAIR,
    TYPE_WATER,
    TYPE_WATER_TOP,
    TYPE_GRASS,
    TYPE_GRASS_BIG,
    TYPE_ROCK,
    TYPE_SPIKE_TOP,
    TYPE_SPIKE_BOTTOM,
    TYPE_TREE1,
    TYPE_TREE2,
    TYPE_MUSHROOM1,
    TYPE_MUSHROOM2,
    TYPE_MUSHROOM3,
    TYPE_PILLAR,
    TYPE_PILLAR_TOP,
    TYPE_PILLAR_BOTTOM,
    TYPE_TORCH,
    TYPE_DOOR,
    TYPE_LADDER,
    TYPE_SPRING,
    TYPE_FAN,
    TYPE_ARROW_LEFT,
    TYPE_ARROW_RIGHT,
    TYPE_ARROW_UP,
    TYPE_ARROW_DOWN,

    TYPE_KEY,
    TYPE_APPLE,
    TYPE_PEAR,
    TYPE_COIN,
    TYPE_GEM,
    TYPE_STATUARY,
    TYPE_LADDER_PART,
    TYPE_PICK,
    TYPE_ACTION,

    TYPE_COUNT, // Must always be last

    // General types (objects of these types can not be created)
    TYPE_ENEMY,
    TYPE_ITEM,
    TYPE_BACKGROUND,
    TYPE_SPIKE
} ObjectTypeId;

typedef enum
{
    SOLID_LEFT = 1,
    SOLID_RIGHT = 2,
    SOLID_TOP = 4,
    SOLID_BOTTOM = 8,
    SOLID_ALL = SOLID_LEFT | SOLID_RIGHT | SOLID_TOP | SOLID_BOTTOM
} SolidFlags;

typedef struct
{
    double left;
    double right;
    double top;
    double bottom;
} Borders;

struct Object_s;
typedef struct Object_s Object;
typedef void (*OnInit)( Object* );
typedef void (*OnFrame)( Object* );
typedef void (*OnHit)( Object* );

typedef struct
{
    ObjectTypeId typeId;
    ObjectTypeId generalTypeId;
    SDL_Rect sprite; // Sprite rect in the spritesheet, unscaled
    int width;       // Body width/height, unscaled. Body is centered within
    int height;      // the sprite.
    int solid;
    double speed;
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
    double frameDelay;          // Seconds
    double frameDelayCounter;   // Seconds
    int wave;
    int alpha;
} Animation;

typedef struct Object_s
{
    ObjectType* type;
    Animation anim;
    double x;
    double y;
    double vx;      // Pixels per second
    double vy;      // Pixels per second
    int removed;
    int state;
    int data;
} Object;

typedef struct
{
    Object** array;
    int reserved;
    int count;
} ObjectArray;

typedef struct
{
    ObjectType* type;
    Animation anim;
    double x;
    double y;
    double vx;
    double vy;
    int removed;        // Unused
    int state;          // Unused
    int data;           // Unused
    int inAir;
    int onLadder;
    int health;
    int invincibility;  // How long player is invincibile, in ms
    int lives;
    int coins;
    int keys;
    ObjectArray items;
} Player;

typedef struct
{
    ObjectType* cells[ROW_COUNT][COLUMN_COUNT];
    ObjectArray objects;
    int r;
    int c;
    void (*init)();
} Level;

void ObjectArray_init( ObjectArray* objects );
void ObjectArray_append( ObjectArray* objects, Object* object );
void ObjectArray_free( ObjectArray* objects );
void ObjectArray_clean( ObjectArray* objects );
void ObjectArray_sortByDepth( ObjectArray* objects );

void createStaticObject( Level* level, ObjectTypeId typeId, int r, int c );
Object* createObject( Level* level, ObjectTypeId typeId, int r, int c );
void initObject( Object* object, ObjectTypeId typeId );
void initPlayer( Player* player );
void initLevel( Level* level );
void initTypes();

extern ObjectType objectTypes[TYPE_COUNT];

#endif
