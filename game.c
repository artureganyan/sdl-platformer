/******************************************************************************
 * Copyright (c) Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#include "game.h"
#include "framecontrol.h"
#include "helpers.h"
#include "render.h"
#include "levels.h"
#include "SDL_ttf.h"
#include <stdio.h>
#include <math.h>

typedef enum
{
    STATE_QUIT = 0,
    STATE_PLAYING,
    STATE_KILLED,
    STATE_GAMEOVER,
    STATE_LEVELCOMPLETE
} GAME_STATE;

static struct {
    GAME_STATE state;
    const Uint8* keystate;
    struct { double x, y; } respawnPos;
    double cleanTime;
    int jumpDenied;
} game;

Level* level = 0;
Player player;

static const double PLAYER_SPEED_RUN = 72;          // Pixels per second 
static const double PLAYER_SPEED_LADDER = 48;       //
static const double PLAYER_SPEED_JUMP = 216;        //
static const double PLAYER_SPEED_FALL_MAX = 120;    //

static const double PLAYER_GRAVITY = 24 * 48;       // Pixels per second per second

static const double PLAYER_ANIM_SPEED_RUN = 8;      // Frames per second
static const double PLAYER_ANIM_SPEED_LADDER = 6;   //

static const double CLEAN_PERIOD = 10000;           // Milliseconds


void damagePlayer( int damage )
{
    if (player.invincibility > 0) {
        return;
    }
    player.health -= damage;
    if (player.health <= 0) {
        player.health = 0;
        killPlayer();
    }
}

void killPlayer()
{
    if (player.invincibility > 0) {
        return;
    }
    setAnimation((Object*)&player, 5, 5, 0);
    if (-- player.lives) {
        game.state = STATE_KILLED;
    } else {
        game.state = STATE_GAMEOVER;
    }
}

void respawnPlayer()
{
    setAnimation((Object*)&player, 0, 0, 0);
    player.invincibility = 2000;
    player.onLadder = 0;
    player.inAir = 0;
    player.x = game.respawnPos.x;
    player.y = game.respawnPos.y;
}

void setLevel( int r, int c )
{
    level = &levels[r][c];
    if (level->init) {
        level->init();
    }
}

void completeLevel()
{
    game.state = STATE_LEVELCOMPLETE;
}

static void processInput()
{
    // ... Left
    if (game.keystate[SDL_SCANCODE_LEFT]) {
        if (!player.onLadder) {
            if (!player.inAir) {
                setAnimation((Object*)&player, 1, 2, PLAYER_ANIM_SPEED_RUN);
            } else {
                setAnimation((Object*)&player, 1, 1, PLAYER_ANIM_SPEED_RUN);
            }
        }
        player.anim.flip = SDL_FLIP_HORIZONTAL;
        player.vx = -PLAYER_SPEED_RUN;

    // ... Right
    } else if (game.keystate[SDL_SCANCODE_RIGHT]) {
        if (!player.onLadder) {
            if (!player.inAir) {
                setAnimation((Object*)&player, 1, 2, PLAYER_ANIM_SPEED_RUN);
            } else {
                setAnimation((Object*)&player, 1, 1, PLAYER_ANIM_SPEED_RUN);
            }
        }
        player.anim.flip = SDL_FLIP_NONE;
        player.vx = PLAYER_SPEED_RUN;

    // ... Not left or right
    } else {
        if (!player.onLadder) {
            setAnimation((Object*)&player, 0, 0, 0);
        }
        player.vx = 0;
    }

    // ... Up
    if (game.keystate[SDL_SCANCODE_UP]) {
        int r, c;
        getObjectCell((Object*)&player, &r, &c);
        if (!isLadder(r, c)) {
            player.onLadder = 0;
            // jumpDenied prevents jump when player reaches the top of the ladder
            // by holding UP key, until this key is released
            if (!player.inAir && !game.jumpDenied) {
                player.vy = -PLAYER_SPEED_JUMP;
            }
        } else {
            player.onLadder = 1;
            player.vy = -PLAYER_SPEED_LADDER;
            player.x = c * CELL_SIZE;
            setAnimationFlip((Object*)&player, 3, PLAYER_ANIM_SPEED_LADDER);
            game.jumpDenied = 1;
        }

    // ... Down
    } else if (game.keystate[SDL_SCANCODE_DOWN]) {
        int r, c;
        getObjectCell((Object*)&player, &r, &c);
        if (isLadder(r + 1, c) || player.onLadder) {
            if (!player.onLadder) {
                player.onLadder = 1;
                player.y = r * CELL_SIZE + CELL_HALF + 1;
            }
            player.vy = PLAYER_SPEED_LADDER;
            player.x = c * CELL_SIZE;
            setAnimationFlip((Object*)&player, 3, PLAYER_ANIM_SPEED_LADDER);
        }

    // ... Not up or down
    } else {
        if (player.onLadder) {
            setAnimation((Object*)&player, 3, 3, 0);
            player.vy = 0;
        } else {
            game.jumpDenied = 0;
        }
    }

    // ... Space
    if (game.keystate[SDL_SCANCODE_SPACE]) {
        int r, c;
        getObjectCell((Object*)&player, &r, &c);
        if (findNearDoor(&r, &c)) {
            if (player.keys > 0) {
                player.keys -= 1;
                level->cells[r][c] = &objectTypes[TYPE_NONE];
            }
        }
    }

#ifdef DEBUG_MODE
    // ... F, simulate "frame by frame" mode
    if (game.keystate[SDL_SCANCODE_F]) {
        SDL_Delay(1000);
    }
#endif
}

static void processPlayer()
{
    // Movement
    const double dt = getElapsedFrameTime() / 1000.0;
    const double hitw = (CELL_SIZE - player.type->body.w) / 2;
    const double hith = hitw;

    int r, c; Borders cell, body;
    getObjectPos((Object*)&player, &r, &c, &cell, &body);

    player.vx = limitAbs(player.vx, MAX_SPEED);
    player.vy = limitAbs(player.vy, MAX_SPEED);

    // ... X
    player.x += player.vx * dt;
    Borders sprite = {player.x, player.x + CELL_SIZE, player.y, player.y + CELL_SIZE};

    // ... Left
    if (sprite.left < cell.left && player.vx <= 0) {
        if (isSolid(r, c - 1, SOLID_RIGHT) ||
            (sprite.top + hith < cell.top && isSolid(r - 1, c - 1, SOLID_RIGHT)) ||
            (sprite.bottom - hith > cell.bottom && isSolid(r + 1, c - 1, SOLID_RIGHT)) ) {
            player.x = cell.left;
            player.vx = 0;
        }
    // ... Right
    } else if (sprite.right > cell.right && player.vx >= 0) {
        if (isSolid(r, c + 1, SOLID_LEFT) ||
            (sprite.top + hith < cell.top && isSolid(r - 1, c + 1, SOLID_LEFT)) ||
            (sprite.bottom - hith > cell.bottom && isSolid(r + 1, c + 1, SOLID_LEFT)) ) {
            player.x = cell.left;
            player.vx = 0;
        }
    }

    // ... Y
    player.y += player.vy * dt;
    sprite = (Borders){player.x, player.x + CELL_SIZE, player.y, player.y + CELL_SIZE};

    // ... Bottom
    if (sprite.bottom > cell.bottom && player.vy >= 0) {
        if (isSolid(r + 1, c, SOLID_TOP) ||
            (sprite.left + hitw < cell.left && isSolid(r + 1, c - 1, SOLID_TOP)) ||
            (sprite.right - hitw > cell.right && isSolid(r + 1, c + 1, SOLID_TOP)) ||
            (!player.onLadder && isSolidLadder(r + 1, c)) ) {
            player.y = cell.top;
            player.vy = 0;
            player.inAir = 0;
            if (player.onLadder) {
                player.onLadder = 0;
                setAnimation((Object*)&player, 0, 0, 0);
            }
        } else {
            player.inAir = !player.onLadder;
        }
    // ... Top
    } else if (sprite.top < cell.top && player.vy <= 0) {
        if (isSolid(r - 1, c, SOLID_BOTTOM) ||
            (sprite.left + hitw < cell.left && isSolid(r - 1, c - 1, SOLID_BOTTOM)) ||
            (sprite.right - hitw > cell.right && isSolid(r - 1, c + 1, SOLID_BOTTOM)) ) {
            player.y = cell.top;
            player.vy += 1;
        }
        player.inAir = !player.onLadder;
    }

    // Screen borders
    getObjectCell((Object*)&player, &r, &c);

    const int lc = level->c;
    const int lr = level->r;

    // ... Left
    if (player.x < 0) {
        if (lc > 0 && !levels[lr][lc - 1].cells[r][COLUMN_COUNT - 1]->solid) {
            if (player.x + CELL_HALF < 0) {
                setLevel(lr, lc - 1);
                player.x = LEVEL_WIDTH - CELL_HALF - 1;
            }
        } else {
            player.x = 0;
        }
    // ... Right
    } else if (player.x + CELL_SIZE > LEVEL_WIDTH) {
        if (lc < LEVEL_COUNTX - 1 && !levels[lr][lc + 1].cells[r][0]->solid) {
            if (player.x + CELL_HALF > LEVEL_WIDTH) {
                setLevel(lr, lc + 1);
                player.x = -CELL_HALF + 1;
            }
        } else {
            player.x = LEVEL_WIDTH - CELL_SIZE;
        }
    }
    // ... Bottom
    if (player.y + player.type->body.h > LEVEL_HEIGHT) {
        if (lr < LEVEL_COUNTY - 1) {
            if (!levels[lr + 1][lc].cells[0][c]->solid) {
                if (player.y + player.type->body.h / 2 > LEVEL_HEIGHT) {
                    setLevel(lr + 1, lc);
                    player.y = -CELL_HALF + 1;
                }
            } else {
                player.y = LEVEL_HEIGHT - player.type->body.h;
                player.inAir = 0;
            }
        } else {
            killPlayer();
        }
    // ... Top
    } else if (player.y < 0) {
        if (lr > 0 && !levels[lr - 1][lc].cells[ROW_COUNT - 1][c]->solid) {
            if (player.y + CELL_HALF < 0) {
                setLevel(lr - 1, lc);
                player.y = LEVEL_HEIGHT - CELL_HALF - 1;
            }
        } else if (lr > 0) {
            player.y = 0;
        } else {
            // Player will simply fall down
        }
    }

    // Environment and others
    getObjectCell((Object*)&player, &r, &c);

    // ... Gravity
    if (!player.onLadder) {
        player.vy += PLAYER_GRAVITY * dt;
        if (player.vy > PLAYER_SPEED_FALL_MAX) {
            player.vy = PLAYER_SPEED_FALL_MAX;
        }
    }

    // ... Ladder
    if (player.onLadder && !isLadder(r, c)) {
        player.onLadder = 0;
        setAnimation((Object*)&player, 0, 0, 0);
        if (player.vy < 0) {
            player.vy = 0;
            player.y = CELL_SIZE * r;
        }
    }

    // ... Water
    if (isWater(r, c)) {
        killPlayer();
    }

    // ... Invincibility
    if (player.invincibility > 0) {
        player.invincibility -= dt * 1000;
        if (player.invincibility < 0) {
            player.invincibility = 0;
        }
        player.anim.alpha = 255 * (1 - (player.invincibility / 200) % 2);  // Blink each 200 ms
    }

    // ... If player stands on the ground, remember this position
    if (!player.inAir && !player.onLadder) {
        game.respawnPos.x = player.x;
        game.respawnPos.y = player.y;
    }
}

static void processObjects()
{
    for (int i = 0; i < level->objects.count; ++ i) {
        Object* object = level->objects.array[i];
        if (object == (Object*)&player || object->removed == 1) {
            continue;
        }
        object->type->onFrame(object);
        if (hitTest(object, (Object*)&player)) {
            object->type->onHit(object);
        }
    }
}

static void processFrame()
{
    // Draw screen
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    drawScreen();

    if (game.state == STATE_KILLED) {
        drawMessage(MESSAGE_PLAYER_KILLED);

    } else if (game.state == STATE_LEVELCOMPLETE) {
        drawMessage(MESSAGE_LEVEL_COMPLETE);

    } else if (game.state == STATE_GAMEOVER) {
        drawMessage(MESSAGE_GAME_OVER);
    }

    SDL_RenderPresent(renderer);

    // Read all events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            game.state = STATE_QUIT;
        }
    }

    // Process user input and game logic
    const double current_time = getElapsedTime();

    if (game.state == STATE_PLAYING) {
        processInput();
        processPlayer();
        processObjects();

    } else if (game.state == STATE_KILLED) {
        if (game.keystate[SDL_SCANCODE_SPACE]) {
            game.state = STATE_PLAYING;
            respawnPlayer();
        }

    } else if (game.state == STATE_LEVELCOMPLETE) {
        // ... Space
        if (game.keystate[SDL_SCANCODE_SPACE]) {
            game.state = STATE_QUIT;
        }

    } else if (game.state == STATE_GAMEOVER) {
        // ... Space
        if (game.keystate[SDL_SCANCODE_SPACE]) {
            game.state = STATE_QUIT;
        }
    }

    // Delete unused objects from memory
    if (current_time >= game.cleanTime) {
        game.cleanTime = current_time + CLEAN_PERIOD;
        ObjectArray_clean(&level->objects);
    }

#ifdef DEBUG_MODE
    printf("fps=%f, objects=%d\n", getCurrentFps(), level->objects.count);
#endif
}

static void onExit()
{
    stopFrameControl();
    
    TTF_Quit();
    SDL_Quit();
}

void initGame()
{
    atexit(onExit);

    initRender("image/sprites.bmp", "font/PressStart2P.ttf");
    initTypes();
    initPlayer(&player);
    initLevels();

    game.keystate = SDL_GetKeyboardState(NULL);
    game.state = STATE_PLAYING;
}

void runGame()
{
    startFrameControl(FRAME_RATE, MAX_DELTA_TIME);

    while (game.state != STATE_QUIT) {
        processFrame();
        waitForNextFrame();
    }
}
