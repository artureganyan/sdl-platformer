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

static const SDL_Color TEXT_COLOR = {255, 255, 255, 255};
static const SDL_Color BOX_CONTENT_COLOR = {0, 0, 0, 255};
static const SDL_Color BOX_BORDER_COLOR = {255, 255, 255, 255};
static const int BOX_BORDER = 2;

enum
{
    TEXT_CACHE_SIZE = 8
};

typedef struct
{
    const char* text;
    SDL_Texture* texture;
} TextTexture;

static struct
{
    TextTexture textures[TEXT_CACHE_SIZE];
    int next;
} textCache;


void initRender()
{
    // Window and renderer
    SDL_CreateWindowAndRenderer(LEVEL_WIDTH * SIZE_FACTOR, LEVEL_HEIGHT * SIZE_FACTOR, 0, &window, &renderer);

    // Sprites
    static const char* spritesPath = "image/sprites.bmp";
    static const Uint8 transparent[3] = {90, 82, 104};
    SDL_Surface* surface = SDL_LoadBMP(spritesPath);
    ensure(surface != NULL,  "initRender(): Can't load sprite sheet");
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, transparent[0], transparent[1], transparent[2]));
    sprites = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    // Font
    TTF_Init();
    font = TTF_OpenFont("font/PressStart2P.ttf", 12);
    ensure(font != NULL, "initRender(): Can't load font");
}

void drawSprite( SDL_Rect* spriteRect, int x, int y, int frame, SDL_RendererFlip flip )
{
    SDL_Rect srcRect = *spriteRect;
    srcRect.x += srcRect.w * frame;
    SDL_Rect dstRect = {x * SIZE_FACTOR, y * SIZE_FACTOR, srcRect.w * SIZE_FACTOR, srcRect.h * SIZE_FACTOR};
    SDL_RenderCopyEx(renderer, sprites, &srcRect, &dstRect, 0, NULL, flip);
}

static void drawObjectBody( Object* object, int x, int y )
{
    SDL_Rect body = {x + object->type->body.x, y + object->type->body.y,
                     object->type->body.w, object->type->body.h};

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderDrawRect(renderer, &body);
}

// x and y are specified to draw the object animation at arbitrary position,
// e.g. to make special effects
void drawObject( Object* object, int x, int y )
{
    const int frame = object->anim.frame;
    const int flip = object->anim.flip;

    SDL_SetTextureAlphaMod(sprites, object->anim.alpha);

    if (object->anim.type == ANIMATION_WAVE) {
        SDL_Rect spriteRect = object->type->sprite;
        spriteRect.w -= frame;
        x += frame;
        drawSprite(&spriteRect, x, y, 0, flip);

        spriteRect.x += spriteRect.w;
        spriteRect.w = frame;
        x -= frame;
        drawSprite(&spriteRect, x, y, 0, flip);
    } else {
        drawSprite(&object->type->sprite, x, y, frame, flip);
    }

//  drawObjectBody(object, x, y);

    SDL_SetTextureAlphaMod(sprites, 255);
}

static void drawBoxEx( int x, int y, int w, int h, int border, SDL_Color borderColor, SDL_Color contentColor )
{
    const SDL_Rect borderRect = {x - border, y - border, w + border * 2, h + border * 2};
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderFillRect(renderer, &borderRect);

    const SDL_Rect contentRect = {x, y, w, h};
    SDL_SetRenderDrawColor(renderer, contentColor.r, contentColor.g, contentColor.b, contentColor.a);
    SDL_RenderFillRect(renderer, &contentRect);
}

static void drawBox( int x, int y, int w, int h )
{
    drawBoxEx(x, y, w, h, BOX_BORDER, BOX_BORDER_COLOR, BOX_CONTENT_COLOR);
}

