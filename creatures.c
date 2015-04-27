/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#include "creatures.h"
#include "render.h"
#include "helpers.h"
#include "game.h"


// Returns 0 if obj can not move by dx and/or dy
int move( Object* obj, int dx, int dy, int checkFloor )
{
    int r, c, cell[4], body[4];
    int canMove = 1;

    obj->x += dx;
    obj->y += dy;
    getObjectPos(obj, &r, &c, cell, body);

    if (dx > 0 && body[1] > cell[1]) {
        if (isSolid(r, c + 1) || body[1] > LEVEL_WIDTH || (checkFloor && !isSolidOrLadder(r + 1, c + 1))) {
            obj->x -= dx;
            canMove = 0;
        }
    } else if (dx < 0 && body[0] < cell[0]) {
        if (isSolid(r, c - 1) || body[0] < 0 || (checkFloor && !isSolidOrLadder(r + 1, c - 1))) {
            obj->x -= dx;
            canMove = 0;
        }
    }
    if (dy > 0 && body[3] > cell[3]) {
        if (isSolid(r + 1, c) || body[3] > LEVEL_HEIGHT) {
            obj->y -= dy;
            canMove = 0;
        }
    } else if (dy < 0 && body[2] > cell[2]) {
        if (isSolid(r - 1, c) || body[0] < 0) {
            obj->y -= dy;
            canMove = 0;
        }
    }
    return canMove;
}

int isVisible( Object* source, Object* target )
{
    int x1 = 0, x2 = 0, r, c, visible = 0;
    if (target->y + CELL_SIZE > source->y + CELL_HALF &&
        target->y < source->y + CELL_HALF) {
        if (target->x < source->x && source->anim.direction < 0) {
            x1 = target->x;
            x2 = source->x;
        } else if (target->x > source->x && source->anim.direction > 0) {
            x1 = source->x;
            x2 = target->x;
        }
        if (x1 != x2) {
            visible = 1;
            r = source->y / CELL_SIZE;
            for (; x1 < x2; x1 += CELL_SIZE) {
                c = x1 / CELL_SIZE;
                if (isSolid(r, c)) {
                    visible = 0;
                    break;
                }
            }
        }
    }
    return visible;
}


void onInit_Object( Object* o ) {}
void onFrame_Object( Object* o ) {}
void onHit_Object( Object* o, Object* t ) {}


void onInit_Enemy( Object* e )
{
    int dir = rand() % 2 ? 1 : -1;
    e->vx = e->type->speed * dir;
    e->anim.direction = dir;
}

void onFrame_Enemy( Object* e )
{
    if (!move(e, e->vx, e->vy, 1)) {
        e->vx = -e->vx;
        e->anim.direction = -e->anim.direction;
    }
    setAnimation(e, 1, 2, 24);
}

void onHit_Enemy( Object* e, Object* player )
{
    setAnimation(e, 4, 4, 0);
    setAnimation((Object*)player, 5, 5, 0);
    gameOver = 1;
}


void onFrame_EnemyShooter( Object* e )
{
    Object* fireball;
    if (!e->attack && isVisible(e, (Object*)&player)) {
        e->attack = 48;
        fireball = createObject(level, TYPE_FIREBALL, 0, 0);
        fireball->x = e->x + e->anim.direction * 20;
        fireball->y = e->y + 2;
        fireball->vx *= e->anim.direction;
        fireball->anim.direction = e->anim.direction;
    }
    if (e->attack) {
        if (e->attack >= 12) {
            setAnimation(e, 4, 4, 24);
        } else {
            setAnimation(e, 1, 1, 24);
        }
        e->attack -= 1;
    } else {
        if (!move(e, e->vx, e->vy, 1)) {
            e->vx = -e->vx;
            e->anim.direction = -e->anim.direction;
        }
        setAnimation(e, 1, 2, 24);
    }
}


void onInit_Fireball( Object* e )
{
    e->vx = e->type->speed;
    setAnimation(e, 1, 2, 5);
}

void onFrame_Fireball( Object* e )
{
    if (!move(e, e->vx, e->vy, 0)) {
        setAnimation(e, 3, 3, 5);
        if (++ e->attack >= 8) {
            e->removed = 1;
        }
    }
}

void onHit_Fireball( Object* e, Object* player )
{
    e->removed = 1;
    setAnimation(player, 5, 5, 0);
    gameOver = 1;
}


void onInit_Bat( Object* e )
{
    onInit_Enemy(e);
    setAnimation(e, 0, 1, 12);
}

void onFrame_Bat( Object* e )
{
    if (!move(e, e->vx, e->vy, 0)) {
        e->vx = -e->vx;
        e->anim.direction = -e->anim.direction;
    }
}

void onHit_Bat( Object* e, Object* player )
{
    setAnimation((Object*)player, 5, 5, 0);
    gameOver = 1;
}


void onHit_Item( Object* item, Object* target )
{
    ObjectTypeId generalTypeId = item->type->generalTypeId;
    item->removed = 1;

    if (generalTypeId == TYPE_COIN) {
        player.coins += 1;
        if (player.coins == level->coins) {
            // TODO Level complete
        }
    } else if (generalTypeId == TYPE_KEY) {
        player.keys += 1;
    }
}


void onInit_Drop( Object* e )
{
    e->y = (e->y / CELL_SIZE) * CELL_SIZE - 10;
    e->vx = e->y; // \todo Fix this later
}

void onFrame_Drop( Object* e )
{
    if (++ e->attack >= 100) {
        if (e->vy < 5) {
            e->vy += 1;
        }
        if (!move(e, 0, e->vy, 0)) {
            e->y = e->vx;
            e->attack = 0;
        }
    }
}

void onHit_Drop( Object* e, Object* player )
{
    setAnimation((Object*)player, 5, 5, 0);
    gameOver = 1;
}

