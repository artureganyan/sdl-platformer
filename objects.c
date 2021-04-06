/******************************************************************************
 * Copyright (c) Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#include "objects.h"
#include "render.h"
#include "helpers.h"
#include "levels.h"
#include "game.h"
#include "framecontrol.h"
#include <math.h>


// Helpers

typedef enum
{
    DIRECTION_NONE = 0,
    DIRECTION_LEFT = 1,
    DIRECTION_RIGHT = 2,
    DIRECTION_UP = 4,
    DIRECTION_DOWN = 8,
    DIRECTION_X = DIRECTION_LEFT | DIRECTION_RIGHT,
    DIRECTION_Y = DIRECTION_UP | DIRECTION_DOWN,
    DIRECTION_XY = DIRECTION_X | DIRECTION_Y
} Direction;

typedef enum
{
    HITTEST_NONE = 0,
    HITTEST_WALLS = 1,
    HITTEST_FLOOR = 2,
    HITTEST_LEVEL = 4,
    HITTEST_ALL = HITTEST_WALLS | HITTEST_FLOOR | HITTEST_LEVEL
} HitTest;

// Moves the object and checks the walls, floor and level borders according
// to hitTest flags. Returns 0 on success, otherwise returns the directions
// which the object could not fully move to.
static int move( Object* object, int hitTest )
{
    const double dt = getElapsedFrameTime() / 1000.0;
    const double dx = limitAbs(object->vx, MAX_SPEED) * dt;
    const double dy = limitAbs(object->vy, MAX_SPEED) * dt;

    const int check_walls = hitTest & HITTEST_WALLS;
    const int check_floor = hitTest & HITTEST_FLOOR;
    const int check_level = hitTest & HITTEST_LEVEL;

    const SDL_Rect bodyRect = object->type->body;

    int result = 0;
    int r, c; Borders cell, body;
    getObjectPos(object, &r, &c, &cell, &body);

    object->x += dx;
    object->y += dy;
    getObjectBody(object, &body);

    if (dx > 0 && body.right > cell.right) {
        if ((check_walls && isSolid(r, c + 1, SOLID_LEFT)) ||
            (check_level && body.right > LEVEL_WIDTH) || 
            (check_floor && !isSolid(r + 1, c + 1, SOLID_TOP) && !isLadder(r + 1, c + 1))) {
            object->x = cell.right - (bodyRect.x + bodyRect.w);
            result |= DIRECTION_X;
        }
    } else if (dx < 0 && body.left < cell.left) {
        if ((check_walls && isSolid(r, c - 1, SOLID_RIGHT)) ||
            (check_level && body.left < 0) ||
            (check_floor && !isSolid(r + 1, c - 1, SOLID_TOP) && !isLadder(r + 1, c - 1))) {
            object->x = cell.left - bodyRect.x;
            result |= DIRECTION_X;
        }
    }
    if (dy > 0 && body.bottom > cell.bottom) {
        if ((check_walls && isSolid(r + 1, c, SOLID_TOP)) ||
            (check_level && body.bottom > LEVEL_HEIGHT)) {
            object->y = cell.bottom - (bodyRect.y + bodyRect.h);
            result |= DIRECTION_Y;
        }
    } else if (dy < 0 && body.top < cell.top) {
        if ((check_walls && isSolid(r - 1, c, SOLID_BOTTOM)) ||
            (check_level && body.top < 0)) {
            object->y = cell.top - bodyRect.y;
            result |= DIRECTION_Y;
        }
    }

    return result;
}

static void setSpeed( Object* object, double vx, double vy )
{
    object->vx = vx;
    object->vy = vy;
    object->anim.flip = vx < 0 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
}

// Returns animation speed (frames per second) for the movement speed (pixels per second)
static inline int speedToFps( double speed )
{
    return ceil(fabs(speed / 12.0));
}

// Returns 1 if the source sees the target
static int isVisible( Object* source, Object* target )
{
    if (target->y + CELL_SIZE > source->y + CELL_HALF &&
        target->y < source->y + CELL_HALF) {
        int x1, x2;
        if (target->x < source->x && (source->anim.flip & SDL_FLIP_HORIZONTAL)) {
            x1 = target->x;
            x2 = source->x;
        } else if (target->x > source->x && !(source->anim.flip & SDL_FLIP_HORIZONTAL)) {
            x1 = source->x;
            x2 = target->x;
        } else {
            return 0;
        }
        const int r = (source->y + CELL_HALF) / CELL_SIZE;
        for (x1 = x1 + CELL_HALF; x1 < x2; x1 += CELL_SIZE) {
            const int c = x1 / CELL_SIZE;
            if (isSolid(r, c, SOLID_LEFT | SOLID_RIGHT)) {
                return 0;
            }
        }
        return 1;
    }
    return 0;
}


// Logic

/*
 * Any object has the integer field "state". It indicates the current object's
 * state and is usually increased on every frame. The states are going in
 * ascending order as follows:
 *
 * 0            STATE_1         STATE_2         STATE_N
 * |---------------|-----x---------|----- ... -----|
 *                       |
 *              object->state
 *
 * Values within [0; STATE_1] belongs to STATE_1, within [STATE_1 + 1; STATE2] -
 * to STATE_2, and so on. The state's range size determines how long the object
 * will be in that state.
 *
 * Note: The minimum state value is usually 0, but it can be negative as well.
 */


