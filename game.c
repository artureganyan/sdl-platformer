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
#include <math.h>

enum {
    STATE_QUIT = 0,
    STATE_PLAYING,
    STATE_INVENTORY,
    STATE_MESSAGE,
    STATE_KILLED,
    STATE_GAMEOVER
} gameState = STATE_PLAYING;

const char* message = NULL;


void processPlayer()
{
    static const int dw = (CELL_SIZE - PLAYER_WIDTH) / 2;
    static const int dh = (CELL_SIZE - (PLAYER_HEIGHT - 14)) / 2;
    int r, c, cell[4], body[4] /*unused*/;
    getObjectPos((Object*)&player, &r, &c, cell, body);

    // Movement
    player.x += player.vx;

    // \note
    // Changing each "isSolid()" to "isSolid() & 0x1" in Left and Right conditions below
    // (and Top) allows blocks which are solid only from one side (bottom) if they have
    // type->solid == 2, and retains normal solid blocks (solid == 1). Of course, it's
    // better to add appropriate version of isSolid().

    // ... Left
    if (player.x < cell[0]) {
        if (isSolid(r, c - 1) & 1 ||
            (player.y + dh < cell[2] && isSolid(r - 1, c - 1) & 1) ||
            (player.y + CELL_SIZE - dh > cell[3] && isSolid(r + 1, c - 1) & 1) ) {
            player.x = cell[0];
            player.vx = 0;
        }
    // ... Right
    } else if (player.x + CELL_SIZE > cell[1]) {
        if (isSolid(r, c + 1) & 1 ||
            (player.y + dh < cell[2] && isSolid(r - 1, c + 1) & 1) ||
            (player.y + CELL_SIZE - dh > cell[3] && isSolid(r + 1, c + 1) & 1) ) {
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
        if (!isLadder(r, c) /*&& ((isLadder(r + 1, c) && player.vy < 0) ||
                                (isLadder(r - 1, c) && player.vy > 0) ||
                                player.vx)*/) {
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
        player.removed = 1;
    }

    // Screen borders
    int lc = level->c;
    int lr = level->r;
    getObjectCell((Object*)&player, &r, &c);

    // ... Left
    if (player.x < 0) {
        if (level->c > 0 && !levels[lr][lc - 1].map[r][COLUMN_COUNT - 1]->solid) {
            if (player.x + CELL_HALF < 0) {
                setLevel(lr, lc - 1);
                player.x = LEVEL_WIDTH - CELL_HALF - 1;
            }
        } else {
            player.x = 0;
        }
    // ... Right
    } else if (player.x + CELL_SIZE > LEVEL_WIDTH) {
        if (level->c < LEVEL_XCOUNT - 1 && !levels[lr][lc + 1].map[r][0]->solid) {
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
        if (level->r < LEVEL_YCOUNT - 1) {
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
        if (level->r > 0 && !levels[lr - 1][lc].map[ROW_COUNT - 1][c]->solid) {
            if (player.y + CELL_HALF < 0) {
                setLevel(lr - 1, lc);
                player.y = LEVEL_HEIGHT - CELL_HALF - 1;
            }
        } else if (level->r > 0) {
            player.y = 0;
        } else {
            // Player will simply fall down
        }
    }
}

void processObjects()
{
    for (int i = 0; i < level->objects.count; ++ i) {
        Object* obj = level->objects.array[i];
        if (obj == (Object*)&player || obj->removed == 1) {
            continue;
        }
        obj->type->onFrame(obj);
        if (abs(obj->x - player.x) < (PLAYER_WIDTH + obj->type->width) / 2 &&
            abs(obj->y - player.y) < (PLAYER_HEIGHT + obj->type->height) / 2) {
            obj->type->onHit(obj);
        }
    }
}

void damagePlayer( int damage )
{
    player.health -= damage;
    if (player.health <= 0) {
        player.health = 0;
        killPlayer();
    }
}

void killPlayer()
{
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

int takeItem( Object* item )
{
    if (!item) return 0;

    const ObjectTypeId generalTypeId = item->type->generalTypeId;
    const int lr = level->r;
    const int lc = level->c;
    int r, c;
    getObjectCell(item, &r, &c);

    if (generalTypeId == TYPE_ACTION) {
        if (lr == 1 && lc == 4) {
            if (item->state == '1') {
                if (player.anim.direction < 0) return 0;
                showMessage("You try to move the block on the floor,\nand it finally goes.");
                level->map[r][c + 2] = level->map[r][c + 1];
                level->map[r][c + 1] = &objectTypes[TYPE_NONE];
                item->removed = 1;
            } else if (item->state == '2') {
                showMessage("The door is locked.");
            }
        }
    } else if (generalTypeId == TYPE_COIN) {
        player.coins += 1;
        item->removed = 1;
    } else {
        appendArray(&player.items, item);
        item->removed = 2;
        cleanArray(&level->objects);
        item->removed = 0;
    }
    return 1;
}

void useItem( Object* item )
{
    if (!item) return;

    const ObjectTypeId generalTypeId = item->type->generalTypeId;
    const ObjectTypeId typeId = item->type->typeId;
    int r = (player.y + CELL_HALF) / CELL_SIZE;
    int c = (player.x + CELL_HALF) / CELL_SIZE;
    int used = 0;

    if (generalTypeId == TYPE_KEY) {
        if (findNearDoor(&r, &c)) {
            level->map[r][c] = &objectTypes[TYPE_NONE];
            item->removed = 1;
            used = 1;
        }

    } else if (typeId == TYPE_LADDER_PART) {
        if (level->r == 0 && level->c == 0) {
            if (r == 7 && c == 5) {
                for (int tr = r; tr >= 2; -- tr) {
                    createObjectInMap(level, TYPE_LADDER, tr, c);
                }
                item->removed = 1;
                used = 1;
            }
        }

    } else if (typeId == TYPE_PICK) {
        if (level->r == 1 && level->c == 1) {
            if (r == 12 && c == 5) {
                showMessage("The pillar is crushed and the cave is collapsed,\nbut you managed to run back.");
//              for (int c = 3; c < 9; ++ c) {
//                  createObjectInMap(level, TYPE_NONE, 8, c);
//                  createObjectInMap(level, TYPE_NONE, 9, c);
//              }
//              for (int c = 3; c < 9; ++ c) {
//                  createObjectInMap(level, TYPE_GROUND_TOP, 10, c);
//                  createObjectInMap(level, TYPE_GROUND, 11, c - 1);
//                  createObjectInMap(level, TYPE_GROUND, 12, c - 2);
//              }
//              createObjectInMap(level, TYPE_NONE, 12, 1);
//              createObjectInMap(level, TYPE_NONE, 12, 2);

                for (int c = 1; c < 10; ++ c) {
                    createObjectInMap(level, TYPE_NONE, 7, c);
                    createObjectInMap(level, TYPE_NONE, 8, c);
                    createObjectInMap(level, TYPE_NONE, 9, c);
                    createObjectInMap(level, TYPE_NONE, 10, c);
                }
                for (int c = 1; c < 9; ++ c) {
                    if (c < 6) createObjectInMap(level, TYPE_GROUND_TOP, 10, c);
                    if (c < 8) createObjectInMap(level, TYPE_GROUND, 11, c);
                    createObjectInMap(level, TYPE_GROUND, 12, c);
                    if (c > 4) createObjectInMap(level, TYPE_GROUND, 13, c);
                    if (c > 4) createObjectInMap(level, TYPE_GROUND, 14, c);
                }
                createObjectInMap(level, TYPE_NONE, 11, 4);
                createObjectInMap(level, TYPE_NONE, 12, 2);
                createObjectInMap(level, TYPE_NONE, 12, 6);
                createObjectInMap(level, TYPE_GROUND_TOP, 9, 1);
                createObjectInMap(level, TYPE_GROUND, 10, 1);

                player.x = 9 * CELL_SIZE;
                player.y = 12 * CELL_SIZE;
                player.anim.direction = -1;
                item->removed = 1;
                used = 1;
            }
        }
    }

    cleanArray(&player.items);
    if (!used) {
        showMessage("Can not use");
    }
}

void on_exit()
{
    timeEndPeriod(SYSTEM_TIMER_PERIOD);
    TTF_Quit();
    SDL_Quit();
}

double PCFreq = 0.0;
__int64 CounterStart = 0;

void StartCounter_()
{
    LARGE_INTEGER li;
    if (!QueryPerformanceFrequency(&li)) {
        printf("Error with QueryPerformanceFrequency");
        return;
    }
    PCFreq = li.QuadPart / 1000.0;
    if (!QueryPerformanceCounter(&li)) {
        printf("Error with QueryPerformanceCounter\n");
        return;
    }
    CounterStart = li.QuadPart;
}

double GetCounter_()
{
    LARGE_INTEGER li;
    if (!QueryPerformanceCounter(&li)) {
        printf("Error with QueryPerformanceCounter\n");
        return 0;
    }
    return (li.QuadPart - CounterStart) / PCFreq;
}

#if 1
    #define InitTime StartCounter_
    #define GetTime GetCounter_
    #define Time double
#else
    #define InitTime SDL_GetTicks
    #define GetTime SDL_GetTicks
    #define Time double
#endif

typedef struct
{
    double period;
    int frame;
    int count;
} Point;

enum { POINT_COUNT = 1000 };
Point points[POINT_COUNT];
int pointCount = 0;


void gameLoop()
{
    SDL_Event event;
    SDL_Rect levelRect = {0, 0, LEVEL_WIDTH, LEVEL_HEIGHT};
    const Uint8* keystate;
    const int playerSpeed[2] = {3, 2};      // {4, 3} can be better
    const int playerAnimSpeed[2] = {6, 6};  // {5, 5}
    int hideScreenCounter = -50;
    int jumpDenied = 0;
    int ladderTimer = 0;
    int cleanTimer = 0;
    int frameCount = 0;
    int selection = 0;
    int prevKeyFrame = 0;
    int keyAllowed = 0;
    Time startTime;
    Time prevRenderTime;

    atexit(on_exit);
    initRender();
    initTypes();
    initPlayer();
    initLevels();

    showMessage("You woke up in the locked room.\nWhere are you?");

    keystate = SDL_GetKeyboardState(NULL);

    InitTime();
    timeBeginPeriod(SYSTEM_TIMER_PERIOD);
    const Time framePeriod = 1000.0 / FRAME_RATE;
    startTime = GetTime();
    prevRenderTime = startTime;

    while (gameState != STATE_QUIT) {

        // Draw screen
        if (gameState == STATE_PLAYING) {
            SDL_SetRenderDrawColor(renderer,
                    (level->background & 0xFF0000) >> 16,
                    (level->background & 0x00FF00) >> 8,
                    (level->background & 0x0000FF),
                    255);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            SDL_RenderClear(renderer);
            drawScreen();

        } else if (gameState == STATE_INVENTORY) {
            drawInventory(selection);

        } else if (gameState == STATE_MESSAGE) {
            drawText(message, 0, 0, LEVEL_WIDTH, LEVEL_HEIGHT, 1);

        } else if (gameState == STATE_KILLED) {
            drawText("You lost a life", 0, 0, LEVEL_WIDTH, LEVEL_HEIGHT, 1);

        } else if (gameState == STATE_GAMEOVER) {
            hideScreenCounter += 2;
            if (hideScreenCounter < 0)   continue;
            if (hideScreenCounter > 255) break;
            //SDL_RenderClear(renderer);
            //processPlayer();
            //processObjects();
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            drawScreen();
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 25 * (hideScreenCounter / 25));
            SDL_RenderFillRect(renderer, &levelRect);
            drawText("Game over", 0, 0, LEVEL_WIDTH, LEVEL_HEIGHT, 0);
        }

        SDL_RenderPresent(renderer);

        // Read all events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                gameState = STATE_QUIT;
            }
        }

        // Process user input and game logic
        keyAllowed = (frameCount - prevKeyFrame >= 8);

        if (gameState == STATE_PLAYING) {

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
            if (keystate[SDL_SCANCODE_UP] || keystate[SDL_SCANCODE_LCTRL]) {
                int r = (player.y + CELL_HALF) / CELL_SIZE;
                int c = (player.x + CELL_HALF) / CELL_SIZE;
                if (!isLadder(r, c) && !player.onLadder) {
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
                    // This condition can be removed if processPlayer() will be more complex
                    if (!isLadder(r, c)) {
                        player.y = r * CELL_SIZE + CELL_HALF - player.vy;
                    }
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
            // ... Space
            if (keystate[SDL_SCANCODE_SPACE] && keyAllowed) {
                int r, c;
                getObjectCell((Object*)&player, &r, &c);
                if (!takeItem(findNearItem(r, c))) {
                    gameState = STATE_INVENTORY;
                    selection = 0;
                }
                prevKeyFrame = frameCount;
            }

            processPlayer();
            processObjects();

        } else if (gameState == STATE_INVENTORY) {

            if (keyAllowed) {
                // ... Up
                if (keystate[SDL_SCANCODE_UP]) {
                    if (-- selection < 0) {
                        selection = player.items.count - 1;
                    }
                    prevKeyFrame = frameCount;
                // ... Down
                } else if (keystate[SDL_SCANCODE_DOWN]) {
                    if (++ selection >= player.items.count) {
                        selection = 0;
                    }
                    prevKeyFrame = frameCount;
                }
                // ... Space
                if (keystate[SDL_SCANCODE_SPACE]) {
                    gameState = STATE_PLAYING;
                    if (player.items.count) {
                        useItem(player.items.array[selection]);
                    }
                    prevKeyFrame = frameCount;
                }
                // ... Esc
                if (keystate[SDL_SCANCODE_ESCAPE]) {
                    gameState = STATE_PLAYING;
                    prevKeyFrame = frameCount;
                }
            }

        } else if (gameState == STATE_MESSAGE) {
            // ... Space
            if (keystate[SDL_SCANCODE_SPACE] && keyAllowed) {
                gameState = STATE_PLAYING;
                prevKeyFrame = frameCount;
            }

        } else if (gameState == STATE_KILLED) {
            // ... Space
            if (keystate[SDL_SCANCODE_SPACE] && keyAllowed) {
                gameState = STATE_PLAYING;
                prevKeyFrame = frameCount;
                setAnimation((Object*)&player, 0, 0, 1);
                // Just to avoid sticking in a wall
                for (int r = 0; r < ROW_COUNT; ++ r) {
                    for (int c = 0; c < COLUMN_COUNT; ++ c) {
                        if (!level->map[r][c]->solid) {
                            player.y = r * CELL_SIZE;
                            player.x = c * CELL_SIZE;
                            r = c = CELL_COUNT;
                        }
                    }
                }
                player.health = 100;
                player.onLadder = 0;
                player.inAir = 0;
            }
        }

        // Delete removed objects from memory
        if (++ cleanTimer > 500) {
            cleanTimer = 0;
            cleanArray(&level->objects);
        }

        // Frame delay
        const Time nextFrameTime = prevRenderTime + framePeriod;
        Time remainedTime;
        int count = 0;
        while ((remainedTime = nextFrameTime - GetTime()) > 0) {
            if (remainedTime >= 1) {
                Sleep(remainedTime);
            } else {
                count += 1;
            }
        }
        Time currentTime = GetTime();
        Time period = currentTime - prevRenderTime;
        if (fabs(period - framePeriod) >= 0.1) {
            if (pointCount < POINT_COUNT) {
                Point* p = &points[pointCount ++];
                p->period = period;
                p->frame = frameCount;
                p->count = count;
            }
        }
        prevRenderTime = currentTime;
        frameCount += 1;
    }

    for (int i = 0; i < pointCount; ++ i) {
        Point* p = &points[i];
        printf("%d) %2.2f %d  ", p->frame, p->period, p->count);
    }
    printf("\n");
    printf("%f %d\n", frameCount / ((GetTime() - startTime) / 1000.0), frameCount);

    timeEndPeriod(SYSTEM_TIMER_PERIOD);
}
