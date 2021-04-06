/******************************************************************************
 * Copyright (c) Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#include "types.h"
#include "render.h"
#include "objects.h"

enum { MIN_FRAME_RATE = 24 };
const double MAX_DELTA_TIME = 1000.0 / MIN_FRAME_RATE;
const double MAX_SPEED = MIN_FRAME_RATE * CELL_SIZE;

ObjectType objectTypes[TYPE_COUNT];


// ObjectArray

void ObjectArray_init( ObjectArray* objects )
{
    objects->reserved = 16;
    objects->count = 0;
    objects->array = (Object**)malloc(sizeof(Object*) * objects->reserved);
}

void ObjectArray_append( ObjectArray* objects, Object* object )
{
    if (objects->count == objects->reserved) {
        objects->reserved *= 2;
        objects->array = (Object**)realloc(objects->array, sizeof(Object*) * objects->reserved);
    }
    objects->array[objects->count ++] = object;
}

void ObjectArray_free( ObjectArray* objects )
{
    free(objects->array);
    objects->array = NULL;
    objects->reserved = 0;
    objects->count = 0;
}

void ObjectArray_clean( ObjectArray* objects )
{
    int r = 0;
    for (int i = 0; i < objects->count; ++ i) {
        Object* object = objects->array[i];
        if (object->removed == 1) {
            free(object);
            r += 1;
        } else if (object->removed == 2) {
            r += 1;
        } else if (r) {
            objects->array[i - r] = object;
        }
    }
    objects->count -= r;
}

static int compareByDepth( const void* object1, const void* object2 )
{
    return ((const Object*)object2)->type->typeId - ((const Object*)object1)->type->typeId;
}

void ObjectArray_sortByDepth( ObjectArray* objects )
{
    qsort(objects->array, objects->count, sizeof(Object*), compareByDepth);
}


// Object constructors

void createStaticObject( Level* level, ObjectTypeId typeId, int r, int c )
{
    level->cells[r][c] = &objectTypes[typeId];
}

Object* createObject( Level* level, ObjectTypeId typeId, int r, int c )
{
    Object* object = (Object*)malloc(sizeof(Object));
    initObject(object, typeId);
    object->x = CELL_SIZE * c;
    object->y = CELL_SIZE * r;
    ObjectArray_append(&level->objects, object);
    return object;
}

void initObject( Object* object, ObjectTypeId typeId )
{
    object->type = &objectTypes[typeId];
    object->x = 0;
    object->y = 0;
    object->vx = 0;
    object->vy = 0;
    object->removed = 0;
    object->state = 0;
    object->data = 0;
    object->anim.flip = SDL_FLIP_NONE;
    object->anim.frameDelayCounter = 0;
    object->anim.type = ANIMATION_FRAME;
    object->anim.alpha = 255;
    setAnimation(object, 0, 0, 0);
    object->type->onInit(object);
}

void initPlayer( Player* player )
{
    initObject((Object*)player, TYPE_PLAYER);
    player->inAir = 0;
    player->onLadder = 0;
    player->health = 100;
    player->invincibility = 0;
    player->lives = 3;
    player->coins = 0;
    player->keys = 0;
    ObjectArray_init(&player->items);
}

void initLevel( Level* level )
{
    for (int r = 0; r < ROW_COUNT; ++ r) {
        for (int c = 0; c < COLUMN_COUNT; ++ c) {
            level->cells[r][c] = &objectTypes[TYPE_NONE];
        }
    }
    level->init = 0;
    level->r = 0;
    level->c = 0;
    ObjectArray_init(&level->objects);
}


// Types

static void initTypeEx( ObjectTypeId typeId, ObjectTypeId generalTypeId, int solid,
                        int spriteRow, int spriteColumn, int spriteWidth, int spriteHeight,
                        SDL_Rect body, double speed, OnInit onInit, OnFrame onFrame, OnHit onHit )
{
    ObjectType* type = &objectTypes[typeId];
    type->typeId = typeId;
    type->generalTypeId = generalTypeId;
    type->sprite.y = spriteRow * SPRITE_SIZE;
    type->sprite.x = spriteColumn * SPRITE_SIZE;
    type->sprite.w = spriteWidth;
    type->sprite.h = spriteHeight;
    type->body = body;
    type->solid = solid;
    type->speed = speed;
    type->onInit = onInit;
    type->onFrame = onFrame;
    type->onHit = onHit;
}

static void initType( ObjectTypeId typeId, ObjectTypeId generalTypeId, int solid, int spriteRow, int spriteColumn )
{
    initTypeEx(typeId, generalTypeId, solid, spriteRow, spriteColumn, SPRITE_SIZE, SPRITE_SIZE,
               (SDL_Rect){0, 0, 16, 16}, 0, Object_onInit, Object_onFrame, Object_onHit);
}

void initTypes()
{
    //             type id            general type id         solid     sprite r, c, w, h              body             speed          onInit                 onFrame                     onHit
    initType    ( TYPE_NONE,            TYPE_NONE,              0,        0,  10                                                                                                                              );
    initTypeEx  ( TYPE_PLAYER,          TYPE_PLAYER,            0,        1,  26, 16, 16,  (SDL_Rect){6,  0,  4,  16},    0,       Object_onInit,          Object_onFrame,             Object_onHit           );
    initType    ( TYPE_WALL_TOP,        TYPE_WALL,          SOLID_ALL,    4,  6                                                                                                                               );
    initType    ( TYPE_WALL,            TYPE_WALL,          SOLID_ALL,    5,  6                                                                                                                               );
    initType    ( TYPE_WALL_FAKE,       TYPE_WALL_FAKE,         0,        5,  6                                                                                                                               );
    initTypeEx  ( TYPE_WALL_STAIR,      TYPE_WALL,          SOLID_TOP,    4,  6,  16, 8,   (SDL_Rect){0,  0,  16, 16},    0,       Object_onInit,          Object_onFrame,             Object_onHit           );
    initType    ( TYPE_GROUND_TOP,      TYPE_WALL,          SOLID_ALL,    6,  3                                                                                                                               );
    initType    ( TYPE_GROUND,          TYPE_WALL,          SOLID_ALL,    7,  3                                                                                                                               );
    initType    ( TYPE_GROUND_FAKE,     TYPE_GROUND_FAKE,       0,        7,  3                                                                                                                               );
    initTypeEx  ( TYPE_GROUND_STAIR,    TYPE_WALL,          SOLID_ALL,    6,  3,  16, 8,   (SDL_Rect){0,  0,  16, 16},    0,       Object_onInit,          Object_onFrame,             Object_onHit           );
    initTypeEx  ( TYPE_WATER_TOP,       TYPE_WATER,             0,        8,  0,  16, 16,  (SDL_Rect){0,  0,  16, 16},    0,       Water_onInit,           Object_onFrame,             Water_onHit            );
    initType    ( TYPE_WATER,           TYPE_WATER,             0,        9,  0                                                                                                                               );
    initType    ( TYPE_GRASS,           TYPE_BACKGROUND,        0,        40, 0                                                                                                                               );
    initType    ( TYPE_GRASS_BIG,       TYPE_BACKGROUND,        0,        40, 0                                                                                                                               );
    initType    ( TYPE_ROCK,            TYPE_BACKGROUND,    SOLID_ALL,    50, 0                                                                                                                               );
    initType    ( TYPE_SPIKE_TOP,       TYPE_SPIKE,             0,        48, 0                                                                                                                               );
    initType    ( TYPE_SPIKE_BOTTOM,    TYPE_SPIKE,             0,        49, 0                                                                                                                               );
    initType    ( TYPE_TREE1,           TYPE_BACKGROUND,        0,        41, 3                                                                                                                               );
    initType    ( TYPE_TREE2,           TYPE_BACKGROUND,        0,        41, 4                                                                                                                               );
    initTypeEx  ( TYPE_CLOUD1,          TYPE_PLATFORM,          0,        51, 6,  16, 16,  (SDL_Rect){0,  0, 16, 16},     0,       Object_onInit,          Object_onFrame,             Cloud_onHit            );
    initType    ( TYPE_CLOUD2,          TYPE_PLATFORM,          0,        51, 5                                                                                                                               );
    initType    ( TYPE_MUSHROOM1,       TYPE_BACKGROUND,        0,        47, 0                                                                                                                               );
    initType    ( TYPE_MUSHROOM2,       TYPE_BACKGROUND,        0,        47, 1                                                                                                                               );
    initType    ( TYPE_MUSHROOM3,       TYPE_BACKGROUND,        0,        47, 2                                                                                                                               );
    initType    ( TYPE_PILLAR_TOP,      TYPE_BACKGROUND,        0,        26, 2                                                                                                                               );
    initType    ( TYPE_PILLAR,          TYPE_BACKGROUND,        0,        27, 2                                                                                                                               );
    initType    ( TYPE_PILLAR_BOTTOM,   TYPE_BACKGROUND,        0,        28, 2                                                                                                                               );
    initTypeEx  ( TYPE_TORCH,           TYPE_BACKGROUND,        0,        62, 26, 16, 16,  (SDL_Rect){5,  0,  6,  6},     0,       Torch_onInit,           Object_onFrame,             Torch_onHit            );
    initType    ( TYPE_DOOR,            TYPE_DOOR,          SOLID_ALL,    10, 0                                                                                                                               );
    initType    ( TYPE_LADDER,          TYPE_LADDER,            0,        12, 2                                                                                                                               );
    initTypeEx  ( TYPE_GHOST,           TYPE_ENEMY,             0,        7,  26, 16, 16,  (SDL_Rect){2,  0,  12, 16},   24,       MovingEnemy_onInit,     ShootingEnemy_onFrame,      Object_onHit           );
    initTypeEx  ( TYPE_SCORPION,        TYPE_ENEMY,             0,        10, 26, 16, 16,  (SDL_Rect){3,  5,  10, 11},   24,       MovingEnemy_onInit,     MovingEnemy_onFrame,        MovingEnemy_onHit      );
    initTypeEx  ( TYPE_SPIDER,          TYPE_ENEMY,             0,        11, 26, 16, 16,  (SDL_Rect){3,  6,  10, 10},   24,       MovingEnemy_onInit,     Spider_onFrame,             MovingEnemy_onHit      );
    initTypeEx  ( TYPE_RAT,             TYPE_ENEMY,             0,        9,  26, 16, 16,  (SDL_Rect){2,  5,  12, 11},   24,       MovingEnemy_onInit,     MovingEnemy_onFrame,        MovingEnemy_onHit      );
    initTypeEx  ( TYPE_BAT,             TYPE_ENEMY,             0,        8,  26, 16, 16,  (SDL_Rect){0,  3,  16, 10},   48,       Bat_onInit,             Bat_onFrame,                Bat_onHit              );
    initTypeEx  ( TYPE_BLOB,            TYPE_ENEMY,             0,        61, 26, 16, 16,  (SDL_Rect){3,  6,  10, 10},   24,       MovingEnemy_onInit,     MovingEnemy_onFrame,        MovingEnemy_onHit      );
    initTypeEx  ( TYPE_FIREBALL,        TYPE_ENEMY,             0,        13, 26, 16, 16,  (SDL_Rect){2,  3,  14, 12},   48,       Fireball_onInit,        Fireball_onFrame,           Bat_onHit              );
    initTypeEx  ( TYPE_SKELETON,        TYPE_ENEMY,             0,        6,  26, 16, 16,  (SDL_Rect){1,  0,  14, 16},   24,       MovingEnemy_onInit,     TeleportingEnemy_onFrame,   TeleportingEnemy_onHit );
    initTypeEx  ( TYPE_ICESHOT,         TYPE_ENEMY,             0,        52, 0,  16, 16,  (SDL_Rect){0,  4,  16, 7},   168,       Shot_onInit,            Shot_onFrame,               Shot_onHit             );
    initTypeEx  ( TYPE_FIRESHOT,        TYPE_ENEMY,             0,        60, 26, 16, 16,  (SDL_Rect){6,  6,  4,  4},   120,       Shot_onInit,            Shot_onFrame,               Shot_onHit             );
    initTypeEx  ( TYPE_DROP,            TYPE_DROP,              0,        37, 43, 16, 16,  (SDL_Rect){6,  6,  4,  4},     0,       Drop_onInit,            Drop_onFrame,               Drop_onHit             );
    initTypeEx  ( TYPE_PLATFORM,        TYPE_PLATFORM,          0,        4,  6,  16, 8,   (SDL_Rect){0,  0,  16, 8},    48,       Platform_onInit,        Platform_onFrame,           Platform_onHit         );
    initTypeEx  ( TYPE_SPRING,          TYPE_SPRING,            0,        65, 26, 16, 16,  (SDL_Rect){0,  8,  16, 8},     0,       Spring_onInit,          Spring_onFrame,             Spring_onHit           );
    initTypeEx  ( TYPE_ARROW_LEFT,      TYPE_WALL,          SOLID_LEFT,   32, 3,  16, 16,  (SDL_Rect){0,  0,  16, 16},    0,       Object_onInit,          Object_onFrame,             Object_onHit           );
    initTypeEx  ( TYPE_ARROW_RIGHT,     TYPE_WALL,          SOLID_RIGHT,  31, 3,  16, 16,  (SDL_Rect){0,  0,  16, 16},    0,       Object_onInit,          Object_onFrame,             Object_onHit           );
    initTypeEx  ( TYPE_KEY,             TYPE_KEY,               0,        45, 26, 16, 16,  (SDL_Rect){0,  0,  16, 16},    0,       Object_onInit,          Item_onFrame,               Item_onHit             );
    initTypeEx  ( TYPE_COIN,            TYPE_COIN,              0,        63, 26, 16, 16,  (SDL_Rect){0,  0,  16, 16},    0,       Object_onInit,          Item_onFrame,               Item_onHit             );
    initTypeEx  ( TYPE_GEM,             TYPE_COIN,              0,        50, 32, 16, 16,  (SDL_Rect){0,  0,  16, 16},    0,       Object_onInit,          Item_onFrame,               Item_onHit             );
    initTypeEx  ( TYPE_APPLE,           TYPE_ITEM,              0,        15, 26, 16, 16,  (SDL_Rect){0,  0,  16, 16},    0,       Object_onInit,          Item_onFrame,               Item_onHit             );
    initTypeEx  ( TYPE_PEAR,            TYPE_ITEM,              0,        15, 27, 16, 16,  (SDL_Rect){0,  0,  16, 16},    0,       Object_onInit,          Item_onFrame,               Item_onHit             );
    initTypeEx  ( TYPE_STATUARY,        TYPE_STATUARY,          0,        52, 27, 16, 16,  (SDL_Rect){0,  0,  16, 16},    0,       Object_onInit,          Item_onFrame,               Item_onHit             );
    initTypeEx  ( TYPE_LADDER_PART,     TYPE_ITEM,              0,        62, 29, 16, 16,  (SDL_Rect){0,  0,  16, 16},    0,       Object_onInit,          Item_onFrame,               Item_onHit             );
    initTypeEx  ( TYPE_PICK,            TYPE_ITEM,              0,        62, 30, 16, 16,  (SDL_Rect){0,  0,  16, 16},    0,       Object_onInit,          Item_onFrame,               Item_onHit             );
    initTypeEx  ( TYPE_HEART,           TYPE_HEART,             0,        62, 31, 16, 16,  (SDL_Rect){4,  4,  8,  8},     0,       Object_onInit,          Item_onFrame,               Item_onHit             );
    initType    ( TYPE_ACTION,          TYPE_ITEM,              0,        0,  10                                                                                                                              );
}