void Object_onInit( Object* object ) {}
void Object_onFrame( Object* object ) {}
void Object_onHit( Object* object ) {}


static const int ENEMY_MOVING = 10000;
static const int ENEMY_WAITING = 12000;

void MovingEnemy_onInit( Object* e )
{
    const int dir = rand() % 2 ? 1 : -1;
    setSpeed(e, e->type->speed * dir, 0);
    e->state = -rand() % ENEMY_MOVING;
}

void MovingEnemy_onFrame( Object* e )
{
    if (e->state <= ENEMY_MOVING) {
        if (move(e, HITTEST_ALL)) {
            setSpeed(e, -e->vx, e->vy);
        }
        setAnimation(e, 1, 2, speedToFps(e->vx));

    } else if (e->state <= ENEMY_WAITING) {
        setAnimation(e, 2, 2, 0);

    } else {
        e->state = ENEMY_MOVING - rand() % (ENEMY_MOVING * 2);
        if (rand() % 2) {
            setSpeed(e, -e->vx, e->vy);
        }
    }

    e->state += getElapsedFrameTime();
}

void MovingEnemy_onHit( Object* e )
{
    if (player.inAir && player.y < e->y) {
        player.vy *= -2;
        return;
    }
    if ((e->vx < 0 && player.x > e->x) || (e->vx > 0 && player.x < e->x)) {
        setSpeed(e, -e->vx, e->vy);
    }
    e->state = ENEMY_MOVING + 1;
    setAnimation(e, 4, 4, 0);
    killPlayer();
}


static const int SHOOTINGENEMY_MOVING = 0;
static const int SHOOTINGENEMY_ATTACK1 = 750;
static const int SHOOTINGENEMY_ATTACK2 = 1000;

void ShootingEnemy_onFrame( Object* e )
{
    if (e->state <= SHOOTINGENEMY_MOVING) {
        if (isVisible(e, (Object*)&player)) {
            Object* shot = createObject(level, TYPE_ICESHOT, 0, 0);
            shot->x = e->anim.flip & SDL_FLIP_HORIZONTAL ? e->x - shot->type->sprite.w : e->x + e->type->sprite.w;
            shot->y = e->y;
            setSpeed(shot, shot->vx * (e->vx > 0 ? 1 : -1), shot->vy);
            e->state = SHOOTINGENEMY_MOVING + 1;
        } else if (move(e, HITTEST_ALL)) {
            setSpeed(e, -e->vx, e->vy);
        }
        setAnimation(e, 1, 2, speedToFps(e->vx));

    } else if (e->state <= SHOOTINGENEMY_ATTACK1) {
        setAnimation(e, 4, 4, 2);

    } else if (e->state <= SHOOTINGENEMY_ATTACK2) {
        setAnimation(e, 1, 1, 2);

    } else {
        e->state = SHOOTINGENEMY_MOVING;
    }

    if (e->state > SHOOTINGENEMY_MOVING)
        e->state += getElapsedFrameTime();
}


static const int SHOT_MOVING = 0;
static const int SHOT_HIT = 170;

void Shot_onInit( Object* e )
{
    setSpeed(e, e->type->speed, 0);
    setAnimation(e, 1, 2, 9);
}

