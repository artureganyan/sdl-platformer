/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#include "levels.h"

enum {
    LEVEL_YOFFSET = CELL_COUNT * LEVEL_XCOUNT,
    LEVEL_ROWOFFSET = COLUMN_COUNT * LEVEL_XCOUNT
};

Level levels[LEVEL_YCOUNT][LEVEL_XCOUNT];
Level* level = &levels[0][0];
extern const char* levelString;


#define INIT(typeId_, spriteRow, spriteColumn) \
{ \
    ObjectType* type = &objectTypes[typeId_]; \
    type->sprite.y = (spriteRow) * SPRITE_SIZE; \
    type->sprite.x = (spriteColumn) * SPRITE_SIZE; \
}

void initSprites_Castle()
{
    INIT( TYPE_WALL_TOP,        4,  6 );
    INIT( TYPE_WALL,            5,  6 );
    INIT( TYPE_GROUND_TOP,      6,  3 );
    INIT( TYPE_GROUND,          7,  3 );
    INIT( TYPE_GRASS,           40, 0 );
    INIT( TYPE_GRASS_BIG,       40, 1 );
    INIT( TYPE_PILLAR_TOP,      26, 2 );
    INIT( TYPE_PILLAR,          27, 2 );
    INIT( TYPE_PILLAR_BOTTOM,   28, 2 );
    INIT( TYPE_DOOR,            10, 0 );
    INIT( TYPE_LADDER,          12, 2 );
}

void initSprites_Forest()
{
    INIT( TYPE_WALL_TOP,        4,  6 );
    INIT( TYPE_WALL,            5,  6 );
    INIT( TYPE_GROUND_TOP,      6,  1 );
    INIT( TYPE_GROUND,          7,  1 );
    INIT( TYPE_GRASS,           40, 0 );
    INIT( TYPE_GRASS_BIG,       40, 1 );
    INIT( TYPE_PILLAR_TOP,      48, 1 );
    INIT( TYPE_PILLAR,          49, 1 );
    INIT( TYPE_PILLAR_BOTTOM,   50, 1 );
    INIT( TYPE_DOOR,            10, 0 );
    INIT( TYPE_LADDER,          12, 2 );
}

#undef INIT


void initLevel( Level* level )
{
    int r, c;
    for (r = 0; r < ROW_COUNT; ++ r) {
        for (c = 0; c < COLUMN_COUNT; ++ c) {
            level->map[r][c] = &objectTypes[TYPE_NONE];
        }
    }
    level->initSprites = initSprites_Castle;
    level->background = 0x000000;

    initArray(&level->objects);
    appendArray(&level->objects, (Object*)&player);
}

