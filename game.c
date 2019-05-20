/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#include "game.h"
#include "framecontrol.h"
#include "helpers.h"
#include "render.h"
#include "levels.h"
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

static GAME_STATE gameState = STATE_PLAYING;
static const char* message = NULL;
static const Uint8* keystate = NULL;
static FrameControl frameControl;


static void processPlayer()
{
    const int dw = (CELL_SIZE - PLAYER_WIDTH) / 2;
    const int dh = (CELL_SIZE - (PLAYER_HEIGHT - 14)) / 2;
    int r, c; Borders cell, body /*unused*/;
    getObjectPos((Object*)&player, &r, &c, &cell, &body);

    // Movement
    player.x += player.vx;
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

    player.y += player.vy;
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
                setAnimation((Object*)&player, 0, 0, 4);
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
        if (player.vy < 5) {
            player.vy += 1;
        }
    }

    // Ladder
    if (player.onLadder) {
        if (!isLadder(r, c)) {
            player.onLadder = 0;
            setAnimation((Object*)&player, 0, 0, 4);
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
    if (player.health > 100) {
        player.health -= 1;
        if (player.health % 10 == 0) {
            player.anim.alpha = 255 * !player.anim.alpha;
        }
    }

    // Screen borders
    const int lc = level->c;
    const int lr = level->r;
    getObjectCell((Object*)&player, &r, &c);

    // ... Left
    if (player.x < 0) {
        if (lc > 0 && !levels[lr][lc - 1].map[r][COLUMN_COUNT - 1]->solid) {
            if (player.x + CELL_HALF < 0) {
                setLevel(lr, lc - 1);
                player.x = LEVEL_WIDTH - CELL_HALF - 1;
            }
        } else {
            player.x = 0;
        }
    // ... Right
    } else if (player.x + CELL_SIZE > LEVEL_WIDTH) {
        if (lc < LEVEL_XCOUNT - 1 && !levels[lr][lc + 1].map[r][0]->solid) {
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
            if (!levels[lr + 1][lc].map[0][c]->solid) {
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
        if (lr > 0 && !levels[lr - 1][lc].map[ROW_COUNT - 1][c]->solid) {
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


void damagePlayer( int damage )
{
    if (player.health > 100) return;
    player.health -= damage;
    if (player.health <= 0) {
        player.health = 0;
        killPlayer();
    }
}

void killPlayer()
{
    if (player.health > 100) return;
    setAnimation((Object*)&player, 5, 5, 5);
    if (-- player.lives) {
        gameState = STATE_KILLED;
    } else {
        gameState = STATE_GAMEOVER;
    }
}

void showMessage( const char* text )
{
    message = text;
    gameState = STATE_MESSAGE;
}

void completeLevel()
{
    gameState = STATE_LEVELCOMPLETE;
}

static void onExit()
{
#ifdef _MSC_VER
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
    initPlayer();
    initLevels();
    keystate = SDL_GetKeyboardState(NULL);
}

void gameLoop()
{
    const int PLAYER_SPEED_RUN = 3;
    const int PLAYER_SPEED_LADDER = 2;
    const int PLAYER_SPEED_JUMP = 9;
    const int PLAYER_ANIM_SPEED_RUN = 6;
    const int PLAYER_ANIM_SPEED_LADDER = 8;
    const int CLEAN_PERIOD = MS_TO_FRAMES(10000);

    struct { int x, y; } prevGroundPos; // Used for respawn
    int ladderAnimTimer = 0;
    int cleanTimer = 0;
    int jumpDenied = 0;

    if (!FrameControl_start(&frameControl, FRAME_RATE)) {
        return;
    }
#ifdef _MSC_VER
    if (timeBeginPeriod(SYSTEM_TIMER_PERIOD) != TIMERR_NOERROR) {
        fprintf(stderr, "gameLoop(): timeBeginPeriod() failed\n");
        return;
    }
#endif
    while (gameState != STATE_QUIT) {

        // Draw screen
        if (gameState == STATE_PLAYING) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            drawScreen();

        } else if (gameState == STATE_MESSAGE) {
            drawText(message);

        } else if (gameState == STATE_KILLED) {
            drawText("You lost a life");

        } else if (gameState == STATE_LEVELCOMPLETE) {
            drawText("Level complete!");

        } else if (gameState == STATE_GAMEOVER) {
            drawText("Game over");
        }

        SDL_RenderPresent(renderer);

        // Read all events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                gameState = STATE_QUIT;
            }
        }

        // Process user input and game logic
        if (gameState == STATE_PLAYING) {

            // ... Left
            if (keystate[SDL_SCANCODE_LEFT]) {
                if (!player.onLadder) {
                    if (!player.inAir) {
                        setAnimation((Object*)&player, 1, 2, PLAYER_ANIM_SPEED_RUN);
                    } else {
                        setAnimation((Object*)&player, 1, 1, PLAYER_ANIM_SPEED_RUN);
                    }
                }
                player.anim.direction = -1;
                player.vx = -PLAYER_SPEED_RUN;

            // ... Right
            } else if (keystate[SDL_SCANCODE_RIGHT]) {
                if (!player.onLadder) {
                    if (!player.inAir) {
                        setAnimation((Object*)&player, 1, 2, PLAYER_ANIM_SPEED_RUN);
                    } else {
                        setAnimation((Object*)&player, 1, 1, PLAYER_ANIM_SPEED_RUN);
                    }
                }
                player.anim.direction = 1;
                player.vx = PLAYER_SPEED_RUN;

            // ... Not left or right
            } else if (player.vx || player.anim.frameStart == 1) {
                if (!player.onLadder) {
                    setAnimation((Object*)&player, 0, 0, 0);
                }
                player.vx = 0;
            }

            // ... Up
            if (keystate[SDL_SCANCODE_UP]) {
                int r, c;
                getObjectCell((Object*)&player, &r, &c);
                if (!isLadder(r, c) && !player.onLadder) {
                    if (!player.inAir && !player.onLadder && !jumpDenied) {
                        player.vy = -PLAYER_SPEED_JUMP;
                    }
                } else {
                    player.onLadder = 1;
                    player.vy = -PLAYER_SPEED_LADDER;
                    player.x = c * CELL_SIZE;
                    setAnimation((Object*)&player, 3, 3, PLAYER_ANIM_SPEED_LADDER);
                    if (ladderAnimTimer ++ >= 8) {
                        ladderAnimTimer = 0;
                        player.anim.direction *= -1;
                    }
                    jumpDenied = 1;
                }

            // ... Down
            } else if (keystate[SDL_SCANCODE_DOWN]) {
                int r, c;
                getObjectCell((Object*)&player, &r, &c);
                if (player.onLadder || isLadder(r + 1, c)) {
                    player.vy = PLAYER_SPEED_LADDER;
                    player.x = c * CELL_SIZE;
                    if (!player.onLadder) {
                        player.onLadder = 1;
                        player.y = r * CELL_SIZE + CELL_HALF + 1;
                    }
                    setAnimation((Object*)&player, 3, 3, 0);
                    if (ladderAnimTimer ++ >= PLAYER_ANIM_SPEED_LADDER) {
                        ladderAnimTimer = 0;
                        player.anim.direction *= -1;
                    }
                }

            // ... Not up or down
            } else {
                if (player.onLadder) {
                    player.vy = 0;
                }
                jumpDenied = 0;
            }

            // ... Space
            if (keystate[SDL_SCANCODE_SPACE]) {
                int r, c;
                getObjectCell((Object*)&player, &r, &c);
                if (findNearDoor(&r, &c)) {
                    if (player.keys > 0) {
                        player.keys -= 1;
                        level->map[r][c] = &objectTypes[TYPE_NONE];
                    }
                }
            }

            processPlayer();
            processObjects();

        } else if (gameState == STATE_MESSAGE) {
            // ... Space
            if (keystate[SDL_SCANCODE_SPACE]) {
                gameState = STATE_PLAYING;
            }

        } else if (gameState == STATE_KILLED) {
            // ... Space
            if (keystate[SDL_SCANCODE_SPACE]) {
                gameState = STATE_PLAYING;
                setAnimation((Object*)&player, 0, 0, 1);
                player.health = 200;
                player.onLadder = 0;
                player.inAir = 0;
                player.x = prevGroundPos.x;
                player.y = prevGroundPos.y;
            }

        } else if (gameState == STATE_LEVELCOMPLETE) {
            // ... Space
            if (keystate[SDL_SCANCODE_SPACE]) {
                break;
            }

        } else if (gameState == STATE_GAMEOVER) {
            // ... Space
            if (keystate[SDL_SCANCODE_SPACE]) {
                break;
            }
        }

        // If player stands on the ground, remember this position
        if (!player.inAir && !player.onLadder) {
            prevGroundPos.x = player.x;
            prevGroundPos.y = player.y;
        }

        // Delete unused objects from memory
        if (cleanTimer ++ >= CLEAN_PERIOD) {
            cleanTimer = 0;
            ObjectArray_clean(&level->objects);
        }

        // Frame delay
        FrameControl_waitNextFrame(&frameControl);
    }

    //printf("\n");
    //printf("fps=%f, frame count=%lu\n", FrameControl_getRealFps(&frameControl),
    //       frameControl.frameCount);
}
