/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#ifndef RENDER_H
#define RENDER_H

#include "types.h"
#include "SDL_ttf.h"

extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern SDL_Texture* sprites;
extern TTF_Font* font;

void initRender();
SDL_Texture* createText( const char* text );
void drawSprite( SDL_Rect* spriteRect, int x, int y, int frame, SDL_RendererFlip flip );
void drawObject( ObjectType* type, int x, int y, int frame, SDL_RendererFlip flip );
void drawText( SDL_Texture* text, int x, int y, int w, int h );
void drawMessage( Message message, int x, int y, int w, int h, int box );
void drawScreen();
void drawInventory( int selectionIndex );
void setAnimation( Object* object, int frameStart, int frameEnd, int frameDelay );

#endif