void initLevels()
{
    int r, c, lr, lc;

    for (lr = 0; lr < LEVEL_YCOUNT; ++ lr) {
        for (lc = 0; lc < LEVEL_XCOUNT; ++ lc) {
            const char* string = levelString + LEVEL_YOFFSET * lr + COLUMN_COUNT * lc;
            Level* level = &levels[lr][lc];
            initLevel(level);
            level->r = lr;
            level->c = lc;

            for (r = 0; r < ROW_COUNT; ++ r) {
                for (c = 0; c < COLUMN_COUNT; ++ c) {
                    const char s = string[r * LEVEL_ROWOFFSET + c];
                    if (s == '*' || s == 'x') {
                        const int type = s == '*' ? TYPE_WALL : TYPE_GROUND;
                        const int type_top = s == '*' ? TYPE_WALL_TOP : TYPE_GROUND_TOP;
                        if (r == 0 || string[(r - 1) * LEVEL_ROWOFFSET + c] == '*') {
                            createObjectInMap(level, type, r, c);
                        } else {
                            createObjectInMap(level, type_top, r, c);
                        }
                    } else if (s == '|') {
                        if (r == 0 || string[(r - 1) * LEVEL_ROWOFFSET + c] == '*') {
                            createObjectInMap(level, TYPE_PILLAR_TOP, r, c);
                        } else if (r == ROW_COUNT - 1 || string[(r + 1) * LEVEL_ROWOFFSET + c] == '*') {
                            createObjectInMap(level, TYPE_PILLAR_BOTTOM, r, c);
                        } else {
                            createObjectInMap(level, TYPE_PILLAR, r, c);
                        }
                    } else if (s == ',') {
                        createObjectInMap(level, (c + 1) % 3 ? TYPE_GRASS : TYPE_GRASS_BIG, r, c);
                    } else if (s == ';') {
                        createObjectInMap(level, c % 2 ? TYPE_TREE1 : TYPE_TREE2, r, c);
                    } else if (s == '~') {
                        //createObjectInMap(level, TYPE_CLOUDS1, r, c);
                    } else if (s == '=') {
                        createObjectInMap(level, TYPE_LADDER, r, c);
                    } else if (s == 'd') {
                        createObjectInMap(level, TYPE_DOOR, r, c);
                    } else if (s == 'o') {
                        createObject(level, TYPE_COIN, r, c);
                    } else if (s == 'k') {
                        createObject(level, TYPE_KEY, r, c);
                    } else if (s == 'g') {
                        createObject(level, TYPE_GHOST, r, c);
                    } else if (s == 's') {
                        createObject(level, TYPE_SCORPION, r, c);
                    } else if (s == 'b') {
                        createObject(level, TYPE_BAT, r, c);
                    } else if (s == 'f') {
                        createObject(level, TYPE_FIREBALL, r, c);
                    } else if (s == '`') {
                        createObject(level, TYPE_DROP, r, c);
                    }
                }
            }

        }
    }

    levels[0][0].initSprites = initSprites_Forest;
    levels[0][0].background = 0x000000;
    setLevel(0, 1);
}

void setLevel( int r, int c )
{
    level = &levels[r][c];
    if (level->initSprites) {
        level->initSprites();
    }
}


const char* levelString =

    "         b        * "  "  `                *"  "*                  *"
    "                *   "  "        o s       o*"  "*          o       *"
    "                ****"  "****  ********  =***"  "*         ***      *"
    "                 ***"  "                =  *"  "*       ***     g  *"
    "                 ***"  "             =******"  "*os    *  *  =******"
    "                 ***"  " g o         =     *"  "******    *  =      "
    "                 ***"  "*****     *****=****"  "*     *  **  =      "
    "                 ***"  "               =   *"  "* s     ***  =    o "
    "                 ***"  "   o  o      f =   *"  "******=********* ***"
    "                 ***"  "  **  **  **********"  "*     =     *   o   "
    "                 ***"  "*         | `  |  ` "  "  go  =   g *  ***  "
    "                 ***"  "**        |    |    "  "****  =  ****       "
    "  b         b    ***"  " **       |    |    "  "      =     *       "
    " ,,,,  ,;,k  ,,,,d  "  "    *  s  |o   | s  "  "      = o   *     g "
    "xxxxxxxxxxxxxxxxx***"  "=*******************"  "***************=****"

    "                   *"  "      **    =       "  "    =   =      =    "
    "                   *"  "    **      =       "  "    **  =      =    "
    "                   *"  "  **        =       "  "  **    =      =    "
    "                   *"  "     **=*******     "  "        =      =    "
    "                   *"  "       =            "  "        =      =    "
    "                   *"  "       =            "  "        = **** =    "
    "                   *"  "       =            "  "        =      =    "
    "                   *"  "       =   b        "  "        =      =    "
    "                   *"  "***** ********= ****"  "        =      =    "
    "                   *"  "              =     "  "        =      =    "
    "                   *"  "        g    o=     "  "        =      =    "
    "                   *"  "   ******   **=**   "  "   **************   "
    "                   *"  "              =     "  "                    "
    "                   *"  "**         ** **  **"  "                    "
    "                   *"  "  **                "  "                    ";
