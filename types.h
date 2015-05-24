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
    BAR_HEIGHT = 32,
    SIZE_FACTOR = 2,
    CELL_SIZE = SPRITE_SIZE * SIZE_FACTOR,
    CELL_HALF = CELL_SIZE / 2,
    ROW_COUNT = (LEVEL_HEIGHT + CELL_SIZE - 1) / CELL_SIZE,
    COLUMN_COUNT = (LEVEL_WIDTH + CELL_SIZE - 1) / CELL_SIZE,
    CELL_COUNT = ROW_COUNT * COLUMN_COUNT,
    FRAME_RATE = 48,
    FRAME_PERIOD = 1000 / FRAME_RATE,
    PLAYER_WIDTH = CELL_SIZE - 12,
    PLAYER_HEIGHT = CELL_SIZE, // -10
    SYSTEM_TIMER_PERIOD = 1 // In ms, used to update frames
} Constant;

typedef enum
{
    TYPE_NONE = 0,

    TYPE_TOPOBJECTS,
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
    TYPE_HEART,
    TYPE_WALL_FAKE,
    TYPE_GROUND_FAKE,

    TYPE_BACKOBJECTS,
    TYPE_PLAYER,
    TYPE_WALL_TOP,
    TYPE_WALL,
    TYPE_GROUND_TOP,
    TYPE_GROUND,
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

    TYPE_ITEMS,
    TYPE_KEY,
    TYPE_APPLE,
    TYPE_PEAR,
    TYPE_COIN,
    TYPE_LADDER_PART,
    TYPE_PICK,
    TYPE_ACTION,
    TYPE_COUNT, // Must always be last

    // General types (objects of these types can not be created)
    TYPE_ENEMY,
    TYPE_BACKGROUND,
    TYPE_SPIKE
} ObjectTypeId;

typedef enum
{
    MESSAGE_NONE = 0,
    MESSAGE_GAMEOVER,
    MESSAGE_NOITEMS,
    MESSAGE_CANNOTUSE,
    MESSAGE_LOSTLIFE,
    MESSAGE_TEST,
    MESSAGE_TEXT,
    MESSAGE_COUNT
} Message;

struct Object_s;
typedef struct Object_s Object;
typedef void (*OnInit)( Object* );
typedef void (*OnFrame)( Object* );
typedef void (*OnHit)( Object* );

typedef struct
{
    ObjectTypeId typeId;
    ObjectTypeId generalTypeId;
    SDL_Rect sprite;
    SDL_Texture* nameTexture;
    const char* name;
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
    int state;
} Object;

typedef struct {
    Object** array;
    int reserved;
    int count;
} ObjectArray;

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
    int health;
    int lives;
    int coins;
    ObjectArray items;
} Player;

typedef struct {
    ObjectType* map[ROW_COUNT][COLUMN_COUNT];
    ObjectArray objects;
    SDL_Texture* nameTexture;
    const char* name;
    int background;
    int r;
    int c;
    void (*init)();
} Level;

void initArray( ObjectArray* objects );
void appendArray( ObjectArray* objects, Object* obj );
void freeArray( ObjectArray* objects );
void cleanArray( ObjectArray* objects );
void reorderDepth( ObjectArray* objects );

void createObjectInMap( Level* level, ObjectTypeId typeId, int r, int c );
Object* createObject( Level* level, ObjectTypeId typeId, int r, int c );
void initTypes();

#define MS_TO_FRAMES(ms) (int)((ms) / 1000.0 * FRAME_RATE)

extern ObjectType objectTypes[TYPE_COUNT];
extern const char* messages[MESSAGE_COUNT];
extern Player player;
extern Level* level;

#endif
