/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#ifndef RENDER_H
#define RENDER_H

#include "types.h"

extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern SDL_Texture* sprites;

void initRender();
void drawSprite( SDL_Rect* spriteRect, int x, int y, int frame, SDL_RendererFlip flip );
void drawObject( ObjectType* type, int x, int y, int frame, SDL_RendererFlip flip );
void drawScreen();
void setAnimation( Object* object, int frameStart, int frameEnd, int frameDelay );

#endif