void Shot_onFrame( Object* e )
{
    if (e->state <= SHOT_MOVING) {
        if (move(e, HITTEST_WALLS | HITTEST_LEVEL)) {
            setAnimation(e, 3, 3, 0);
            e->state = SHOT_MOVING + 1;
        }

    } else if (e->state <= SHOT_HIT) {
        e->state += getElapsedFrameTime();

    } else {
        e->removed = 1;
    }
}

void Shot_onHit( Object* e )
{
    setAnimation(e, 3, 3, 0);
    e->state = SHOT_MOVING + 1;
    killPlayer();
}


static const int BAT_FLY_HEIGHT = CELL_SIZE * 1.25;

void Bat_onInit( Object* e )
{
    MovingEnemy_onInit(e);
    setAnimation(e, 0, 1, speedToFps(e->vx));
    setSpeed(e, e->vx, e->type->speed / 2.0);
    e->state = 0;
}

void Bat_onFrame( Object* e )
{
    int r, c; Borders cell, body;
    if (e->state == 0) {
        getObjectPos(e, &r, &c, &cell, &body);
        e->state = cell.top + BAT_FLY_HEIGHT;
    }

    const int m = move(e, HITTEST_WALLS | HITTEST_LEVEL);
    if (m & DIRECTION_X) {
        setSpeed(e, -e->vx, e->vy);
    }
    if (m & DIRECTION_Y) {
        setSpeed(e, e->vx, -e->vy);
    } else {
        getObjectPos(e, &r, &c, &cell, &body);
        if (body.bottom >= e->state || body.top <= e->state - BAT_FLY_HEIGHT) {
            setSpeed(e, e->vx, -e->vy);
        }
    }
}

void Bat_onHit( Object* e )
{
    killPlayer();
}


static const int ITEM_IDLE = 0;
static const int ITEM_TAKEN = 1;
static const double ITEM_FADE_SPEED = 0.25; // Seconds

void Item_onHit( Object* item )
{
    if (item->state <= ITEM_IDLE) {
        ObjectTypeId generalTypeId = item->type->generalTypeId;

        if (generalTypeId == TYPE_COIN) {
            player.coins += 1;
        } else if (generalTypeId == TYPE_KEY) {
            player.keys += 1;
        } else if (generalTypeId == TYPE_HEART) {
            player.lives += 1;
        } else if (generalTypeId == TYPE_STATUARY) {
            completeLevel();
        } else {
            // Add the item to player.items, for example
        }

        item->state = ITEM_IDLE + 1;
        setSpeed(item, item->vx, -7 * 24);
        setAnimation(item, 0, 0, 0);
    }
}

void Item_onFrame( Object* item )
{
    if (item->state <= ITEM_IDLE) {
        // Nothing

    } else if (item->state <= ITEM_TAKEN) {
        const double dt = getElapsedFrameTime() / 1000.0;
        item->anim.alpha -= (255 / ITEM_FADE_SPEED) * dt;
        if (item->anim.alpha < 0) {
            item->anim.alpha = 0;
            item->state = ITEM_TAKEN + 1;
        }
        setSpeed(item, item->vx, item->vy - item->vy * dt / ITEM_FADE_SPEED);
        move(item, HITTEST_NONE);

    } else {
        item->removed = 1;
    }
}


static const int FIREBALL_MOVING = 0;
static const int FIREBALL_ATTACK1 = 500;
static const int FIREBALL_ATTACK2 = 1000;

void Fireball_onInit( Object* e )
{
    MovingEnemy_onInit(e);
    setSpeed(e, e->vx, e->vx);
}

