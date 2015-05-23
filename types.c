/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#include "types.h"
#include "render.h"
#include "creatures.h"

ObjectType objectTypes[TYPE_COUNT];
Player player;

const char* messages[MESSAGE_COUNT] = {
        "",
        "Game Over",
        "No items",
        "Can not use",
        "You lost life",
        "3",
        ""
    };


void initArray( ObjectArray* objects )
{
    objects->reserved = 16;
    objects->count = 0;
    objects->array = (Object**)malloc(sizeof(Object) * objects->reserved);
}

void appendArray( ObjectArray* objects, Object* obj )
{
    if (objects->count == objects->reserved) {
        objects->reserved *= 2;
        objects->array = (Object**)realloc(objects->array, sizeof(Object) * objects->reserved);
    }
    objects->array[objects->count ++] = obj;
}

void freeArray( ObjectArray* objects )
{
    free(objects->array);
    objects->array = NULL;
    objects->reserved = 0;
    objects->count = 0;
}

void cleanArray( ObjectArray* objects )
{
    int r = 0;
    for (int i = 0; i < objects->count; ++ i) {
        Object* obj = objects->array[i];
        if (obj->removed == 1) {
            free(obj);
            r += 1;
        } else if (obj->removed == 2) {
            r += 1;
        } else if (r) {
            objects->array[i - r] = obj;
        }
    }
    objects->count -= r;
}

void reorderDepth( ObjectArray* objects )
{
    for (int i = 0, r = 0; i < objects->count; ++ i) {
        Object* obj = objects->array[i];
        if (obj->type->typeId >= TYPE_BACKOBJECTS) {
            objects->array[i] = objects->array[r];
            objects->array[r] = obj;
            r += 1;
        }
    }
}


void createObjectInMap( Level* level, ObjectTypeId typeId, int r, int c )
{
    level->map[r][c] = &objectTypes[typeId];
}

Object* createObject( Level* level, ObjectTypeId typeId, int r, int c )
{
    Object* obj = (Object*)malloc(sizeof(Object));
    obj->type = &objectTypes[typeId];
    obj->x = CELL_SIZE * c;
    obj->y = CELL_SIZE * r;
    obj->vx = 0;
    obj->vy = 0;
    obj->removed = 0;
    obj->state = 0;
    obj->anim.direction = 1;
    setAnimation(obj, 0, 0, 0);

    appendArray(&level->objects, obj);
    obj->type->onInit(obj);
    return obj;
}


void initTypeEx( ObjectTypeId typeId, int spriteRow, int spriteColumn, ObjectTypeId generalTypeId, int solid,
                 OnInit onInit, OnFrame onFrame, OnHit onHit, const char* name )
{
    ObjectType* type = &objectTypes[typeId];
    type->sprite.y = (spriteRow) * SPRITE_SIZE;
    type->sprite.x = (spriteColumn) * SPRITE_SIZE;
    type->sprite.w = SPRITE_SIZE;
    type->sprite.h = SPRITE_SIZE;
    type->typeId = typeId;
    type->generalTypeId = generalTypeId;
    type->name = name;
    type->nameTexture = createText(type->name);
    type->solid = solid;
    type->speed = 0;
    type->width = CELL_SIZE;
    type->height = CELL_SIZE;
    type->onInit = onInit;
    type->onFrame = onFrame;
    type->onHit = onHit;
}

void initType( ObjectTypeId typeId, int spriteRow, int spriteColumn, ObjectTypeId generalTypeId, int solid )
{
    initTypeEx(typeId, spriteRow, spriteColumn, generalTypeId, solid,
            onInit_Object, onFrame_Object, onHit_Object, NULL);
}


