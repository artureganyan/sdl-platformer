/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#include "render.h"


SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* sprites;
TTF_Font* font;
SDL_Color textColor = {255, 255, 255};
SDL_Texture* messageTextures[MESSAGE_COUNT];
SDL_Rect levelRect = {0, BAR_HEIGHT, LEVEL_WIDTH, LEVEL_HEIGHT};
SDL_Rect barRect = {0, 0, LEVEL_WIDTH, BAR_HEIGHT};
//#define BAR_ENABLED


void initRender()
{
    int i;

    // Window and renderer
#ifdef BAR_ENABLED
    SDL_CreateWindowAndRenderer(LEVEL_WIDTH, LEVEL_HEIGHT + BAR_HEIGHT, 0, &window, &renderer);
    SDL_RenderSetViewport(renderer, &levelRect);
#else
    SDL_CreateWindowAndRenderer(LEVEL_WIDTH, LEVEL_HEIGHT, 0, &window, &renderer);
#endif

    // Sprites
    static const char* spritesFilePath = "image/sprites.bmp";
    static const Uint8 transparent[3] = {90, 82, 104};
    SDL_Surface* bmp = SDL_LoadBMP(spritesFilePath);
    SDL_SetColorKey(bmp, SDL_TRUE, SDL_MapRGB(bmp->format, transparent[0], transparent[1], transparent[2]));
    sprites = SDL_CreateTextureFromSurface(renderer, bmp);
    SDL_FreeSurface(bmp);

    // Font
    TTF_Init();
    font = TTF_OpenFont("font/PressStart2P.ttf", 12);

    // Messages
    for (i = 0; i < MESSAGE_COUNT; ++ i) {
        messageTextures[i] = createText(messages[i]);
    }
}

SDL_Texture* createText( const char* text )
{
    if (!text) return NULL;
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, textColor);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
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

// Draws text at (x, y) or at the center of rectangle (x, y, w, h) if w and h > 0
void drawText( SDL_Texture* text, int x, int y, int w, int h )
{
    SDL_Rect rect = {x, y};
    SDL_QueryTexture(text, NULL, NULL, &rect.w, &rect.h);
    if (w > 0) rect.x += (w - rect.w) / 2;
    if (h > 0) rect.y += (h - rect.h) / 2;
    SDL_RenderCopy(renderer, text, NULL, &rect);
}

// The same as drawText() but for specified Message
void drawMessage( Message message, int x, int y, int w, int h, int box )
{
    SDL_Texture* text = messageTextures[message];
    if (box) {
        const int padding = 10;
        const SDL_Rect defaultRect = {x, y, CELL_SIZE * 5, CELL_SIZE * 2.5};
        SDL_Rect rect = {x, y};
        SDL_QueryTexture(text, NULL, NULL, &rect.w, &rect.h);
        if (rect.w > defaultRect.w) {
            rect.w += padding * 2;
        } else {
            rect.w = defaultRect.w;
        }
        if (rect.h > defaultRect.h) {
            rect.h += padding * 2;
        } else {
            rect.h = defaultRect.h;
        }
        if (w > 0) rect.x += (w - rect.w) / 2;
        if (h > 0) rect.y += (h - rect.h) / 2;
        drawBox(rect.x, rect.y, rect.w, rect.h);
    }
    drawText(text, x, y, w, h);
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
    drawText(level->nameTexture, 0, 0, LEVEL_WIDTH, BAR_HEIGHT);
    /*
    drawObject(&objectTypes[TYPE_HEART], 0, -2, 0, SDL_FLIP_NONE);
    drawObject(&objectTypes[TYPE_HEART], CELL_HALF, -2, 0, SDL_FLIP_NONE);
    drawObject(&objectTypes[TYPE_HEART], CELL_HALF*2, -2, 0, SDL_FLIP_NONE);
    */
    drawObject(&objectTypes[TYPE_HEART], 0, -1, 0, SDL_FLIP_NONE);
    drawMessage(MESSAGE_TEST, 28, 0, 0, 32, 0);
    SDL_Rect healthBar = {49, CELL_HALF - 5, player.health / 1.5, 10};
    SDL_Rect healthBox = {49 - 1, CELL_HALF - 5 - 1, 100 / 1.5 + 2, 10 + 2};
    /*
    SDL_Rect healthBox1 = {50 - 2, CELL_HALF - 5 - 2, 50 + 4, 10 + 4};
    SDL_Rect healthBox2 = {50, CELL_HALF - 5, 50, 10};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &healthBox1);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &healthBox2);
    */
    SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
    SDL_RenderDrawRect(renderer, &healthBox);
    SDL_SetRenderDrawColor(renderer, 190, 38, 51, 255);
    SDL_RenderFillRect(renderer, &healthBar);
    SDL_RenderSetViewport(renderer, &levelRect);
#endif

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
        int i;
        for (i = i0; i < i1; ++ i) {
            const int y = content.y + CELL_HALF + (CELL_SIZE + hspace) * (i - i0);
            ObjectType* type = items->array[i]->type;
            drawObject(type, x, y, 0, SDL_FLIP_NONE);
            if (type->name) {
                drawText(type->nameTexture, x + CELL_SIZE + 10, y, 0, CELL_SIZE);
            }
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &selection);
    } else {
        drawMessage(MESSAGE_NOITEMS, content.x + CELL_HALF,
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