void Fireball_onFrame( Object* e )
{
    if (e->state <= FIREBALL_MOVING) {
        if (isVisible(e, (Object*)&player)) {
            Object* shot = createObject(level, TYPE_FIRESHOT, 0, 0);
            shot->x = e->anim.flip & SDL_FLIP_HORIZONTAL ? e->x - shot->type->sprite.w : e->x + e->type->sprite.w;
            shot->y = e->y + 2;
            setSpeed(shot, shot->vx * (e->vx > 0 ? 1 : -1), shot->vy);
            e->state = FIREBALL_MOVING + 1;
        }
        setAnimation(e, 0, 1, 2);

    } else if (e->state <= FIREBALL_ATTACK1) {
        setAnimation(e, 4, 4, 0);

    } else if (e->state <= FIREBALL_ATTACK2) {
        setAnimation(e, 0, 1, 2);

    } else {
        e->state = FIREBALL_MOVING;
    }

    const int m = move(e, HITTEST_WALLS | HITTEST_LEVEL);
    if (m) {
        setSpeed(e, m & DIRECTION_X ? -e->vx : e->vx, m & DIRECTION_Y ? -e->vy : e->vy);
    }

    const int dt = getElapsedFrameTime();
    e->data -= dt;
    if (e->data < 0) {
        if (rand() % 10 == 9) {
            setSpeed(e, -e->vx, e->vy);
        }
        if (rand() % 10 == 9) {
            setSpeed(e, e->vx, -e->vy);
        }
        e->data = 1000;
    }

    if (e->state > FIREBALL_MOVING)
        e->state += dt;
}


static const int DROP_WAITING = 0;
static const int DROP_CREATE = 1000;
static const int DROP_FALLING = 1001;
static const int DROP_FELL = 5000;

void Drop_onInit( Object* e )
{
    e->state = -rand() % 2000;
}

void Drop_onFrame( Object* e )
{
    if (e->state <= DROP_WAITING) {
        e->state += getElapsedFrameTime();
        if (e->state > DROP_CREATE) {
            e->state = DROP_CREATE;
        }

    } else if (e->state <= DROP_CREATE) {
        Object* drop = createObject(level, TYPE_DROP, 0, 0);
        drop->x = e->x;
        drop->y = e->y;
        drop->state = DROP_FALLING;
        e->state = DROP_WAITING - 2000 - rand() % 8000;

    } else if (e->state <= DROP_FALLING) {
        if (e->vy < 120) {
            e->vy += 48 * getElapsedFrameTime() / 1000.0;
        }
        if (move(e, HITTEST_WALLS | HITTEST_LEVEL)) {
            move(e, HITTEST_NONE);
            e->state = DROP_FALLING + 1;
        }

    } else if (e->state <= DROP_FELL) {
        e->state += getElapsedFrameTime();
        e->anim.alpha -= ceil(255 * getElapsedFrameTime() / (DROP_FELL - DROP_FALLING));
        if (e->anim.alpha < 0) {
            e->anim.alpha = 0;
        }

    } else {
        e->removed = 1;
    }
}

void Drop_onHit( Object* e )
{
    killPlayer();
}


static const int SPIDER_MOVING = 10000;
static const int SPIDER_WAITING = 12000;

void Spider_onFrame( Object* e )
{
    MovingEnemy_onFrame(e);

    if (rand() % 100 == 99) {
        const int direction = e->vx > 0 ? 1 : -1;
        if (fabs(e->vx) == e->type->speed) {
            setSpeed(e, direction * e->type->speed * 2.5, e->vy);
        } else {
            setSpeed(e, direction * e->type->speed, e->vy);
        }
    }
}


static const int TELEPORTINGENEMY_MOVING = 4000;
static const int TELEPORTINGENEMY_BEFORE_TELEPORT = 7000;
static const int TELEPORTINGENEMY_TELEPORT = 8000;
static const int TELEPORTINGENEMY_AFTER_TELEPORT = 9000;

