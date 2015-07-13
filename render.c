/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#include "render.h"
#include <string.h>

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* sprites;
TTF_Font* font;
SDL_Color textColor = {255, 255, 255, 255};
SDL_Rect levelRect = {0, BAR_HEIGHT, LEVEL_WIDTH, LEVEL_HEIGHT};
SDL_Rect barRect = {0, 0, LEVEL_WIDTH, BAR_HEIGHT};
//#define BAR_ENABLED

enum {
    TEXT_CACHE_SIZE = 8
};

typedef struct
{
    const char* text;
    SDL_Texture* texture;
} TextTexture;

struct {
    TextTexture textures[TEXT_CACHE_SIZE];
    int next;
} textCache;


void initRender()
{
    // Window and renderer
#ifdef BAR_ENABLED
    SDL_CreateWindowAndRenderer(LEVEL_WIDTH, LEVEL_HEIGHT + BAR_HEIGHT, 0, &window, &renderer);
    SDL_RenderSetViewport(renderer, &levelRect);
#else
    SDL_CreateWindowAndRenderer(LEVEL_WIDTH, LEVEL_HEIGHT, 0, &window, &renderer);
#endif

    // Sprites
    static const char* spritesPath = "image/sprites.bmp";
    static const Uint8 transparent[3] = {90, 82, 104};
    SDL_Surface* bmp = SDL_LoadBMP(spritesPath);
    SDL_SetColorKey(bmp, SDL_TRUE, SDL_MapRGB(bmp->format, transparent[0], transparent[1], transparent[2]));
    sprites = SDL_CreateTextureFromSurface(renderer, bmp);
    SDL_FreeSurface(bmp);

    // Font
    TTF_Init();
    font = TTF_OpenFont("font/PressStart2P.ttf", 12);
}

SDL_Texture* createText( const char* text )
{
    if (!text) return NULL;

    // Split text into lines
    char* lines[64];
    int lineCount = 0;
    int textWidth = 0;

    for (int i = 0, s = 0; ; ++ i, ++ s) {
        const char c = text[i];
        if (c == '\n' || c == '\0') {
            char* line = (char*)malloc(s + 1);
            strncpy(line, &text[i - s], s);
            line[s] = 0;
            lines[lineCount ++] = line;
            int w, h;
            TTF_SizeText(font, line, &w, &h);
            if (w > textWidth) {
                textWidth = w;
            }
            s = -1;
            if (c == '\0') {
                break;
            }
        }
    }

    // Create texture
    const int LINE_HEIGHT = TTF_FontHeight(font);
    const int LINE_SPACING = 10;
    const int textHeight = LINE_HEIGHT + (LINE_HEIGHT + LINE_SPACING) * (lineCount - 1);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
            SDL_TEXTUREACCESS_TARGET, textWidth, textHeight);
    SDL_SetRenderTarget(renderer, texture);

    for (int i = 0; i < lineCount; ++ i) {
        int w, h;
        TTF_SizeText(font, lines[i], &w, &h);
        SDL_Rect destRect = {(textWidth - w) / 2, (LINE_HEIGHT + LINE_SPACING) * i, w, h};
        SDL_Surface* lineSurface = TTF_RenderText_Solid(font, lines[i], textColor);
        SDL_Texture* lineTexture = SDL_CreateTextureFromSurface(renderer, lineSurface);
        SDL_RenderCopy(renderer, lineTexture, NULL, &destRect);
        SDL_FreeSurface(lineSurface);
        SDL_free(lineTexture);
        free(lines[i]);
    }
    SDL_SetRenderTarget(renderer, NULL);

    return texture;
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

void drawBox( int x, int y, int w, int h )
{
    const int border = 2;
    const SDL_Rect rect = {x, y, w, h};
    const SDL_Rect borderRect = {x - border, y - border, w + border * 2, h + border * 2};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &borderRect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
}

