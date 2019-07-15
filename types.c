/******************************************************************************
 * Copyright (c) Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#include "types.h"
#include "render.h"
#include "objects.h"

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
    object->anim.direction = 1;
    object->anim.frameDelayCounter = 0;
    object->anim.wave = 0;
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

void initTypeEx( ObjectTypeId typeId, int spriteRow, int spriteColumn, ObjectTypeId generalTypeId, int solid,
                 OnInit onInit, OnFrame onFrame, OnHit onHit )
{
    ObjectType* type = &objectTypes[typeId];
    type->typeId = typeId;
    type->generalTypeId = generalTypeId;
    type->sprite.y = spriteRow * SPRITE_SIZE;
    type->sprite.x = spriteColumn * SPRITE_SIZE;
    type->sprite.w = SPRITE_SIZE;
    type->sprite.h = SPRITE_SIZE;
    type->solid = solid;
    type->speed = 0;
    type->width = SPRITE_SIZE;
    type->height = SPRITE_SIZE;
    type->onInit = onInit;
    type->onFrame = onFrame;
    type->onHit = onHit;
}

void initType( ObjectTypeId typeId, int spriteRow, int spriteColumn, ObjectTypeId generalTypeId, int solid )
{
    initTypeEx(typeId, spriteRow, spriteColumn, generalTypeId, solid,
               Object_onInit, Object_onFrame, Object_onHit);
}

void initTypes()
{
    //             type id             sprite r, c    general type id         solid             onInit                 onFrame                  onHit
    initType    ( TYPE_NONE,            0,      10,     TYPE_NONE,              0                                                                                       );
    initType    ( TYPE_PLAYER,          1,      26,     TYPE_PLAYER,            0                                                                                       );
    initType    ( TYPE_WALL_TOP,        4,      6,      TYPE_WALL,          SOLID_ALL                                                                                   );
    initType    ( TYPE_WALL,            5,      6,      TYPE_WALL,          SOLID_ALL                                                                                   );
    initType    ( TYPE_WALL_FAKE,       5,      6,      TYPE_WALL_FAKE,         0                                                                                       );
    initType    ( TYPE_WALL_STAIR,      4,      6,      TYPE_WALL,          SOLID_TOP                                                                                   );
    initType    ( TYPE_GROUND_TOP,      6,      3,      TYPE_WALL,          SOLID_ALL                                                                                   );
    initType    ( TYPE_GROUND,          7,      3,      TYPE_WALL,          SOLID_ALL                                                                                   );
    initType    ( TYPE_GROUND_FAKE,     7,      3,      TYPE_GROUND_FAKE,       0                                                                                       );
    initType    ( TYPE_GROUND_STAIR,    6,      3,      TYPE_WALL,          SOLID_ALL                                                                                   );
    initTypeEx  ( TYPE_WATER_TOP,       8,      0,      TYPE_WATER,             0,          Water_onInit,           Object_onFrame,         Water_onHit                 );
    initType    ( TYPE_WATER,           9,      0,      TYPE_WATER,             0                                                                                       );
    initType    ( TYPE_GRASS,           40,     0,      TYPE_BACKGROUND,        0                                                                                       );
    initType    ( TYPE_GRASS_BIG,       40,     0,      TYPE_BACKGROUND,        0                                                                                       );
    initType    ( TYPE_ROCK,            50,     0,      TYPE_BACKGROUND,    SOLID_ALL                                                                                   );
    initType    ( TYPE_SPIKE_TOP,       48,     0,      TYPE_SPIKE,             0                                                                                       );
    initType    ( TYPE_SPIKE_BOTTOM,    49,     0,      TYPE_SPIKE,             0                                                                                       );
    initType    ( TYPE_TREE1,           41,     3,      TYPE_BACKGROUND,        0                                                                                       );
    initType    ( TYPE_TREE2,           41,     4,      TYPE_BACKGROUND,        0                                                                                       );
    initTypeEx  ( TYPE_CLOUD1,          51,     6,      TYPE_PLATFORM,          0,          Object_onInit,          Object_onFrame,         Cloud_onHit                 );
    initType    ( TYPE_CLOUD2,          51,     5,      TYPE_PLATFORM,          0                                                                                       );
    initType    ( TYPE_MUSHROOM1,       47,     0,      TYPE_BACKGROUND,        0                                                                                       );
    initType    ( TYPE_MUSHROOM2,       47,     1,      TYPE_BACKGROUND,        0                                                                                       );
    initType    ( TYPE_MUSHROOM3,       47,     2,      TYPE_BACKGROUND,        0                                                                                       );
    initType    ( TYPE_PILLAR_TOP,      26,     2,      TYPE_BACKGROUND,        0                                                                                       );
    initType    ( TYPE_PILLAR,          27,     2,      TYPE_BACKGROUND,        0                                                                                       );
    initType    ( TYPE_PILLAR_BOTTOM,   28,     2,      TYPE_BACKGROUND,        0                                                                                       );
    initTypeEx  ( TYPE_TORCH,           62,     26,     TYPE_BACKGROUND,        0,          Torch_onInit,           Object_onFrame,         Torch_onHit                 );
    initType    ( TYPE_DOOR,            10,     0,      TYPE_DOOR,          SOLID_ALL                                                                                   );
    initType    ( TYPE_LADDER,          12,     2,      TYPE_LADDER,            0                                                                                       );
    initTypeEx  ( TYPE_GHOST,           7,      26,     TYPE_ENEMY,             0,          Enemy_onInit,           ShooterEnemy_onFrame,       Object_onHit            );
    initTypeEx  ( TYPE_SCORPION,        10,     26,     TYPE_ENEMY,             0,          Enemy_onInit,           Enemy_onFrame,              Enemy_onHit             );
    initTypeEx  ( TYPE_SPIDER,          11,     26,     TYPE_ENEMY,             0,          Enemy_onInit,           Spider_onFrame,             Enemy_onHit             );
    initTypeEx  ( TYPE_RAT,             9,      26,     TYPE_ENEMY,             0,          Enemy_onInit,           Enemy_onFrame,              Enemy_onHit             );
    initTypeEx  ( TYPE_BAT,             8,      26,     TYPE_ENEMY,             0,          Bat_onInit,             Bat_onFrame,                Bat_onHit               );
    initTypeEx  ( TYPE_BLOB,            61,     26,     TYPE_ENEMY,             0,          Enemy_onInit,           Enemy_onFrame,              Enemy_onHit             );
    initTypeEx  ( TYPE_FIREBALL,        13,     26,     TYPE_ENEMY,             0,          Fireball_onInit,        Fireball_onFrame,           Bat_onHit               );
    initTypeEx  ( TYPE_SKELETON,        6,      26,     TYPE_ENEMY,             0,          Enemy_onInit,           TeleportingEnemy_onFrame,   TeleportingEnemy_onHit  );
    initTypeEx  ( TYPE_ICESHOT,         52,     0,      TYPE_ENEMY,             0,          Shot_onInit,            Shot_onFrame,               Shot_onHit              );
    initTypeEx  ( TYPE_FIRESHOT,        60,     26,     TYPE_ENEMY,             0,          Shot_onInit,            Shot_onFrame,               Shot_onHit              );
    initTypeEx  ( TYPE_DROP,            37,     43,     TYPE_DROP,              0,          Drop_onInit,            Drop_onFrame,               Drop_onHit              );
    initTypeEx  ( TYPE_PLATFORM,        4,      6,      TYPE_PLATFORM,          0,          Platform_onInit,        Platform_onFrame,           Platform_onHit          );
    initTypeEx  ( TYPE_SPRING,          65,     26,     TYPE_SPRING,            0,          Spring_onInit,          Spring_onFrame,             Spring_onHit            );
    initTypeEx  ( TYPE_FAN,             64,     26,     TYPE_FAN,               0,          Fan_onInit,             Fan_onFrame,                Object_onHit            );
    initTypeEx  ( TYPE_ARROW_LEFT,      32,     3,      TYPE_WALL,          SOLID_LEFT,     Object_onInit,          Object_onFrame,             Object_onHit            );
    initTypeEx  ( TYPE_ARROW_RIGHT,     31,     3,      TYPE_WALL,          SOLID_RIGHT,    Object_onInit,          Object_onFrame,             Object_onHit            );
    initTypeEx  ( TYPE_KEY,             45,     26,     TYPE_KEY,               0,          Object_onInit,          Item_onFrame,               Item_onHit              );
    initTypeEx  ( TYPE_COIN,            63,     26,     TYPE_COIN,              0,          Object_onInit,          Item_onFrame,               Item_onHit              );
    initTypeEx  ( TYPE_GEM,             50,     32,     TYPE_COIN,              0,          Object_onInit,          Item_onFrame,               Item_onHit              );
    initTypeEx  ( TYPE_APPLE,           15,     26,     TYPE_ITEM,              0,          Object_onInit,          Item_onFrame,               Item_onHit              );
    initTypeEx  ( TYPE_PEAR,            15,     27,     TYPE_ITEM,              0,          Object_onInit,          Item_onFrame,               Item_onHit              );
    initTypeEx  ( TYPE_STATUARY,        52,     27,     TYPE_STATUARY,          0,          Object_onInit,          Item_onFrame,               Item_onHit              );
    initTypeEx  ( TYPE_LADDER_PART,     62,     29,     TYPE_ITEM,              0,          Object_onInit,          Item_onFrame,               Item_onHit              );
    initTypeEx  ( TYPE_PICK,            62,     30,     TYPE_ITEM,              0,          Object_onInit,          Item_onFrame,               Item_onHit              );
    initTypeEx  ( TYPE_HEART,           62,     31,     TYPE_HEART,             0,          Object_onInit,          Item_onFrame,               Item_onHit              );
    initType    ( TYPE_ACTION,          0,      10,     TYPE_ITEM,              0                                                                                       );

    objectTypes[TYPE_PLAYER].width = PLAYER_WIDTH;
    objectTypes[TYPE_PLAYER].height = PLAYER_HEIGHT;
    objectTypes[TYPE_GHOST].speed = 1;
    objectTypes[TYPE_SCORPION].speed = 1;
    objectTypes[TYPE_SCORPION].width = 9;
    objectTypes[TYPE_SCORPION].height = 11;
    objectTypes[TYPE_SPIDER].speed = 1;
    objectTypes[TYPE_SPIDER].width = 11;
    objectTypes[TYPE_SPIDER].height = 10;
    objectTypes[TYPE_RAT].speed = 1;
    objectTypes[TYPE_RAT].width = 12;
    objectTypes[TYPE_RAT].height = 11;
    objectTypes[TYPE_BLOB].speed = 1;
    objectTypes[TYPE_BLOB].width = 11;
    objectTypes[TYPE_BLOB].height = 10;
    objectTypes[TYPE_BAT].speed = 2;
    objectTypes[TYPE_BAT].height = 10;
    objectTypes[TYPE_FIREBALL].speed = 2;
    objectTypes[TYPE_FIREBALL].width = 14;
    objectTypes[TYPE_FIREBALL].height = 12;
    objectTypes[TYPE_SKELETON].speed = 1;
    objectTypes[TYPE_ICESHOT].speed = 7;
    objectTypes[TYPE_ICESHOT].height = 7;
    objectTypes[TYPE_FIRESHOT].speed = 5;
    objectTypes[TYPE_FIRESHOT].width = 4;
    objectTypes[TYPE_FIRESHOT].height = 4;
    objectTypes[TYPE_DROP].width = 4;
    objectTypes[TYPE_DROP].height = 4;
    objectTypes[TYPE_HEART].width = 8;
    objectTypes[TYPE_HEART].height = 8;
    objectTypes[TYPE_PLATFORM].width = SPRITE_SIZE;
    objectTypes[TYPE_PLATFORM].height = SPRITE_SIZE;
    objectTypes[TYPE_PLATFORM].speed = 2;
    objectTypes[TYPE_PLATFORM].sprite.h = 8;
    objectTypes[TYPE_WALL_STAIR].sprite.h = 8;
    objectTypes[TYPE_GROUND_STAIR].sprite.h = 8;

    // Calculate real sizes (multiply on SIZE_FACTOR)
    for (ObjectTypeId t = TYPE_NONE; t < TYPE_COUNT; ++ t) {
        // Skip the player because it already has correct size
        if (t == TYPE_PLAYER) continue;
        ObjectType* type = &objectTypes[t];
        type->width *= SIZE_FACTOR;
        type->height *= SIZE_FACTOR;
    }
}
