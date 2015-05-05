/******************************************************************************
 * Copyright (c) 2015 Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#include "levels.h"
#include "render.h"

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

void initSprites_Underground()
{
    INIT( TYPE_WALL_TOP,        4,  6 );
    INIT( TYPE_WALL,            5,  6 );
    INIT( TYPE_GROUND_TOP,      6,  2 );
    INIT( TYPE_GROUND,          7,  2 );
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
    level->init = initSprites_Castle;
    level->background = 0x000000;
    level->nameTexture = NULL;
    level->name = "Some screen";

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
                        const char st = string[(r - 1) * LEVEL_ROWOFFSET + c];
                        if (r == 0 || st == '*' || st == 'x') {
                            createObjectInMap(level, type, r, c);
                        } else {
                            createObjectInMap(level, type_top, r, c);
                        }
                    } else if (s == '|') {
                        const char st = string[(r - 1) * LEVEL_ROWOFFSET + c];
                        const char sb = string[(r + 1) * LEVEL_ROWOFFSET + c];
                        if (r == 0 || st == '*' || st == 'x') {
                            createObjectInMap(level, TYPE_PILLAR_TOP, r, c);
                        } else if (r == ROW_COUNT - 1 || sb == '*' || sb == 'x') {
                            createObjectInMap(level, TYPE_PILLAR_BOTTOM, r, c);
                        } else {
                            createObjectInMap(level, TYPE_PILLAR, r, c);
                        }
                    } else if (s == '~') {
                        const char st = string[(r - 1) * LEVEL_ROWOFFSET + c];
                        if (r == 0 || st == '~' || st == 'x' || st == '*') {
                            createObjectInMap(level, TYPE_WATER, r, c);
                        } else {
                            createObjectInMap(level, TYPE_WATER_TOP, r, c);
                        }
                    } else if (s == '^') {
                        const char st = string[(r - 1) * LEVEL_ROWOFFSET + c];
                        if (st == '*' || st == 'x') {
                            createObjectInMap(level, TYPE_SPIKE_TOP, r, c);
                        } else {
                            createObjectInMap(level, TYPE_SPIKE_BOTTOM, r, c);
                        }
                    } else if (s == ',') {
                        createObjectInMap(level, (c + 1) % 3 ? TYPE_GRASS : TYPE_GRASS_BIG, r, c);
                    } else if (s == '.') {
                        createObjectInMap(level, TYPE_MUSHROOM1 + c % 3, r, c);
                    } else if (s == ';') {
                        createObjectInMap(level, c % 2 ? TYPE_TREE1 : TYPE_TREE2, r, c);
                    } else if (s == '@') {
                        createObjectInMap(level, TYPE_ROCK, r, c);
                    } else if (s == '=') {
                        createObjectInMap(level, TYPE_LADDER, r, c);
                    } else if (s == 'd') {
                        createObjectInMap(level, TYPE_DOOR, r, c);
                    } else if (s == 'o') {
                        createObject(level, TYPE_COIN, r, c);
                    } else if (s == 'k') {
                        createObject(level, TYPE_KEY, r, c);
                    } else if (s == 'a') {
                        createObject(level, TYPE_APPLE, r, c);
                    } else if (s == 'i') {
                        createObject(level, TYPE_PEAR, r, c);
                    } else if (s == 'g') {
                        createObject(level, TYPE_GHOST, r, c);
                    } else if (s == 's') {
                        createObject(level, TYPE_SCORPION, r, c);
                    } else if (s == 'p') {
                        createObject(level, TYPE_SPIDER, r, c);
                    } else if (s == 'r') {
                        createObject(level, TYPE_RAT, r, c);
                    } else if (s == 'b') {
                        createObject(level, TYPE_BAT, r, c);
                    } else if (s == 'q') {
                        createObject(level, TYPE_BLOB, r, c);
                    } else if (s == 'f') {
                        createObject(level, TYPE_FIREBALL, r, c);
                    } else if (s == 'e') {
                        createObject(level, TYPE_SKELETON, r, c);
                    } else if (s == '`') {
                        createObject(level, TYPE_DROP, r, c);
                    } else if (s == '_') {
                        createObject(level, TYPE_PLATFORM, r, c);
                    } else if (s == '&') {
                        createObject(level, TYPE_CLOUD1, r, c);
                    }
                }
            }

        }
    }

    createObject(&levels[0][0], TYPE_LADDER_PART, 7, 8);
    createObject(&levels[0][0], TYPE_PICK, 7, 12);
    createObject(&levels[1][2], TYPE_PLATFORM, 13, 3);
    //createObject(&levels[0][0], TYPE_PLATFORM, 7, 5)->vx = 0;

    levels[0][0].init = initSprites_Forest;
    levels[0][1].init = initSprites_Forest;
    levels[0][2].init = initSprites_Forest;
    levels[1][0].init = initSprites_Underground;
    levels[1][1].init = initSprites_Underground;
    levels[1][2].init = initSprites_Underground;
    levels[1][3].init = initSprites_Underground;
    levels[1][4].init = initSprites_Underground;

    levels[0][0].name = "Forest";
    levels[1][0].name = "Cave";
    levels[1][1].name = "Hidden Entrance";

    for (lr = 0; lr < LEVEL_YCOUNT; ++ lr) {
        for (lc = 0; lc < LEVEL_XCOUNT; ++ lc) {
            Level* level = &levels[lr][lc];
            reorderDepth(&level->objects);
            level->nameTexture = createText(level->name);
        }
    }

    setLevel(0, 0);
    player.x = CELL_SIZE * 1;
    player.y = CELL_SIZE * 1;
}

void setLevel( int r, int c )
{
    //int count = 48 + rand() % 24;

    level = &levels[r][c];
    if (level->init) {
        level->init();
    }

    // To change position of objects we simply play few frames
    // \todo This can kill player!
    /*
    while (count --) {
        processObjects();
    }
    */
}


