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


void initLevel( Level* level )
{
	int r, c;
    for (r = 0; r < ROW_COUNT; ++ r) {
        for (c = 0; c < COLUMN_COUNT; ++ c) {
        	level->map[r][c] = &objectTypes[TYPE_NONE];
        }
    }

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
                    if (s == '*') {
                    	if (r == 0 || string[(r - 1) * LEVEL_ROWOFFSET + c] == '*') {
                    		createObjectInMap(level, TYPE_WALL, r, c);
                    	} else {
                    		createObjectInMap(level, TYPE_WALL_TOP, r, c);
                    	}
                    } else if (s == '|') {
						if (r == 0 || string[(r - 1) * LEVEL_ROWOFFSET + c] == '*') {
							createObjectInMap(level, TYPE_PILLAR_TOP, r, c);
						} else if (r == ROW_COUNT - 1 || string[(r + 1) * LEVEL_ROWOFFSET + c] == '*') {
							createObjectInMap(level, TYPE_PILLAR_BOTTOM, r, c);
						} else {
							createObjectInMap(level, TYPE_PILLAR, r, c);
						}
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
                    } else if (s == '@') {
                        createObject(level, TYPE_DROP, r, c);
                    }
                }
            }

        }
    }

    level = &levels[0][1];
}


const char* levelString =

	"                 *  "  "  @                *"  "*                  *"
	"               *    "  "        o s       o*"  "*          o       *"
	"               *****"  "**=*  ********  =***"  "*         ***      *"
	"                 ***"  "  =             =  *"  "*       ***     g  *"
	"                 ***"  "  =          =******"  "*os    *  *  =******"
	"                 ***"  " g o         =     *"  "******    *  =      "
	"                 ***"  "*****     *****=****"  "*     *  **  =      "
	"                 ***"  "     b         =   *"  "* s     ***  =    o "
	"                 ***"  "   o  o      f =   *"  "******=********* ***"
	"                 ***"  "  **  **  **********"  "*     =     *   o   "
	"                 ***"  "*         | @  |  @ "  "  go  =   g *  ***  "
	"                 ***"  "**        |    |    "  "****  =  ****       "
	"                 ***"  " **       |    |    "  "      =     *       "
	"                 ***"  "  **   s  |o   | s  "  "      = o   *     g "
	"********************"  "********************"  "***************=****"

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

