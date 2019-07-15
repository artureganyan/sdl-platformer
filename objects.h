/******************************************************************************
 * Copyright (c) Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#ifndef OBJECTS_H
#define OBJECTS_H

#include "types.h"

void Object_onInit( Object* object );
void Object_onFrame( Object* object );
void Object_onHit( Object* object );

void Enemy_onInit( Object* e );
void Enemy_onFrame( Object* e );
void Enemy_onHit( Object* e );

void ShooterEnemy_onFrame( Object* e );

void Shot_onInit( Object* e );
void Shot_onFrame( Object* e );
void Shot_onHit( Object* e );

void Bat_onInit( Object* e );
void Bat_onFrame( Object* e );
void Bat_onHit( Object* e );

void Item_onHit( Object* item );
void Item_onFrame( Object* item );

void Drop_onInit( Object* e );
void Drop_onFrame( Object* e );
void Drop_onHit( Object* e );

void Fireball_onInit( Object* e );
void Fireball_onFrame( Object* e );

void Spider_onFrame( Object* e );

void TeleportingEnemy_onFrame( Object* e );
void TeleportingEnemy_onHit( Object* e );

void Platform_onInit( Object* e );
void Platform_onFrame( Object* e );
void Platform_onHit( Object* e );

void Spring_onInit( Object* e );
void Spring_onFrame( Object* e );
void Spring_onHit( Object* e );

void Fan_onInit( Object* e );
void Fan_onFrame( Object* e );

void Cloud_onHit( Object* e );

void Torch_onInit( Object* e );
void Torch_onHit( Object* e );

void Water_onInit( Object* e );
void Water_onHit( Object* e );

#endif