void TeleportingEnemy_onFrame( Object* e )
{
    if (e->state <= TELEPORTINGENEMY_MOVING) {
        if (move(e, HITTEST_ALL)) {
            setSpeed(e, -e->vx, e->vy);
        }
        setAnimation(e, 1, 2, speedToFps(e->vx));

    } else if (e->state <= TELEPORTINGENEMY_BEFORE_TELEPORT) {
        setAnimation(e, 2, 2, 0);
        e->anim.alpha -= ceil(255 * getElapsedFrameTime() / (TELEPORTINGENEMY_TELEPORT - TELEPORTINGENEMY_BEFORE_TELEPORT));
        if (e->anim.alpha < 0) {
            e->anim.alpha = 0;
        }

    } else if (e->state <= TELEPORTINGENEMY_TELEPORT) {
        const int currentRow = (e->y + CELL_HALF) / CELL_SIZE;
        for (int i = 0; i < CELL_COUNT; i++) {
            const int r = rand() % (ROW_COUNT - 1);
            const int c = rand() % COLUMN_COUNT;
            if (r == currentRow) {
                continue;
            }
            const int canStand     = !isSolid(r, c,     SOLID_ALL)   && isSolid(r + 1, c,     SOLID_TOP);
            const int canMoveLeft  = !isSolid(r, c - 1, SOLID_RIGHT) && isSolid(r + 1, c - 1, SOLID_TOP);
            const int canMoveRight = !isSolid(r, c + 1, SOLID_LEFT)  && isSolid(r + 1, c + 1, SOLID_TOP);
            if (canStand && (canMoveLeft || canMoveRight)) {
                e->y = CELL_SIZE * r;
                e->x = CELL_SIZE * c;
                break;
            }
        }
        e->state = TELEPORTINGENEMY_TELEPORT + 1;
        e->anim.alpha = 0;

    } else if (e->state <= TELEPORTINGENEMY_AFTER_TELEPORT) {
        e->anim.alpha += ceil(255 * getElapsedFrameTime() / (TELEPORTINGENEMY_AFTER_TELEPORT - TELEPORTINGENEMY_TELEPORT));
        if (e->anim.alpha > 255) {
            e->anim.alpha = 255;
        }

    } else {
        e->state = -rand() % 2000;
        e->anim.alpha = 255;
    }

    e->state += getElapsedFrameTime();
}

void TeleportingEnemy_onHit( Object* e )
{
    if (e->state <= TELEPORTINGENEMY_MOVING) {
        MovingEnemy_onHit(e);
    }
}


void Platform_onInit( Object* e )
{
    setSpeed(e, e->type->speed, 0);
}

void Platform_onFrame( Object* e )
{
    if (move(e, HITTEST_WALLS | HITTEST_LEVEL)) {
        e->vx = -e->vx;
        e->vy = -e->vy;
    }
}

void Platform_onHit( Object* e )
{
    const double dt = getElapsedFrameTime() / 1000.0;
    const double dw = (CELL_SIZE - player.type->body.w) / 2.0;
    const double dh = (CELL_SIZE - player.type->body.h) / 2.0;
    const double border = 3;

    Borders pb, eb;
    getObjectBody((Object*)&player, &pb);
    getObjectBody(e, &eb);

    const int hitX = pb.right >= (eb.left + border) && pb.left <= (eb.right - border);
    const int hitY = pb.bottom >= (eb.top + border) && pb.top <= (eb.bottom - border);

    // Top
    if (pb.bottom > eb.top && pb.bottom < eb.bottom && hitX) {
        if (!player.vx) {
            player.x += e->vx * dt;
        }
        player.y = eb.top - dh - player.type->body.h;
        player.inAir = 0;
    // Bottom
    } else if (pb.top < eb.bottom && pb.top > eb.top && hitX) {
        player.y = eb.bottom - dh;
    // Left
    } else if (pb.right > eb.left && pb.right < eb.right && hitY) {
        player.x = eb.left - dw - player.type->body.w;
    // Right
    } else if (pb.left < eb.right && pb.left > eb.left && hitY) {
        player.x = eb.right - dw;
    }
}


void Spring_onInit( Object* e )
{
}

void Spring_onFrame( Object* e )
{
    if (e->state > 0) {
        e->state -= getElapsedFrameTime();
    } else {
        setAnimation(e, 0, 0, 0);
        e->state = 0;
    }
}

void Spring_onHit( Object* e )
{
    if (e->state == 0 && player.vy > 48) {
        player.vy = -15 * 24;
        e->state = 1000;
        setAnimation(e, 1, 1, 0);
    }
}


void Cloud_onHit( Object* e )
{
    if (player.y + CELL_HALF < e->y + CELL_SIZE) {
        if (player.vy > 0) {
            player.y -= player.vy * 0.9 * getElapsedFrameTime() / 1000.0;
        }
        player.inAir = 0;
    }
}


void Torch_onInit( Object* e )
{
    setAnimation(e, 0, 1, 4);
}

void Torch_onHit( Object* e )
{
}


void Water_onInit( Object* e )
{
    setAnimationWave(e, 24);
}

void Water_onHit( Object* e )
{
    int er, ec;
    getObjectCell(e, &er, &ec);

    int pr, pc;
    getObjectCell((Object*)&player, &pr, &pc);

    if (er == pr) {
        killPlayer();
    }
}
