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

void cleanArray( ObjectArray* objects ) {
    int i, r;
    for (i = 0, r = 0; i < objects->count; ++ i) {
        Object* obj = objects->array[i];
        if (obj->removed) {
            free(obj);
            r += 1;
        } else if (r) {
            objects->array[i - r] = obj;
        }
    }
    objects->count -= r;
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
    obj->attack = 0;
    obj->anim.direction = 1;
    setAnimation(obj, 0, 0, 0);

    appendArray(&level->objects, obj);
    obj->type->onInit(obj);
    return obj;
}


void initTypes()
{
    #define INIT(typeId_, spriteX, spriteY, generalTypeId_, solid_) \
    { \
        ObjectType* type = &objectTypes[typeId_]; \
        type->sprite.x = (spriteX); \
        type->sprite.y = (spriteY); \
        type->sprite.w = SPRITE_SIZE; \
        type->sprite.h = SPRITE_SIZE; \
        type->typeId = typeId_; \
        type->generalTypeId = generalTypeId_; \
        type->solid = solid_; \
        type->onInit = onInit_Object; \
        type->onFrame = onFrame_Object; \
        type->onHit = onHit_Object; \
    }

    #define INIT_EX(typeId, spriteX, spriteY, generalTypeId, solid, onInit_, onFrame_, onHit_) \
    INIT(typeId, spriteX, spriteY, generalTypeId, solid); \
    { \
        ObjectType* type = &objectTypes[typeId]; \
        type->onInit = onInit_; \
        type->onFrame = onFrame_; \
        type->onHit = onHit_; \
    }

    //          type id         sprite x, y   general type id     solid     onInit        onFrame          onHit
    INIT    ( TYPE_NONE,        136,    0,      TYPE_NONE,          0                                               );
    INIT    ( TYPE_PLAYER,      416,    16,     TYPE_PLAYER,        0                                               );
    INIT    ( TYPE_WALL,        96,     80,     TYPE_WALL,          1                                               );
    INIT    ( TYPE_WALL_TOP,    96,     64,     TYPE_WALL,          1                                               );
    INIT    ( TYPE_PILLAR,      32,     432,    TYPE_BACKGROUND,    0                                               );
    INIT    ( TYPE_PILLAR_TOP,  32,     416,    TYPE_BACKGROUND,    0                                               );
    INIT    ( TYPE_PILLAR_BOTTOM, 32,   448,    TYPE_BACKGROUND,    0                                               );
    INIT    ( TYPE_DOOR,        0,      160,    TYPE_DOOR,          1                                               );
    INIT    ( TYPE_LADDER,      32,     192,    TYPE_LADDER,        0                                               );
    INIT_EX ( TYPE_GHOST,       416,    112,    TYPE_ENEMY,         0,  onInit_EnemyShooter, onFrame_EnemyShooter,  onHit_Object    );
    INIT_EX ( TYPE_SCORPION,    416,    160,    TYPE_ENEMY,         0,  onInit_Enemy,   onFrame_Enemy,  onHit_Enemy );
    INIT_EX ( TYPE_BAT,         416,    128,    TYPE_ENEMY,         0,  onInit_Bat,     onFrame_Bat,    onHit_Bat   );
    INIT_EX ( TYPE_FIREBALL,    416,    210,    TYPE_ENEMY,         0,  onInit_EnemyShooter,    onFrame_Fireball,   onHit_Bat   );
    INIT_EX ( TYPE_KEY,         416,    719,    TYPE_KEY,           0,  onInit_Object,  onFrame_Object, onHit_Item  );
    INIT_EX ( TYPE_COIN,        432,    751,    TYPE_COIN,          0,  onInit_Object,  onFrame_Object, onHit_Item  );
    INIT_EX ( TYPE_FIRESHOT,    512,    592,    TYPE_ENEMY,         0,  onInit_Shot, onFrame_Shot, onHit_Shot   );
    INIT_EX ( TYPE_ICESHOT,     0,      832,    TYPE_ENEMY,         0,  onInit_Shot, onFrame_Shot, onHit_Shot   );
    INIT_EX ( TYPE_DROP,        0,      848,    TYPE_DROP,          0,  onInit_Drop,    onFrame_Drop,   onHit_Drop  );

    objectTypes[TYPE_GHOST].speed = 1;
    objectTypes[TYPE_SCORPION].speed = 1;
    objectTypes[TYPE_BAT].speed = 2;
    objectTypes[TYPE_FIREBALL].speed = 2;
    objectTypes[TYPE_ICESHOT].speed = 5;
    objectTypes[TYPE_FIRESHOT].speed = 5;

    #undef INIT
    #undef INIT_EX
}


