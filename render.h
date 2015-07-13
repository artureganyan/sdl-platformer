/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#ifndef RENDER_H
#define RENDER_H

#include "types.h"
#include "SDL_ttf.h"

extern SDL_Renderer* renderer;

void initRender();
void drawSprite( SDL_Rect* spriteRect, int x, int y, int frame, SDL_RendererFlip flip );
void drawObject( ObjectType* type, int x, int y, int frame, SDL_RendererFlip flip );
void drawText( const char* text, int x, int y, int w, int h, int withBox );
void drawScreen();
void drawInventory( int selectionIndex );
void setAnimation( Object* object, int frameStart, int frameEnd, int frameDelay );

#endif
