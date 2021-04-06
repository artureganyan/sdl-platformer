/******************************************************************************
 * Copyright (c) Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#include "render.h"
#include "game.h"
#include "framecontrol.h"
#include "helpers.h"
#include "SDL_ttf.h"
#include <string.h>
#include <stdio.h>

SDL_Renderer* renderer;
static SDL_Texture* sprites;
static SDL_Window* window;
static TTF_Font* font;
static SDL_Texture* messages[MESSAGE_COUNT];

static const SDL_Color TEXT_COLOR = {255, 255, 255, 255};
static const SDL_Color TEXT_BOX_CONTENT_COLOR = {0, 0, 0, 255};
static const SDL_Color TEXT_BOX_BORDER_COLOR = {255, 255, 255, 255};
static const int TEXT_BOX_BORDER = 1 * SIZE_FACTOR;
static const int TEXT_BOX_PADDING = 5 * SIZE_FACTOR;
static const int TEXT_FONT_SIZE = 8 * SIZE_FACTOR;


// The text must be one-line
static void initMessage( MessageId id, const char* text )
{
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, TEXT_COLOR);
    messages[id] = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
}

void initRender( const char* spritesPath, const char* fontPath )
{
    // Window and renderer
    SDL_CreateWindowAndRenderer(LEVEL_WIDTH * SIZE_FACTOR, LEVEL_HEIGHT * SIZE_FACTOR, 0, &window, &renderer);

    // Sprites
    static const Uint8 transparent[3] = {90, 82, 104};
    SDL_Surface* surface = SDL_LoadBMP(spritesPath);
    ensure(surface != NULL,  "initRender(): Can't load sprite sheet");
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, transparent[0], transparent[1], transparent[2]));
    sprites = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    // Font
    TTF_Init();
    font = TTF_OpenFont(fontPath, TEXT_FONT_SIZE);
    ensure(font != NULL, "initRender(): Can't open font");

    // Messages
    initMessage(MESSAGE_PLAYER_KILLED,  "You lost a life");
    initMessage(MESSAGE_GAME_OVER,      "Game over");
    initMessage(MESSAGE_LEVEL_COMPLETE, "Level complete!");
}

void drawSprite( SDL_Rect spriteRect, int x, int y, int frame, SDL_RendererFlip flip )
{
    spriteRect.x += spriteRect.w * frame;
    SDL_Rect dstRect = {x * SIZE_FACTOR, y * SIZE_FACTOR, spriteRect.w * SIZE_FACTOR, spriteRect.h * SIZE_FACTOR};
    SDL_RenderCopyEx(renderer, sprites, &spriteRect, &dstRect, 0, NULL, flip);
}

static void drawObjectBody( Object* object )
{
    SDL_Rect body = {(object->x + object->type->body.x) * SIZE_FACTOR,
                     (object->y + object->type->body.y) * SIZE_FACTOR,
                     object->type->body.w * SIZE_FACTOR,
                     object->type->body.h * SIZE_FACTOR};

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderDrawRect(renderer, &body);
}

void drawObject( Object* object )
{
    const int frame = object->anim.frame;
    const int flip = object->anim.flip;
    const int x = object->x;
    const int y = object->y;

    SDL_SetTextureAlphaMod(sprites, object->anim.alpha);

    if (object->anim.type == ANIMATION_WAVE) {
        SDL_Rect spriteRect = object->type->sprite;
        spriteRect.w -= frame;
        drawSprite(spriteRect, x + frame, y, 0, flip);

        spriteRect.x += spriteRect.w;
        spriteRect.w = frame;
        drawSprite(spriteRect, x, y, 0, flip);
    } else {
        drawSprite(object->type->sprite, x, y, frame, flip);
    }

#ifdef DEBUG_MODE
    drawObjectBody(object);
#endif

    SDL_SetTextureAlphaMod(sprites, 255);
}

static void drawBox( SDL_Rect box, int border, SDL_Color borderColor, SDL_Color contentColor )
{
    const SDL_Rect borderRect = {box.x - border, box.y - border, box.w + border * 2, box.h + border * 2};
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderFillRect(renderer, &borderRect);

    SDL_SetRenderDrawColor(renderer, contentColor.r, contentColor.g, contentColor.b, contentColor.a);
    SDL_RenderFillRect(renderer, &box);
}

void drawMessage( MessageId id )
{
    SDL_Texture* texture = messages[id];

    SDL_Rect textRect = {0, 0};
    SDL_QueryTexture(texture, NULL, NULL, &textRect.w, &textRect.h);
    textRect.x = (SIZE_FACTOR * LEVEL_WIDTH - textRect.w) / 2;
    textRect.y = (SIZE_FACTOR * LEVEL_HEIGHT - textRect.h) / 2;

    const int padding = TEXT_BOX_PADDING;
    const SDL_Rect boxRect = {textRect.x - padding, textRect.y - padding,
                              textRect.w + padding * 2, textRect.h + padding * 2};
    drawBox(boxRect, TEXT_BOX_BORDER, TEXT_BOX_BORDER_COLOR, TEXT_BOX_CONTENT_COLOR);
    
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
}

void drawScreen()
{
    // Level
    for (int r = 0; r < ROW_COUNT; ++ r) {
        for (int c = 0; c < COLUMN_COUNT; ++ c) {
            ObjectType* type = level->cells[r][c];
            drawSprite(type->sprite, CELL_SIZE * c, CELL_SIZE * r, 0, SDL_FLIP_NONE);
        }
    }

    // Objects
    const double dt = getElapsedFrameTime() / 1000.0;
    for (int i = 0; i < level->objects.count; ++ i) {
        Object* object = level->objects.array[i];
        Animation* anim = &object->anim;
        if (object->removed) {
            continue;
        }
        anim->frameDelayCounter -= dt;
        if (anim->frameDelayCounter <= 0) {
            anim->frameDelayCounter = anim->frameDelay;
            anim->frame += 1;
            if (anim->frame > anim->frameEnd) {
                anim->frame = anim->frameStart;
            }
            if (anim->type == ANIMATION_FLIP) {
                anim->flip = anim->flip == SDL_FLIP_NONE ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
            }
        }
        drawObject(object);
    }
}

static void setAnimationEx( Object* object, int start, int end, int fps, int type )
{
    Animation* anim = &object->anim;
    anim->type = type;
    anim->frameStart = start;
    anim->frameEnd = end;
    anim->frameDelay = 1.0 / fps;
    if (anim->frame < anim->frameStart || anim->frame > anim->frameEnd) {
        anim->frame = anim->frameStart;
    }
    if (anim->frameDelayCounter > anim->frameDelay || anim->frameDelayCounter < 0) {
        anim->frameDelayCounter = anim->frameDelay;
    }
}

void setAnimation( Object* object, int frameStart, int frameEnd, int fps )
{
    setAnimationEx(object, frameStart, frameEnd, fps, ANIMATION_FRAME);
}

void setAnimationWave( Object* object, int fps )
{
    setAnimationEx(object, 0, object->type->sprite.w - 1, fps, ANIMATION_WAVE);
}

void setAnimationFlip( Object* object, int frame, int fps )
{
    setAnimationEx(object, frame, frame, fps, ANIMATION_FLIP);
}