static SDL_Texture* createText( const char* text, SDL_Color color )
{
    if (!text) {
        return NULL;
    }

    // Split text into lines and get the maximum line width
    int maxLineCount = 16;
    int lineCount = 0;
    int textWidth = 0;
    char** lines = (char**)malloc(maxLineCount * sizeof(char*));

    for (int i = 0, lineSize = 0; ; ++ i, ++ lineSize) {
        const char c = text[i];
        if (c == '\n' || c == '\0') {
            // ... Create the line
            char* line = (char*)malloc(lineSize + 1);
            strncpy(line, &text[i - lineSize], lineSize);
            line[lineSize] = 0;
            lines[lineCount ++] = line;
            // ... Calculate its texture size
            int w, h;
            TTF_SizeText(font, line, &w, &h);
            if (textWidth < w) {
                textWidth = w;
            }
            // ... Prepare for the next line
            lineSize = -1;
            if (lineCount == maxLineCount) {
                maxLineCount *= 2;
                lines = (char**)realloc(lines, maxLineCount * sizeof(char*));
            }
            if (c == '\0') {
                break;
            }
        }
    }

    // Create texture
    const int LINE_HEIGHT = TTF_FontHeight(font);
    const int LINE_SPACING = 10;
    const int textHeight = (LINE_HEIGHT + LINE_SPACING) * lineCount - LINE_SPACING;
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET, textWidth, textHeight);

    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    for (int i = 0; i < lineCount; ++ i) {
        int w, h;
        TTF_SizeText(font, lines[i], &w, &h);
        SDL_Rect destRect = {(textWidth - w) / 2, (LINE_HEIGHT + LINE_SPACING) * i, w, h};
        SDL_Surface* lineSurface = TTF_RenderText_Solid(font, lines[i], color);
        SDL_Texture* lineTexture = SDL_CreateTextureFromSurface(renderer, lineSurface);
        SDL_RenderCopy(renderer, lineTexture, NULL, &destRect);
        SDL_FreeSurface(lineSurface);
        SDL_DestroyTexture(lineTexture);
        free(lines[i]);
    }

    free(lines);
    SDL_SetRenderTarget(renderer, NULL);

    return texture;
}

// Draws text at (x, y) or at the center of rectangle (x, y, w, h) if w or h > 0.
// If withBox is 1, draws a box around the text.
static void _drawText( SDL_Texture* text, int x, int y, int w, int h, int withBox )
{
    x *= SIZE_FACTOR;
    y *= SIZE_FACTOR;
    w *= SIZE_FACTOR;
    h *= SIZE_FACTOR;

    SDL_Rect textRect = {x, y};
    SDL_QueryTexture(text, NULL, NULL, &textRect.w, &textRect.h);
    if (withBox) {
        const int padding = 10;
        const SDL_Rect defaultRect = {x, y, CELL_SIZE * SIZE_FACTOR * 5, CELL_SIZE * SIZE_FACTOR * 2.5};
        SDL_Rect boxRect = textRect;
        if (boxRect.w > defaultRect.w) {
            boxRect.w += padding * 2;
        } else {
            boxRect.w = defaultRect.w;
        }
        if (boxRect.h > defaultRect.h) {
            boxRect.h += padding * 2;
        } else {
            boxRect.h = defaultRect.h;
        }
        if (w > 0) boxRect.x += (w - boxRect.w) / 2;
        if (h > 0) boxRect.y += (h - boxRect.h) / 2;
        drawBox(boxRect.x, boxRect.y, boxRect.w, boxRect.h);
    }
    if (w > 0) textRect.x += (w - textRect.w) / 2;
    if (h > 0) textRect.y += (h - textRect.h) / 2;
    SDL_RenderCopy(renderer, text, NULL, &textRect);
}

// Draws the text and caches its texture.
//
// Requirements: If a string pointed by the parameter "text" is changed dynamically,
// its address should also be changed. The string address is cached, and later any
// string with the same address will be drawn from cache, unchanged.
//
void drawTextEx( const char* text, int x, int y, int w, int h, int withBox )
{
    if (!text) {
        return;
    }
    SDL_Texture* texture = NULL;
    for (int i = 0; i < TEXT_CACHE_SIZE; ++ i) {
        TextTexture* t = &textCache.textures[i];
        if (t->text == text) {
            texture = t->texture;
            break;
        }
    }
    if (!texture) {
        TextTexture* t = &textCache.textures[textCache.next];
        if (t->texture) {
            SDL_DestroyTexture(t->texture);
        }
        t->texture = createText(text, TEXT_COLOR);
        t->text = text;
        texture = t->texture;
        textCache.next = (textCache.next + 1) % TEXT_CACHE_SIZE;
    }
    _drawText(texture, x, y, w, h, withBox);
}

void drawText( const char* text )
{
    drawTextEx(text, 0, 0, LEVEL_WIDTH, LEVEL_HEIGHT, 1);
}

void drawScreen()
{
    // Level
    for (int r = 0; r < ROW_COUNT; ++ r) {
        for (int c = 0; c < COLUMN_COUNT; ++ c) {
            ObjectType* type = level->cells[r][c];
            drawSprite(&type->sprite, CELL_SIZE * c, CELL_SIZE * r, 0, SDL_FLIP_NONE);
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
        drawObject(object, object->x, object->y);
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
