/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#include "game.h"
#include "helpers.h"
#include "render.h"
#include "levels.h"
#include <windows.h>
#include <stdio.h>

int gameOver = 0;

void processPlayer()
{
    static const int dw = (CELL_SIZE - PLAYER_WIDTH) / 2;
    static const int dh = (CELL_SIZE - PLAYER_HEIGHT) / 2;
    const int prevX = player.x;
    int r, c, cell[4], body[4] /*unused*/;
    getObjectPos((Object*)&player, &r, &c, cell, body);

    // Movement
    player.x += player.vx;
    // ... Left
    if (player.x < cell[0]) {
        if (isSolid(r, c - 1) ||
            (player.y + dh < cell[2] && isSolid(r - 1, c - 1)) ||
            (player.y + CELL_SIZE - dh > cell[3] && isSolid(r + 1, c - 1)) ) {
            player.x = cell[0];
            player.vx = 0;
        }
    // ... Right
    } else if (player.x + CELL_SIZE > cell[1]) {
        if (isSolid(r, c + 1) ||
            (player.y + dh < cell[2] && isSolid(r - 1, c + 1)) ||
            (player.y + CELL_SIZE - dh > cell[3] && isSolid(r + 1, c + 1)) ) {
            player.x = cell[0];
            player.vx = 0;
        }
    }

    player.y += player.vy;
    // ... Bottom
    if (player.y + CELL_SIZE > cell[3]) {
        if (isSolid(r + 1, c) ||
            (player.x + dw < cell[0] && isSolid(r + 1, c - 1)) ||
            (player.x - dw + CELL_SIZE > cell[1] && isSolid(r + 1, c + 1)) ||
            (!player.onLadder && isSolidLadder(r + 1, c)) ) {
            player.y = cell[2];
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
    } else if (player.y < cell[2]) {
        if (isSolid(r - 1, c) ||
            (player.x + dw < cell[0] && isSolid(r - 1, c - 1)) ||
            (player.x - dw + CELL_SIZE > cell[1] && isSolid(r - 1, c + 1)) ) {
            player.y = cell[2];
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
        // Player gets off the ladder. Or it has just got on the ladder by
        // pressing "down", but still stays in non-ladder cell - then this
        // condition will take it off the ladder. That's not good, but if
        // we hold "down", on next frame player will climb down further.
        if (!isLadder(r, c)) {
            player.onLadder = 0;
            setAnimation((Object*)&player, 0, 0, 4);
            if (player.vy < 0) {
                player.vy = 0;
                player.y = CELL_SIZE * r;
            }
        }
    }

    // Screen borders
    // ... Left
    if (player.x + CELL_HALF < 0) {
        if (level->c > 0) {
            setLevel(level->r, level->c - 1);
            player.x = LEVEL_WIDTH - CELL_SIZE + CELL_HALF - 1;

            // To change position of objects we can simply play N frames
            //for (i = 0; i < N; ++ i) {
            //  processObjects();
            //}
            return;
        } else {
            player.x = prevX;
        }
    // ... Right
    } else if (player.x + CELL_HALF > LEVEL_WIDTH) {
        if (level->c < LEVEL_XCOUNT - 1) {
            setLevel(level->r, level->c + 1);
            player.x = -CELL_HALF + 1;
            return;
        } else {
            player.x = prevX;
        }
    // ... Bottom
    } else if (player.y + CELL_HALF > LEVEL_HEIGHT) {
        if (level->r < LEVEL_YCOUNT - 1) {
            setLevel(level->r + 1, level->c);
            player.y = -CELL_HALF + 1;
            return;
        } else {
            gameOver = 1;
        }
    // ... Top
    } else if (player.y + CELL_HALF < 0) {
        if (level->r > 0) {
            setLevel(level->r - 1, level->c);
            player.y = LEVEL_HEIGHT - CELL_SIZE + CELL_HALF - 1;
            return;
        } else {
            // Player will simply fall down
        }
    }
}

void processObjects()
{
    int i;
    for (i = 0; i < level->objects.count; ++ i) {
        Object* obj = level->objects.array[i];
        if (obj == (Object*)&player || obj->removed) {
            continue;
        }
        obj->type->onFrame(obj);
        if (abs(obj->x - player.x) < (PLAYER_WIDTH + obj->type->width) / 2 &&
            abs(obj->y - player.y) < (PLAYER_HEIGHT + obj->type->height) / 2) {
            obj->type->onHit(obj, (Object*)&player);
        }
    }
}

void killPlayer()
{
    setAnimation((Object*)&player, 5, 5, 5);
    gameOver = 1;
}

// Whether or not to use system timer to update frames. If defined, system
// timer period will be decreased to 1 ms, so the timer is able to awake
// our game every FRAME_PERIOD ms. In fact, 10 ms period is enough to awake
// the game every 20 ms (i.e. 50 fps), but 1 ms is used for reliability.
// Remember that lower period causes higher power consumption.
#define USE_SYSTEM_TIMER

void on_exit()
{
    #ifdef USE_SYSTEM_TIMER
    timeEndPeriod(SYSTEM_TIMER_PERIOD);
    #endif
}

void gameLoop()
{
    SDL_Event event;
    SDL_Rect levelRect = {0, 0, LEVEL_WIDTH, LEVEL_HEIGHT};
    const Uint8* keystate;
    const int playerSpeed[2] = {3, 2}; // {4, 3} can be better
    const int playerAnimSpeed[2] = {6, 6}; // {5, 5}
    int playing = 1;
    int hideScreenCounter = -50;
    int jumpDenied = 0;
    int ladderTimer = 0;
    int frameCount = 0;
    int cleanCounter = 0;
    Uint32 startTime;
    #ifndef USE_SYSTEM_TIMER
    Uint32 prevRenderTime;
    Uint32 currentTime;
    #endif

    atexit(on_exit);
    SDL_CreateWindowAndRenderer(LEVEL_WIDTH, LEVEL_HEIGHT, 0, &window, &renderer);
    initRender();
    initTypes();
    initLevels();
    keystate = SDL_GetKeyboardState(NULL);

    player.type = &objectTypes[TYPE_PLAYER];
    player.anim.frameDelayCounter = 0;
    player.x = CELL_SIZE * 0;
    player.y = CELL_SIZE * 0;
    player.removed = 0;
    player.inAir = 0;
    player.onLadder = 0;
    player.lives = 3;
    player.coins = 0;
    player.keys = 0;

    #ifdef USE_SYSTEM_TIMER
    timeBeginPeriod(SYSTEM_TIMER_PERIOD);
    #else
    prevRenderTime = SDL_GetTicks();
    #endif

    startTime = SDL_GetTicks();

    while (playing) {
        #ifdef USE_SYSTEM_TIMER
        // Frame delay (this way does not use CPU between frames)
        SDL_Delay(FRAME_PERIOD);
        #endif

        // Read all events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                playing = 0;
            }
        }

        // Game over
        if (gameOver) {
            hideScreenCounter += 2;
            if (hideScreenCounter < 0)   continue;
            if (hideScreenCounter > 255) break;
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 25 * (hideScreenCounter / 25));
            SDL_RenderClear(renderer);
            drawScreen();
            SDL_RenderFillRect(renderer, &levelRect);
            SDL_RenderPresent(renderer);
            continue;
        }

        // Check keyboard
        // ... Left
        if (keystate[SDL_SCANCODE_LEFT]) {
            if (!player.onLadder) {
                if (!player.inAir) {
                    setAnimation((Object*)&player, 1, 2, playerAnimSpeed[0]);
                } else {
                    setAnimation((Object*)&player, 1, 1, playerAnimSpeed[0]);
                }
            }
            player.anim.direction = -1;
            player.vx = -playerSpeed[0];
            //jumpDenied = 0;
        // ... Right
        } else if (keystate[SDL_SCANCODE_RIGHT]) {
            if (!player.onLadder) {
                if (!player.inAir) {
                    setAnimation((Object*)&player, 1, 2, playerAnimSpeed[0]);
                } else {
                    setAnimation((Object*)&player, 1, 1, playerAnimSpeed[0]);
                }
            }
            player.anim.direction = 1;
            player.vx = playerSpeed[0];
            //jumpDenied = 0;
        // ... Not left or right
        } else if (player.vx || player.anim.frameStart == 1) {
            if (!player.onLadder) {
                setAnimation((Object*)&player, 0, 0, 0);
            }
            player.vx = 0;
        }
        // ... Up
        if (keystate[SDL_SCANCODE_UP]) {
            int r = (player.y + CELL_HALF) / CELL_SIZE;
            int c = (player.x + CELL_HALF) / CELL_SIZE;
            if (!isLadder(r, c)) {
                if (!player.inAir && !player.onLadder && !jumpDenied) {
                    player.vy = -9; // -10 can be better
                }
            } else {
                player.onLadder = 1;
                player.vy = -playerSpeed[1];
                player.x = c * CELL_SIZE;
                setAnimation((Object*)&player, 3, 3, playerAnimSpeed[1]);
                if (ladderTimer ++ >= 8) {
                    ladderTimer = 0;
                    player.anim.direction *= -1;
                }
                jumpDenied = 1;
            }
        // ... Down
        } else if (keystate[SDL_SCANCODE_DOWN]) {
            int r = (player.y + CELL_HALF) / CELL_SIZE;
            int c = (player.x + CELL_HALF) / CELL_SIZE;
            if (player.onLadder || isLadder(r + 1, c)) {
                player.onLadder = 1;
                player.vy = playerSpeed[1];
                player.x = c * CELL_SIZE;
                setAnimation((Object*)&player, 3, 3, playerAnimSpeed[1]);
                if (ladderTimer ++ >= 8) {
                    ladderTimer = 0;
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
        // ... Open door
        if (keystate[SDL_SCANCODE_SPACE]) {
            int r = (player.y + CELL_HALF) / CELL_SIZE;
            int c = (player.x + CELL_HALF) / CELL_SIZE;
            if (findNearDoor(&r, &c)) {
                if (player.keys > 0) {
                    player.keys -= 1;
                    level->map[r][c] = &objectTypes[TYPE_NONE];
                }
            }
        }
        /*
        // ... Attack
        if (keystate[SDL_SCANCODE_LCTRL]) {
            //if (!player.attack) {
                player.attack = 18;
            //}
        }
        if (player.attack) {
            if (player.attack -- > 12) {
                setAnimation((Object*)&player, 4, 4, 5);
            } else if (player.anim.frameStart == 4) {
                setAnimation((Object*)&player, 0, 0, 5);
            }
        }
        */

        /*
        if (player.inAir) {
            setAnimation((Object*)&player, 1, 1, playerAnimSpeed[0]);
        }
        */

        // Just for test, remove it later
        if (keystate[SDL_SCANCODE_R]) {
            player.x = CELL_SIZE * 10;
            player.y = CELL_SIZE * 5;
        }
        if (keystate[SDL_SCANCODE_N]) {
            if (!player.inAir) {
                createObjectInMap(level, TYPE_WALL, rand() % ROW_COUNT, rand() % COLUMN_COUNT);
                player.inAir = 1;
            }
        }
        //

        #ifndef USE_SYSTEM_TIMER
        // Frame delay (this way uses CPU all the time)
        currentTime = SDL_GetTicks();
        if (currentTime - prevRenderTime < FRAME_PERIOD) {
            continue;
        }
        prevRenderTime = currentTime;
        #endif

        // Movement and objects
        processPlayer();
        processObjects();

        // Rendering
        SDL_SetRenderDrawColor(renderer,
                (level->background & 0xFF0000) >> 16,
                (level->background & 0x00FF00) >> 8,
                (level->background & 0x0000FF),
                255);
        SDL_RenderClear(renderer);
        drawScreen();

        // Earthquake
        /*
        if (rand() % 8 == 0) {
            SDL_Rect rect = {rand() % 4, rand() % 4, LEVEL_WIDTH, LEVEL_HEIGHT};
            SDL_RenderSetViewport(renderer, &rect);
        } else {
            SDL_Rect rect = {0, 0, LEVEL_WIDTH, LEVEL_HEIGHT};
            SDL_RenderSetViewport(renderer, &rect);
        }
        //*/

        SDL_RenderPresent(renderer);

        // Delete removed objects from memory
        if (++ cleanCounter > 500) {
            cleanCounter = 0;
            cleanArray(&level->objects);
        }

        frameCount += 1;
    }

    printf("%f\n", frameCount / ((SDL_GetTicks() - startTime) / 1000.0));

    #ifdef USE_SYSTEM_TIMER
    timeEndPeriod(SYSTEM_TIMER_PERIOD);
    #endif
}
