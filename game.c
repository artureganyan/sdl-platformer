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

enum {
    STATE_QUIT = 0,
    STATE_PLAYING,
    STATE_INVENTORY,
    STATE_MESSAGE,
    STATE_KILLED,
    STATE_GAMEOVER
} gameState = STATE_PLAYING;

Message currentMessage = MESSAGE_NONE;
SDL_Texture* currentMessageTexture = NULL;


void processPlayer()
{
    static const int dw = (CELL_SIZE - PLAYER_WIDTH) / 2;
    static const int dh = (CELL_SIZE - (PLAYER_HEIGHT - 14)) / 2;
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
        if (!isLadder(r, c) /*&& ((isLadder(r + 1, c) && player.vy < 0) ||
                                (isLadder(r - 1, c) && player.vy > 0))*/) {
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
    if (player.x + CELL_HALF < 0) {
        if (level->c > 0 && !levels[lr][lc - 1].map[r][COLUMN_COUNT - 1]->solid) {
            setLevel(lr, lc - 1);
            player.x = LEVEL_WIDTH - CELL_SIZE + CELL_HALF - 1;
        } else {
            player.x = prevX;
        }
    // ... Right
    } else if (player.x + CELL_HALF > LEVEL_WIDTH) {
        if (level->c < LEVEL_XCOUNT - 1 && !levels[lr][lc + 1].map[r][0]->solid) {
            setLevel(lr, lc + 1);
            player.x = -CELL_HALF + 1;
        } else {
            player.x = prevX;
        }
    // ... Bottom
    } else if (player.y + CELL_HALF > LEVEL_HEIGHT) {
        if (level->r < LEVEL_YCOUNT - 1) {
            if (!levels[lr + 1][lc].map[0][c]->solid) {
                setLevel(lr + 1, lc);
                player.y = -CELL_HALF + 1;
            } else {
                player.y = LEVEL_HEIGHT - CELL_HALF; // Not correct
            }
        } else {
            killPlayer();
        }
    // ... Top
    } else if (player.y + CELL_HALF < 0) {
        if (level->r > 0 && !levels[lr - 1][lc].map[ROW_COUNT - 1][c]->solid) {
            setLevel(lr - 1, lc);
            player.y = LEVEL_HEIGHT - CELL_SIZE + CELL_HALF - 1;
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

void showMessage( Message message )
{
    currentMessage = message;
    gameState = STATE_MESSAGE;
}

void showText( const char* text )
{
    if (currentMessageTexture) {
        SDL_free(currentMessageTexture);
    }
    currentMessageTexture = createText(text);
    currentMessage = MESSAGE_TEXT;
    gameState = STATE_MESSAGE;
}

void takeItem( Object* item )
{
    const ObjectTypeId generalTypeId = item->type->generalTypeId;
    const int lr = level->r;
    const int lc = level->c;
    int r, c;
    getObjectCell(item, &r, &c);

    if (generalTypeId == TYPE_ACTION) {
        if (lr == 1 && lc == 0) {
            if (item->state == '1') {
                showText("You try to move the block on the floor,\nand it finally goes.");
                level->map[r][c + 2] = level->map[r][c + 1];
                level->map[r][c + 1] = &objectTypes[TYPE_NONE];
            } else if (item->state == '2') {
                showText("The door is locked.");
            }
        }
    } else if (generalTypeId == TYPE_COIN) {
        player.coins += 1;
    } else {
        appendArray(&player.items, item);
        item->removed = 2;
        cleanArray(&level->objects);
        item->removed = 0;
    }
}

void useItem( Object* item )
{
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
        if (level->r == 0 && level->c == 0) {
            if (r == 7 && c == 5) {
                for (int tr = r + 1; tr < r + 3; ++ tr) {
                    createObjectInMap(level, TYPE_NONE, tr, c);
                }
                item->removed = 1;
                used = 1;
            }
        }
    }

    cleanArray(&player.items);
    if (!used) {
        showMessage(MESSAGE_CANNOTUSE);
    }
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
    TTF_Quit();
    SDL_Quit();
}

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
    Uint32 startTime;
    #ifndef USE_SYSTEM_TIMER
    Uint32 prevRenderTime;
    Uint32 currentTime;
    #endif

    atexit(on_exit);
    initRender();
    initTypes();

    player.type = &objectTypes[TYPE_PLAYER];
    player.anim.direction = 1;
    player.anim.frameDelayCounter = 0;
    player.removed = 0;
    player.inAir = 0;
    player.onLadder = 0;
    player.health = 100;
    player.lives = 3;
    player.coins = 0;
    initArray(&player.items);

    initLevels();

    showText("You woke up in the locked room.\nWhere are you?");

    keystate = SDL_GetKeyboardState(NULL);

    #ifdef USE_SYSTEM_TIMER
    timeBeginPeriod(SYSTEM_TIMER_PERIOD);
    #else
    prevRenderTime = SDL_GetTicks();
    #endif

    startTime = SDL_GetTicks();

    while (gameState != STATE_QUIT) {
        #ifdef USE_SYSTEM_TIMER
        // Frame delay (this way does not use CPU between frames)
        SDL_Delay(FRAME_PERIOD);
        #endif

        // Read all events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                gameState = STATE_QUIT;
            }
        }

        // Check keyboard
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
            // ... Space
            if (keystate[SDL_SCANCODE_SPACE] && keyAllowed) {
                Object* item;
                int r, c;
                getObjectCell((Object*)&player, &r, &c);
                item = findNearItem(r, c);
                if (item) {
                    takeItem(item);
                } else {
                    gameState = STATE_INVENTORY;
                    selection = 0;
                }
                prevKeyFrame = frameCount;
            }

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
                player.health = 100;
                player.x = 0;
                player.y = 0;
                player.onLadder = 0;
                player.inAir = 0;
            }
        }

        #ifndef USE_SYSTEM_TIMER
        // Frame delay (this way uses CPU all the time)
        currentTime = SDL_GetTicks();
        if (currentTime - prevRenderTime < FRAME_PERIOD) {
            continue;
        }
        prevRenderTime = currentTime;
        #endif

        if (gameState == STATE_PLAYING) {
            SDL_SetRenderDrawColor(renderer,
                    (level->background & 0xFF0000) >> 16,
                    (level->background & 0x00FF00) >> 8,
                    (level->background & 0x0000FF),
                    255);
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            processPlayer();
            processObjects();
            drawScreen();

            /*
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
            SDL_RenderFillRect(renderer, &levelRect);
            */

        } else if (gameState == STATE_INVENTORY) {
            drawInventory(selection);

        } else if (gameState == STATE_MESSAGE) {
            if (currentMessage != MESSAGE_TEXT) {
                drawMessage(currentMessage, 0, 0, LEVEL_WIDTH, LEVEL_HEIGHT, 1);
            } else {
                drawText(currentMessageTexture, 0, 0, LEVEL_WIDTH, LEVEL_HEIGHT, 1);
            }

        } else if (gameState == STATE_KILLED) {
            drawMessage(MESSAGE_LOSTLIFE, 0, 0, LEVEL_WIDTH, LEVEL_HEIGHT, 1);

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
            drawMessage(MESSAGE_GAMEOVER, 0, 0, LEVEL_WIDTH, LEVEL_HEIGHT, 0);
        }

        SDL_RenderPresent(renderer);

        // Delete removed objects from memory
        if (++ cleanTimer > 500) {
            cleanTimer = 0;
            cleanArray(&level->objects);
        }
        frameCount += 1;
    }

    printf("%f\n", frameCount / ((SDL_GetTicks() - startTime) / 1000.0));

    #ifdef USE_SYSTEM_TIMER
    timeEndPeriod(SYSTEM_TIMER_PERIOD);
    #endif
}
