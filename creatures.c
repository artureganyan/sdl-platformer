/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#include "creatures.h"
#include "render.h"
#include "helpers.h"
#include "game.h"


// Returns 0 if obj can not move by dx or dy
int move( Object* obj, int dx, int dy, int checkFloor )
{
    int r, c, cell[4], body[4];
    int canMove = 1;

    obj->x += dx;
    obj->y += dy;
    getObjectPos(obj, &r, &c, cell, body);

    if (isSolid(r, c)) {
        obj->x -= dx;
        obj->y -= dy;
        return 0;
    }
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
    } else if (dy < 0 && body[2] < cell[2]) {
        if (isSolid(r - 1, c) || body[2] < 0) {
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
            r = (source->y + CELL_HALF) / CELL_SIZE;
            for (x1 = x1 + CELL_HALF; x1 < x2; x1 += CELL_SIZE) {
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
    e->attack = 500 + rand() % 1000;
}

void onFrame_Enemy( Object* e )
{
    if (e->attack -- > 0) {
        if (!move(e, e->vx, e->vy, 1)) {
            e->vx = -e->vx;
            e->anim.direction = -e->anim.direction;
        }
        setAnimation(e, 1, 2, 24);
    } else {
        if (e->attack -- <= -100) {
            e->attack = 500 + rand() % 1000;
            if (rand() % 2) {
                e->vx = -e->vx;
                e->anim.direction = -e->anim.direction;
            }
        }
        setAnimation(e, 2, 2, 24);
    }
}

void onHit_Enemy( Object* e, Object* player )
{
    // Kill the enemy if player jumps on it
    /*
    if (player->y + PLAYER_HEIGHT <= (e->y + (CELL_SIZE - e->type->height) / 2)) {
        player->vy = -5;
        e->removed = 1;
    }
    */
    setAnimation(e, 4, 4, 0);
    killPlayer();
}


void onInit_EnemyShooter( Object* e )
{
    onInit_Enemy(e);
    e->attack = 0;
}

void onFrame_EnemyShooter( Object* e )
{
    if (!e->attack && isVisible(e, (Object*)&player)) {
        Object* shot = createObject(level, TYPE_ICESHOT, 0, 0);
        shot->x = e->anim.direction > 0 ? e->x + CELL_SIZE : e->x;
        shot->y = e->y + 2;
        shot->vx *= e->anim.direction;
        shot->anim.direction = e->anim.direction;
        e->attack = 48;
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


void onInit_Shot( Object* e )
{
    e->vx = e->type->speed;
    setAnimation(e, 1, 2, 5);
}

void onFrame_Shot( Object* e )
{
    if (!move(e, e->vx, e->vy, 0)) {
        setAnimation(e, 3, 3, 5);
        if (++ e->attack >= 8) {
            e->removed = 1;
        }
    }
}

void onHit_Shot( Object* e, Object* player )
{
    setAnimation(e, 3, 3, 5);
    killPlayer();
}


void onInit_Bat( Object* e )
{
    onInit_Enemy(e);
    setAnimation(e, 0, 1, 12);
    e->attack = 0;
    e->vy = 1;  // vy must be > 0, so the bat firstly go down,
                // and then return to the previous height
}

void onFrame_Bat( Object* e )
{
    int vy = e->attack % 2 ? e->vy : 0;
    if (!move(e, e->vx, vy, 0)) {
        e->vx = -e->vx;
        e->anim.direction = -e->anim.direction;
    }
    if (++ e->attack > CELL_SIZE) {
        e->attack = 0;
        e->vy = -e->vy;
    }
}

void onHit_Bat( Object* e, Object* player )
{
    killPlayer();
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


void onInit_Fireball( Object* e )
{
    onInit_EnemyShooter(e);
}

void onFrame_Fireball( Object* e )
{
    if (!e->attack && isVisible(e, (Object*)&player)) {
        Object* shot = createObject(level, TYPE_FIRESHOT, 0, 0);
        shot->x = e->x + e->anim.direction * 20;
        shot->y = e->y + 2;
        shot->vx *= e->anim.direction;
        shot->anim.direction = e->anim.direction;
        e->attack = 48;
    }
    if (!move(e, e->vx, e->vy, 0)) {
        e->vx = -e->vx;
        e->anim.direction = -e->anim.direction;
    }
    if (e->attack > 0) {
        e->attack -= 1;
    }
    if (e->attack > 24) {
        setAnimation(e, 4, 4, 24);
    } else {
        setAnimation(e, 0, 1, 24);
    }
}


void onInit_Drop( Object* e )
{
    e->y = (e->y / CELL_SIZE) * CELL_SIZE - (CELL_SIZE - e->type->height) / 2 - 1;
    e->attack = 0;  // >= 0  - source of drops
                    // == -1 - the drop in fall
                    // <  -1 - the drop on floor
}

void onFrame_Drop( Object* e )
{
    if (e->attack == -1) {
        if (e->vy < 5) {
            e->vy += 1;
        }
        if (!move(e, 0, e->vy, 0)) {
            e->y += e->vy;
            e->attack = -2;
        }
    } else if (e->attack < -1) {
        if (e->attack -- == -50) {
            e->removed = 1;
        }
    } else if (e->attack -- == 0) {
        Object* drop = createObject(level, TYPE_DROP, 0, 0);
        drop->x = e->x;
        drop->y = e->y;
        drop->attack = -1;
        e->attack = 100 + rand() % 500;
    }
}

void onHit_Drop( Object* e, Object* player )
{
    killPlayer();
}


void onFrame_Spider( Object* e )
{
    onFrame_Enemy(e);
    if (rand() % 100 == 99) {
        e->vx = (abs(e->vx) == 2 ? 1 : 2) * e->anim.direction;
    }
}


void onFrame_Teleporting( Object* e )
{
    if (e->attack > 0) {
        if (!move(e, e->vx, e->vy, 1)) {
            e->vx = -e->vx;
            e->anim.direction = -e->anim.direction;
        }
        setAnimation(e, 1, 2, 24);
    } else {
        if (e->attack == -100) {
            int r, c, prevr = (e->y + CELL_HALF) / CELL_SIZE;
            int count = CELL_COUNT;
            while (count --) {
                r = rand() % (ROW_COUNT - 1);
                c = rand() % COLUMN_COUNT;
                if (isSolid(r + 1, c) && !isSolid(r, c) && r != prevr) {
                    e->y = CELL_SIZE * r;
                    e->x = CELL_SIZE * c;
                }
            }
        } else if (e->attack <= -200) {
            e->attack = 100 + rand() % 100;
        }
        setAnimation(e, 2, 2, 24);
    }
    e->attack -= 1;
}

void onHit_Teleporting( Object* e, Object* player )
{
    if (e->attack >= -100) {
        onHit_Enemy(e, player);
    }
}


void onFrame_Mimicry( Object* e )
{
    if (e->attack > -100) {
        if (!move(e, e->vx, e->vy, 1)) {
            e->vx = -e->vx;
            e->anim.direction = -e->anim.direction;
        }
        setAnimation(e, 1, 2, 24);

        if (e->attack < 0 && rand() % 100 > 70) {
            const int types[] = {TYPE_COIN, TYPE_SPIKE_BOTTOM, TYPE_MUSHROOM1, TYPE_MUSHROOM2, TYPE_MUSHROOM3,
                                 TYPE_PILLAR_BOTTOM, TYPE_ROCK, TYPE_KEY, TYPE_GROUND_TOP};
            int r, c;
            getObjectCell(e, &r, &c);
            e->removed = 2;
            e->vy = level->map[r][c]->typeId;
            level->map[r][c] = &objectTypes[types[rand() % 9]];
            e->attack = -100;
        }

    } else if (e->attack <= -200) {
        int r, c;
        getObjectCell(e, &r, &c);
        level->map[r][c] = &objectTypes[e->vy];
        e->removed = 0;
        e->attack = 100 + rand() % 100;
    }
    e->attack -= 1;
}

void onHit_Mimicry( Object* e, Object* player )
{
    int r, c;
    getObjectCell(e, &r, &c);
    level->map[r][c] = &objectTypes[e->vy];
    e->removed = 0;
    onHit_Enemy(e, player);
}