void initTypes()
{
    //             type id             sprite r, c    general type id     solid     onInit                      onFrame           onHit            name
    initType    ( TYPE_NONE,            0,      10,     TYPE_NONE,          0                                                                               );
    initType    ( TYPE_PLAYER,          1,      26,     TYPE_PLAYER,        0                                                                               );
    initType    ( TYPE_WALL_TOP,        4,      6,      TYPE_WALL,          1                                                                               );
    initType    ( TYPE_WALL,            5,      6,      TYPE_WALL,          1                                                                               );
    initType    ( TYPE_WALL_FAKE,       5,      6,      TYPE_WALL_FAKE,     0                                                                               );
    initType    ( TYPE_GROUND_TOP,      6,      3,      TYPE_WALL,          1                                                                               );
    initType    ( TYPE_GROUND,          7,      3,      TYPE_WALL,          1                                                                               );
    initType    ( TYPE_GROUND_FAKE,     7,      3,      TYPE_GROUND_FAKE,   0                                                                               );
    initType    ( TYPE_WATER_TOP,       8,      0,      TYPE_WATER,         0                                                                               );
    initType    ( TYPE_WATER,           9,      0,      TYPE_WATER,         0                                                                               );
    initType    ( TYPE_GRASS,           40,     0,      TYPE_BACKGROUND,    0                                                                               );
    initType    ( TYPE_GRASS_BIG,       40,     0,      TYPE_BACKGROUND,    0                                                                               );
    initType    ( TYPE_ROCK,            50,     0,      TYPE_BACKGROUND,    1                                                                               );
    initType    ( TYPE_SPIKE_TOP,       48,     0,      TYPE_SPIKE,         0                                                                               );
    initType    ( TYPE_SPIKE_BOTTOM,    49,     0,      TYPE_SPIKE,         0                                                                               );
    initType    ( TYPE_TREE1,           41,     3,      TYPE_BACKGROUND,    0                                                                               );
    initType    ( TYPE_TREE2,           41,     4,      TYPE_BACKGROUND,    0                                                                               );
    initTypeEx  ( TYPE_CLOUD1,          51,     6,      TYPE_PLATFORM,      0,  onInit_Object,          onFrame_Object,         onHit_Cloud,        NULL    );
    initType    ( TYPE_CLOUD2,          51,     5,      TYPE_PLATFORM,      0                                                                               );
    initType    ( TYPE_MUSHROOM1,       47,     0,      TYPE_BACKGROUND,    0                                                                               );
    initType    ( TYPE_MUSHROOM2,       47,     1,      TYPE_BACKGROUND,    0                                                                               );
    initType    ( TYPE_MUSHROOM3,       47,     2,      TYPE_BACKGROUND,    0                                                                               );
    initType    ( TYPE_PILLAR_TOP,      26,     2,      TYPE_BACKGROUND,    0                                                                               );
    initType    ( TYPE_PILLAR,          27,     2,      TYPE_BACKGROUND,    0                                                                               );
    initType    ( TYPE_PILLAR_BOTTOM,   28,     2,      TYPE_BACKGROUND,    0                                                                               );
    initType    ( TYPE_DOOR,            10,     0,      TYPE_DOOR,          1                                                                               );
    initType    ( TYPE_LADDER,          12,     2,      TYPE_LADDER,        0                                                                               );
    initTypeEx  ( TYPE_GHOST,           7,      26,     TYPE_ENEMY,         0,  onInit_Enemy,           onFrame_EnemyShooter,   onHit_Object,       NULL    );
    initTypeEx  ( TYPE_SCORPION,        10,     26,     TYPE_ENEMY,         0,  onInit_Enemy,           onFrame_Enemy,          onHit_Enemy,        NULL    );
    initTypeEx  ( TYPE_SPIDER,          11,     26,     TYPE_ENEMY,         0,  onInit_Enemy,           onFrame_Spider,         onHit_Enemy,        NULL    );
    initTypeEx  ( TYPE_RAT,             9,      26,     TYPE_ENEMY,         0,  onInit_Enemy,           onFrame_Enemy,          onHit_Enemy,        NULL    );
    initTypeEx  ( TYPE_BAT,             8,      26,     TYPE_ENEMY,         0,  onInit_Bat,             onFrame_Bat,            onHit_Bat,          NULL    );
    initTypeEx  ( TYPE_BLOB,            12,     26,     TYPE_ENEMY,         0,  onInit_Enemy,           onFrame_Mimicry,        onHit_Mimicry,      NULL    );
    initTypeEx  ( TYPE_FIREBALL,        13,     26,     TYPE_ENEMY,         0,  onInit_Fireball,        onFrame_Fireball,       onHit_Bat,          NULL    );
    initTypeEx  ( TYPE_SKELETON,        6,      26,     TYPE_ENEMY,         0,  onInit_Enemy,           onFrame_Teleporting,    onHit_Teleporting,  NULL    );
    initTypeEx  ( TYPE_ICESHOT,         52,     0,      TYPE_ENEMY,         0,  onInit_Shot,            onFrame_Shot,           onHit_Shot,         NULL    );
    initTypeEx  ( TYPE_FIRESHOT,        54,     0,      TYPE_ENEMY,         0,  onInit_Shot,            onFrame_Shot,           onHit_Shot,         NULL    );
    initTypeEx  ( TYPE_DROP,            37,     43,     TYPE_DROP,          0,  onInit_Drop,            onFrame_Drop,           onHit_Drop,         NULL    );
    initTypeEx  ( TYPE_PLATFORM,        4,      6,      TYPE_PLATFORM,      0,  onInit_Platform,        onFrame_Platform,       onHit_Platform,     NULL    );
    initTypeEx  ( TYPE_KEY,             45,     26,     TYPE_KEY,           0,  onInit_Object,          onFrame_Object,         onHit_Item,         "Key"   );
    initTypeEx  ( TYPE_COIN,            47,     27,     TYPE_COIN,          0,  onInit_Object,          onFrame_Object,         onHit_Item,         "Coin"  );
    initTypeEx  ( TYPE_APPLE,           15,     26,     TYPE_APPLE,         0,  onInit_Object,          onFrame_Object,         onHit_Item,         "Apple" );
    initTypeEx  ( TYPE_PEAR,            15,     27,     TYPE_PEAR,          0,  onInit_Object,          onFrame_Object,         onHit_Item,         "Pear"  );
    initTypeEx  ( TYPE_LADDER_PART,     43,     32,     TYPE_LADDER_PART,   0,  onInit_Object,          onFrame_Object,         onHit_Item,         "Ladder");
    initTypeEx  ( TYPE_PICK,            43,     33,     TYPE_PICK,          0,  onInit_Object,          onFrame_Object,         onHit_Item,         "Pick"  );
    initType    ( TYPE_HEART,           57,     27,     TYPE_HEART,         0                                                                               );
    initType    ( TYPE_ACTION,          0,      10,     TYPE_ACTION,        0                                                                               );

    objectTypes[TYPE_PLAYER].width = PLAYER_WIDTH;
    objectTypes[TYPE_PLAYER].height = PLAYER_HEIGHT;
    objectTypes[TYPE_GHOST].speed = 1;
    objectTypes[TYPE_SCORPION].speed = 1;
    objectTypes[TYPE_SCORPION].width = 18;
    objectTypes[TYPE_SCORPION].height = 22;
    objectTypes[TYPE_SPIDER].speed = 1;
    objectTypes[TYPE_SPIDER].width = 22;
    objectTypes[TYPE_SPIDER].height = 20;
    objectTypes[TYPE_RAT].speed = 1;
    objectTypes[TYPE_RAT].width = 24;
    objectTypes[TYPE_RAT].height = 22;
    objectTypes[TYPE_BLOB].speed = 1;
    objectTypes[TYPE_BLOB].width = 22;
    objectTypes[TYPE_BLOB].height = 20;
    objectTypes[TYPE_BAT].speed = 2;
    objectTypes[TYPE_BAT].height = 18;
    objectTypes[TYPE_FIREBALL].speed = 2;
    objectTypes[TYPE_FIREBALL].width = 24;
    objectTypes[TYPE_FIREBALL].height = 22;
    objectTypes[TYPE_SKELETON].speed = 1;
    objectTypes[TYPE_ICESHOT].speed = 5;
    objectTypes[TYPE_ICESHOT].height = 14;
    objectTypes[TYPE_FIRESHOT].speed = 5;
    objectTypes[TYPE_FIRESHOT].width = 8;
    objectTypes[TYPE_FIRESHOT].height = 8;
    objectTypes[TYPE_DROP].width = 8;
    objectTypes[TYPE_DROP].height = 8;
    objectTypes[TYPE_PLATFORM].width = CELL_SIZE;
    objectTypes[TYPE_PLATFORM].height = CELL_SIZE;
    objectTypes[TYPE_PLATFORM].speed = 2;
}