const char* levelString =

"    &           &   "  "                    "  "                  * "  "  `                *"  "*                  *"
"          &         "  "                    "  " _              *   "  "        o s       o*"  "*          o       *"
"xxxxx               "  "                    "  " k  k  _        ****"  "****  ********  =***"  "*         ***      *"
"xxx                 "  "                    "  "kakiaik          ***"  "                =  *"  "*       ***     g  *"
"xx                  "  "                    "  "*******  ******  ***"  "             =******"  "*os    *  *  =******"
"x                   "  "                    "  "                 ***"  " g o         =     *"  "******    *  =      "
"x                   "  "    b               "  "        &        ***"  "*****     *****=****"  "*     *  **  =      "
"xxxxx  ,,;,, a ,;,, "  ",,,                 "  "                 ***"  "               =   *"  "* s     ***  =    o "
"xxxxxxxxxxxxxxxxxxxx"  "xxxx                "  "   _          _  ***"  "   o  o      f =   *"  "******=********* ***"
"xxxxxxxxxxxxxxxxxxxx"  "xxxxxxx   ,,;,      "  "                 ***"  "  **  ** ***********"  "*     =     *   o   "
"  xxx ` x  ` x  ` xx"  "xxxxx    xxxxxx     "  "                 ***"  "*        |  ` |   `|"  "  go  =   g *  ***  "
"   ^    ^    ^    ^ "  " |      xxxxxxxxx   "  "                 ***"  "**       |    |    |"  "****  =  ****       "
"                    "  " |        |    xxx  "  "     b      b    ***"  " **      |    |    |"  "      =     *       "
"  .    s  .         "  " |. ,,,,s | . xxxxxx"  "x ,,, d,;,k d,,,,d d"  "    *  s | o  |  s |"  "      = o   *     g "
"xxxx~~xxxxxxxxxxxxxx"  "xxxxxxxxxxxxx=xxxxxx"  "xxxxxxxxxxxxxxxxx***"  "=*******************"  "***************=****"

"xxxx  xxxxxxxxxxxxxx"  "xxxxxxxxxxxxx=xxxxxx"  "xxxxxxxxxxxxxxxxxx**"  "********************"  "               =    "
"xxx   `xxxxxxxxxxxxx"  "xxxxxxxxxxxx = xxxxx"  "xxxxxxxxxxxxxxxxxxxx"  "**         * ooooo *"  "   b           =    "
"x ^      xxx `  xxxx"  "xxx  ^  xxxx =  xxxx"  "xx   ^   b    ^   xx"  "**         d ooooo *"  "               =    "
"x               `  x"  "x        |   =  ^  x"  "x                 xx"  "**     =************"  "          b    =    "
"x       xxxxxx      "  "         | p =     |"  "      xxxxxxx     xx"  "** o   =    g       "  "               =    "
"x      xxxxxxxx   xx"  "xxxx  xxxxxxxxxxxxxx"  "xxxx   xxxxx      xx"  "*****  =   *****    "  "               =    "
"x   xxxxxxxxxxx  xxx"  "x ^    xxxxxxxxxxxxx"  "xxx               x*"  "**     =            "  "               =    "
"xx  `xxxxxxxxx      "  "   b    |     | xxxx"  "xx            r     "  "       =    s       "  "                    "
"xxx   xxxxxxxxxxxxxx"  "x       |     |     "  " | q         xxxxx**"  "**************=*****"  "*******      *******"
"xxxx  xxxxxxxxxxxxxx"  "x    xxxxxxxxxxxxxxx"  "xxxxxx  x      xxxx*"  "**   b        =   **"  "***         *    ***"
"xxxx  xxxxxxxxxxxxxx"  "x @    xxxxxxxxxxxxx"  "xxxx             xxx"  "x*  o   o     =   **"  "**       x        **"
"xxxx  xxxxxxxxxxxxxx"  "xxxxx     | b       "  " |                xx"  "x***********      xx"  "xx       |        xx"
"xxx    xxxxxxxxxxxxx"  "x  xxx    |         "  "e|                xx"  "xx                xx"  "xxx      |       xxx"
"          .         "  "  k xxx~~xxx~~xx~~xx"  "xxx~~~~~~~~~~~~~~~xx"  "xx~~~~~~~~~~~~~~~~xx"  "xxxx~~~~~x~~~~~~xxxx"
"xxxxxxxxxxxxxxxxxxxx"  "xxxxxxx~~xxx~~xx~~xx"  "xxxx~~~~~~~~~~~~~~xx"  "xx~~~~~~~~~~~~~~~~xx"  "xxxx~~**~x~*~~*~xxxx";
