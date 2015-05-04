/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#ifndef CREATURES_H
#define CREATURES_H

#include "types.h"

void onInit_Object( Object* o );
void onFrame_Object( Object* o );
void onHit_Object( Object* o );

void onInit_Enemy( Object* e );
void onFrame_Enemy( Object* e );
void onHit_Enemy( Object* e );

void onInit_EnemyShooter( Object* e );
void onFrame_EnemyShooter( Object* e );

void onInit_Shot( Object* e );
void onFrame_Shot( Object* e );
void onHit_Shot( Object* e );

void onInit_Bat( Object* e );
void onFrame_Bat( Object* e );
void onHit_Bat( Object* e );

void onHit_Item( Object* item );

void onInit_Drop( Object* e );
void onFrame_Drop( Object* e );
void onHit_Drop( Object* e );

void onInit_Fireball( Object* e );
void onFrame_Fireball( Object* e );

void onFrame_Spider( Object* e );

void onFrame_Teleporting( Object* e );
void onHit_Teleporting( Object* e );

void onFrame_Mimicry( Object* e );
void onHit_Mimicry( Object* e );

void onInit_Platform( Object* e );
void onFrame_Platform( Object* e );
void onHit_Platform( Object* e );

void onHit_Cloud( Object* e );

#endif