// Draws text at (x, y) or at the center of rectangle (x, y, w, h) if w and/or h > 0.
// If withBox is 1, draws a box around the text.
void _drawText( SDL_Texture* text, int x, int y, int w, int h, int withBox )
{
    SDL_Rect textRect = {x, y};
    SDL_QueryTexture(text, NULL, NULL, &textRect.w, &textRect.h);
    if (withBox) {
        const int padding = 10;
        const SDL_Rect defaultRect = {x, y, CELL_SIZE * 5, CELL_SIZE * 2.5};
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
// Requirements: If a string pointed by the "text" parameter is changed dynamically,
// its address should be changed as well. The string address is cached, and later any
// string with the same address will be drawn from cache, unchanged.
//
void drawText( const char* text, int x, int y, int w, int h, int withBox )
{
    if (!text) return;
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
        t->texture = createText(text);
        t->text = text;
        texture = t->texture;
        textCache.next = (textCache.next + 1) % TEXT_CACHE_SIZE;
    }
    _drawText(texture, x, y, w, h, withBox);
}

void drawScreen()
{
#ifdef BAR_ENABLED
    // Bar
    SDL_Rect barBorders = {0, 0, LEVEL_WIDTH, BAR_HEIGHT};
    SDL_Rect bar = {-4, 4, LEVEL_WIDTH + 8, BAR_HEIGHT - 8};
    SDL_RenderSetViewport(renderer, &barRect);
    SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
    SDL_RenderFillRect(renderer, &barBorders);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &bar);
    drawText(level->name, 0, 0, LEVEL_WIDTH, BAR_HEIGHT, 0);
    /*
    drawObject(&objectTypes[TYPE_HEART], 0, -2, 0, SDL_FLIP_NONE);
    drawObject(&objectTypes[TYPE_HEART], CELL_HALF, -2, 0, SDL_FLIP_NONE);
    drawObject(&objectTypes[TYPE_HEART], CELL_HALF*2, -2, 0, SDL_FLIP_NONE);
    */
    drawObject(&objectTypes[TYPE_HEART], 0, -1, 0, SDL_FLIP_NONE);
    drawText("3", 28, 0, 0, BAR_HEIGHT + 4, 0);
    const int b = 2;
    SDL_Rect healthBar = {49, CELL_HALF - 5, player.health / 1.5, 10};
    SDL_Rect healthBack = {healthBar.x, healthBar.y, 100 / 1.5, healthBar.h};
    SDL_Rect healthBox = {healthBar.x - b, healthBar.y - b, 100 / 1.5 + b*2, healthBar.h + b*2};
    /*
    SDL_Rect healthBox1 = {50 - 2, CELL_HALF - 5 - 2, 50 + 4, 10 + 4};
    SDL_Rect healthBox2 = {50, CELL_HALF - 5, 50, 10};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &healthBox1);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &healthBox2);
    //*/
    ///*
    SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
    SDL_RenderFillRect(renderer, &healthBox);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &healthBack);
    SDL_SetRenderDrawColor(renderer, 190, 38, 51, 255);
    SDL_RenderFillRect(renderer, &healthBar);
    //*/
    SDL_RenderSetViewport(renderer, &levelRect);
#endif

    // Level
    for (int r = 0; r < ROW_COUNT; ++ r) {
        for (int c = 0; c < COLUMN_COUNT; ++ c) {
            drawObject(level->map[r][c], CELL_SIZE * c, CELL_SIZE * r, 0, SDL_FLIP_NONE);
        }
    }

    // Objects
    for (int i = 0; i < level->objects.count; ++ i) {
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

void drawInventory( int selectionIndex )
{
    const int MAX_ITEMS = 5;
    const ObjectArray* items = &player.items;
    const int count = items->count > MAX_ITEMS ? MAX_ITEMS :
                      items->count > 0 ? items->count : 1;
    const int index = selectionIndex >= MAX_ITEMS ? MAX_ITEMS - 1 : selectionIndex;
    const int hspace = 4;
    const int h = count * (CELL_SIZE + hspace) + (CELL_SIZE - hspace);
    const int w = 8 * CELL_SIZE;
    const SDL_Rect content = {(LEVEL_WIDTH - w) / 2, (LEVEL_HEIGHT - h) / 2, w, h};
    const SDL_Rect selection =
        { content.x + CELL_HALF - 8, content.y + CELL_HALF + (CELL_SIZE + hspace) * index - 2,
          w - CELL_SIZE + 16, CELL_SIZE + 4 };

    drawBox(content.x, content.y, content.w, content.h);

    if (items->count) {
        const int x = content.x + CELL_HALF;
        const int i0 = selectionIndex >= MAX_ITEMS ? selectionIndex - MAX_ITEMS + 1 : 0;
        const int i1 = items->count > MAX_ITEMS ? i0 + MAX_ITEMS : items->count;
        for (int i = i0; i < i1; ++ i) {
            const int y = content.y + CELL_HALF + (CELL_SIZE + hspace) * (i - i0);
            ObjectType* type = items->array[i]->type;
            drawObject(type, x, y, 0, SDL_FLIP_NONE);
            if (type->name) {
                drawText(type->name, x + CELL_SIZE + 10, y, 0, CELL_SIZE, 0);
            }
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &selection);
    } else {
        drawText("No items", content.x + CELL_HALF,
                content.y + CELL_HALF, content.w - CELL_SIZE, CELL_SIZE, 0);
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

