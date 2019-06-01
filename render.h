/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#ifndef RENDER_H
#define RENDER_H

#include "types.h"

extern SDL_Renderer* renderer;

void initRender();
void drawSprite( SDL_Rect* spriteRect, int x, int y, int frame, SDL_RendererFlip flip );
void drawObject( Object* object, int x, int y );
void drawTextEx( const char* text, int x, int y, int w, int h, int withBox );
void drawText( const char* text );
void drawScreen();
void setAnimation( Object* object, int frameStart, int frameEnd, int frameDelay );

#endif
