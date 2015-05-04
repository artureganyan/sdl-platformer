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

void drawText( SDL_Texture* text, int x, int y, int w, int h )
{
    SDL_Rect rect = {x, y};
    SDL_QueryTexture(text, NULL, NULL, &rect.w, &rect.h);
    if (w > 0) rect.x += (w - rect.w) / 2;
    if (h > 0) rect.y += (h - rect.h) / 2;
    SDL_RenderCopy(renderer, text, NULL, &rect);
}

void drawMessage( Message message, int x, int y, int w, int h )
{
    drawText(messageTextures[message], x, y, w, h);
}

void drawScreen()
{
#ifdef BAR_ENABLED
    SDL_Rect barBorders = {0, 0, LEVEL_WIDTH, BAR_HEIGHT};
    SDL_Rect bar = {-4, 4, LEVEL_WIDTH + 8, BAR_HEIGHT - 8};
    SDL_RenderSetViewport(renderer, &barRect);
    SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
    SDL_RenderFillRect(renderer, &barBorders);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &bar);
    drawText(level->nameTexture, 0, 0, LEVEL_WIDTH, BAR_HEIGHT);
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
    const int x = (LEVEL_WIDTH - w) / 2;
    const int y = (LEVEL_HEIGHT - h) / 2;
    const SDL_Rect content = {x, y, w, h};
    const SDL_Rect border = {x - 2, y - 2, w + 4, h + 4};
    const SDL_Rect selection =
        { x + CELL_HALF - 8, y + CELL_HALF + (CELL_SIZE + hspace) * index - 2,
          w - CELL_SIZE + 16, CELL_SIZE + 4};
    int i;

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &border);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &content);

    if (items->count) {
        const int i0 = selectionIndex >= MAX_ITEMS ? selectionIndex - MAX_ITEMS + 1 : 0;
        const int i1 = items->count > MAX_ITEMS ? i0 + MAX_ITEMS : items->count;
        const int x = content.x + CELL_HALF;
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
                content.y + CELL_HALF, content.w - CELL_SIZE, CELL_SIZE);
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

