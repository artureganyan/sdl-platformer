/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#include "render.h"


SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* sprites;


void initRender()
{
    static const char* spritesFilePath = "image/sprites.bmp";
    static const Uint8 transparent[3] = {90, 82, 104};
    SDL_Surface* bmp = SDL_LoadBMP(spritesFilePath);
    SDL_SetColorKey(bmp, SDL_TRUE, SDL_MapRGB(bmp->format, transparent[0], transparent[1], transparent[2]));
    sprites = SDL_CreateTextureFromSurface(renderer, bmp);
    SDL_FreeSurface(bmp);
}

void drawSprite( SDL_Rect* spriteRect, int x, int y, int frame, SDL_RendererFlip flip )
{
    static SDL_Rect destRect = {0, 0, CELL_SIZE, CELL_SIZE};
    int prevX = spriteRect->x;
    spriteRect->x += SPRITE_SIZE * frame;
    destRect.x = x;
    destRect.y = y;
    SDL_RenderCopyEx(renderer, sprites, spriteRect, &destRect, 0, NULL, flip);
    spriteRect->x = prevX;
}

void drawObject( ObjectType* type, int x, int y, int frame, SDL_RendererFlip flip )
{
    drawSprite(&type->sprite, x, y, frame, flip);
}

void drawScreen()
{
    // Level
    int r, c, i;
    for (r = 0; r < ROW_COUNT; ++ r) {
        for (c = 0; c < COLUMN_COUNT; ++ c) {
            drawObject(level->map[r][c], CELL_SIZE * c, CELL_SIZE * r, 0, SDL_FLIP_NONE);
        }
    }

    // Objects
    for (i = 0; i < level->objects.count; ++ i) {
        Object* obj = level->objects.array[i];
        Animation* anim = &obj->anim;
        if (obj->removed) continue;
        if (anim->frameStart < anim->frameEnd) {
            if (-- anim->frameDelayCounter <= 0) {
                anim->frameDelayCounter = anim->frameDelay;
                anim->frame += 1;
                if (anim->frame > anim->frameEnd) {
                    anim->frame = anim->frameStart;
                }
            }
        }
        drawObject(obj->type, obj->x, obj->y, anim->frame,
                anim->direction < 0 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    }
}

void setAnimation( Object* object, int frameStart, int frameEnd, int frameDelay )
{
    Animation* anim = &object->anim;
    anim->frameStart = frameStart;
    anim->frameEnd = frameEnd;
    anim->frameDelay = frameDelay;
    if (anim->frame < anim->frameStart || anim->frame > anim->frameEnd) {
        anim->frame = anim->frameStart;
    }
    if (anim->frameDelayCounter > anim->frameDelay || anim->frameDelayCounter < 0) {
        anim->frameDelayCounter = anim->frameDelay;
    }
}

