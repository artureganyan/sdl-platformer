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
    STATE_MESSAGE,
    STATE_KILLED,
    STATE_GAMEOVER,
    STATE_LEVELCOMPLETE
} GAME_STATE;

static struct {
    GAME_STATE state;
    const Uint8* keystate;
    const char* message;
    struct { double x, y; } prevGroundPos; // Used for respawn
    double prevCleanTime;
    int jumpDenied;
} game;

Level* level = 0;
Player player;

static const double PLAYER_SPEED_RUN = 72;
static const double PLAYER_SPEED_LADDER = 48;
static const double PLAYER_SPEED_JUMP = 216;
static const double PLAYER_ANIM_SPEED_RUN = 8;
static const double PLAYER_ANIM_SPEED_LADDER = 6;
static const double CLEAN_PERIOD = 10; // Seconds


static void processPlayer()
{
    const double dt = getElapsedFrameTime() / 1000.0;
    const int dw = (CELL_SIZE - PLAYER_WIDTH) / 2;
    const int dh = (CELL_SIZE - (PLAYER_HEIGHT - 14)) / 2;
    int r, c; Borders cell, body /*unused*/;
    getObjectPos((Object*)&player, &r, &c, &cell, &body);

    // Movement
    player.x += player.vx * dt;
    // ... Left
    if (player.x < cell.left && player.vx <= 0) {
        if (isSolid(r, c - 1, SOLID_RIGHT) ||
            (player.y + dh < cell.top && isSolid(r - 1, c - 1, SOLID_RIGHT)) ||
            (player.y + CELL_SIZE - dh > cell.bottom && isSolid(r + 1, c - 1, SOLID_RIGHT)) ) {
            player.x = cell.left;
            player.vx = 0;
        }
    // ... Right
    } else if (player.x + CELL_SIZE > cell.right && player.vx >= 0) {
        if (isSolid(r, c + 1, SOLID_LEFT) ||
            (player.y + dh < cell.top && isSolid(r - 1, c + 1, SOLID_LEFT)) ||
            (player.y + CELL_SIZE - dh > cell.bottom && isSolid(r + 1, c + 1, SOLID_LEFT)) ) {
            player.x = cell.left;
            player.vx = 0;
        }
    }

    player.y += player.vy * dt;
    // ... Bottom
    if (player.y + CELL_SIZE > cell.bottom && player.vy >= 0) {
        if (isSolid(r + 1, c, SOLID_TOP) ||
            (player.x + dw < cell.left && isSolid(r + 1, c - 1, SOLID_TOP)) ||
            (player.x - dw + CELL_SIZE > cell.right && isSolid(r + 1, c + 1, SOLID_TOP)) ||
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
    } else if (player.y < cell.top && player.vy <= 0) {
        if (isSolid(r - 1, c, SOLID_BOTTOM) ||
            (player.x + dw < cell.left && isSolid(r - 1, c - 1, SOLID_BOTTOM)) ||
            (player.x - dw + CELL_SIZE > cell.right && isSolid(r - 1, c + 1, SOLID_BOTTOM)) ) {
            player.y = cell.top;
            player.vy += 1;
        }
        player.inAir = !player.onLadder;
    }

    // Gravity
    if (!player.onLadder) {
        if (player.vy < 120) {
            player.vy += 24 * 48 * dt;
        }
    }

    // Ladder
    if (player.onLadder) {
        if (!isLadder(r, c)) {
            player.onLadder = 0;
            setAnimation((Object*)&player, 0, 0, 0);
            if (player.vy < 0) {
                player.vy = 0;
                player.y = CELL_SIZE * r;
            }
        }
    }

    // Water
    if (isWater(r, c)) {
        killPlayer();
    }

    // Invincibility
    if (player.invincibility > 0) {
        player.invincibility -= dt * 1000;
        if (player.invincibility < 0) {
            player.invincibility = 0;
        }
        player.anim.alpha = 255 * (1 - (player.invincibility / 200) % 2);  // Blink each 200 ms
    }

    // Screen borders
    const int lc = level->c;
    const int lr = level->r;
    getObjectCell((Object*)&player, &r, &c);

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
        if (lc < LEVEL_XCOUNT - 1 && !levels[lr][lc + 1].cells[r][0]->solid) {
            if (player.x + CELL_HALF > LEVEL_WIDTH) {
                setLevel(lr, lc + 1);
                player.x = -CELL_HALF + 1;
            }
        } else {
            player.x = LEVEL_WIDTH - CELL_SIZE;
        }
    }
    // ... Bottom
    if (player.y + PLAYER_HEIGHT > LEVEL_HEIGHT) {
        if (lr < LEVEL_YCOUNT - 1) {
            if (!levels[lr + 1][lc].cells[0][c]->solid) {
                if (player.y + PLAYER_HEIGHT / 2 > LEVEL_HEIGHT) {
                    setLevel(lr + 1, lc);
                    player.y = -CELL_HALF + 1;
                }
            } else {
                player.y = LEVEL_HEIGHT - PLAYER_HEIGHT;
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
}

static void processObjects()
{
    for (int i = 0; i < level->objects.count; ++ i) {
        Object* object = level->objects.array[i];
        if (object == (Object*)&player || object->removed == 1) {
            continue;
        }
        object->type->onFrame(object);
        if (abs(object->x - player.x) < (PLAYER_WIDTH + object->type->width) / 2 &&
            abs(object->y - player.y) < (PLAYER_HEIGHT + object->type->height) / 2) {
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

    if (game.state == STATE_MESSAGE) {
        drawText(game.message);

    } else if (game.state == STATE_KILLED) {
        drawText("You lost a life");

    } else if (game.state == STATE_LEVELCOMPLETE) {
        drawText("Level complete!");

    } else if (game.state == STATE_GAMEOVER) {
        drawText("Game over");
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
    if (game.state == STATE_PLAYING) {

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
        } else if (player.vx || player.anim.frameStart == 1) {
            if (!player.onLadder) {
                setAnimation((Object*)&player, 0, 0, 0);
            }
            player.vx = 0;
        }

        // ... Up
        if (game.keystate[SDL_SCANCODE_UP]) {
            int r, c;
            getObjectCell((Object*)&player, &r, &c);
            if (!isLadder(r, c) && !player.onLadder) {
                if (!player.inAir && !player.onLadder && !game.jumpDenied) {
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
            if (player.onLadder || isLadder(r + 1, c)) {
                player.vy = PLAYER_SPEED_LADDER;
                player.x = c * CELL_SIZE;
                if (!player.onLadder) {
                    player.onLadder = 1;
                    player.y = r * CELL_SIZE + CELL_HALF + 1;
                }
                setAnimationFlip((Object*)&player, 3, PLAYER_ANIM_SPEED_LADDER);
            }

        // ... Not up or down
        } else {
            if (player.onLadder) {
                setAnimation((Object*)&player, 3, 3, 0);
                player.vy = 0;
            }
            game.jumpDenied = 0;
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

        processPlayer();
        processObjects();

    } else if (game.state == STATE_MESSAGE) {
        // ... Space
        if (game.keystate[SDL_SCANCODE_SPACE]) {
            game.state = STATE_PLAYING;
        }

    } else if (game.state == STATE_KILLED) {
        // ... Space
        if (game.keystate[SDL_SCANCODE_SPACE]) {
            game.state = STATE_PLAYING;
            setAnimation((Object*)&player, 0, 0, 0);
            player.invincibility = 2000;
            player.onLadder = 0;
            player.inAir = 0;
            player.x = game.prevGroundPos.x;
            player.y = game.prevGroundPos.y;
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

    // If player stands on the ground, remember this position
    if (!player.inAir && !player.onLadder) {
        game.prevGroundPos.x = player.x;
        game.prevGroundPos.y = player.y;
    }

    // Delete unused objects from memory
    const double current_time = getElapsedTime();
    if (current_time - game.prevCleanTime >= CLEAN_PERIOD) {
        game.prevCleanTime = current_time;
        ObjectArray_clean(&level->objects);
    }
}

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

void showMessage( const char* text )
{
    game.message = text;
    game.state = STATE_MESSAGE;
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

static void onExit()
{
#ifdef _WIN32
    timeEndPeriod(SYSTEM_TIMER_PERIOD);
#endif
    TTF_Quit();
    SDL_Quit();
}

void initGame()
{
    atexit(onExit);
    initRender();
    initTypes();
    initPlayer(&player);
    initLevels();

    game.keystate = SDL_GetKeyboardState(NULL);
    game.state = STATE_PLAYING;
}

void runGame()
{
    if (!startFrameControl(FRAME_RATE)) {
        fprintf(stderr, "gameLoop(): startFrameControl() failed\n");
        return;
    }
#ifdef _WIN32
    if (timeBeginPeriod(SYSTEM_TIMER_PERIOD) != TIMERR_NOERROR) {
        fprintf(stderr, "gameLoop(): timeBeginPeriod() failed\n");
        return;
    }
#endif
    while (game.state != STATE_QUIT) {
        processFrame();
        waitForNextFrame();
    }
}
