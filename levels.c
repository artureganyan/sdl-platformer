/******************************************************************************
 * Copyright (c) Artur Eganyan
 *
 * This software is provided "AS IS", WITHOUT ANY WARRANTY, express or implied.
 ******************************************************************************/

#include "levels.h"
#include "render.h"
#include "game.h"
#include "helpers.h"

Level levels[LEVEL_COUNTY][LEVEL_COUNTX];
static const char* levelsString;


static void changeSprite( ObjectTypeId typeId, int spriteRow, int spriteColumn )
{
    ObjectType* type = &objectTypes[typeId];
    type->sprite.y = spriteRow * SPRITE_SIZE;
    type->sprite.x = spriteColumn * SPRITE_SIZE;
}

static void changeSprites_Castle()
{
    changeSprite( TYPE_WALL_TOP,        4,  6  );
    changeSprite( TYPE_WALL,            5,  6  );
    changeSprite( TYPE_WALL_FAKE,       5,  6  );
    changeSprite( TYPE_WALL_STAIR,      4,  6  );
    changeSprite( TYPE_GROUND_TOP,      6,  3  );
    changeSprite( TYPE_GROUND,          7,  3  );
    changeSprite( TYPE_GROUND_FAKE,     7,  3  );
    changeSprite( TYPE_GROUND_STAIR,    6,  3  );
    changeSprite( TYPE_GRASS,           40, 0  );
    changeSprite( TYPE_GRASS_BIG,       40, 1  );
    changeSprite( TYPE_PILLAR_TOP,      26, 2  );
    changeSprite( TYPE_PILLAR,          27, 2  );
    changeSprite( TYPE_PILLAR_BOTTOM,   28, 2  );
    changeSprite( TYPE_DOOR,            10, 0  );
    changeSprite( TYPE_LADDER,          12, 2  );
}

static void changeSprites_Forest()
{
    changeSprite( TYPE_WALL_TOP,        4,  6  );
    changeSprite( TYPE_WALL,            5,  6  );
    changeSprite( TYPE_WALL_STAIR,      4,  6  );
    changeSprite( TYPE_GROUND_TOP,      6,  1  );
    changeSprite( TYPE_GROUND,          7,  1  );
    changeSprite( TYPE_GROUND_STAIR,    6,  1  );
    changeSprite( TYPE_GRASS,           40, 0  );
    changeSprite( TYPE_GRASS_BIG,       40, 1  );
    changeSprite( TYPE_PILLAR_TOP,      48, 1  );
    changeSprite( TYPE_PILLAR,          49, 1  );
    changeSprite( TYPE_PILLAR_BOTTOM,   50, 1  );
    changeSprite( TYPE_DOOR,            10, 0  );
    changeSprite( TYPE_LADDER,          12, 2  );
}

static void changeSprites_Underground()
{
    changeSprite( TYPE_WALL_TOP,        4,  6  );
    changeSprite( TYPE_WALL,            5,  6  );
    changeSprite( TYPE_WALL_STAIR,      4,  6  );
    changeSprite( TYPE_GROUND_TOP,      6,  2  );
    changeSprite( TYPE_GROUND,          7,  2  );
    changeSprite( TYPE_GROUND_STAIR,    6,  2  );
    changeSprite( TYPE_GRASS,           40, 0  );
    changeSprite( TYPE_GRASS_BIG,       40, 1  );
    changeSprite( TYPE_PILLAR_TOP,      48, 1  );
    changeSprite( TYPE_PILLAR,          49, 1  );
    changeSprite( TYPE_PILLAR_BOTTOM,   50, 1  );
    changeSprite( TYPE_DOOR,            10, 0  );
    changeSprite( TYPE_LADDER,          12, 2  );
}

static inline const char* getLevelString( const char* allLevels, int r, int c )
{
    return allLevels + r * CELL_COUNT * LEVEL_COUNTX + c * COLUMN_COUNT;
}

static inline const char getLevelChar( const char* levelString, int r, int c )
{
    return levelString[r * COLUMN_COUNT * LEVEL_COUNTX + c];
}

static void initLevelsFromString( const char* string )
{
    struct { int r, c; } startLevel;

    // Iterate over the levels
    for (int lr = 0; lr < LEVEL_COUNTY; ++ lr) {
        for (int lc = 0; lc < LEVEL_COUNTX; ++ lc) {
            const char* levelString = getLevelString(string, lr, lc);

            Level* level = &levels[lr][lc];
            initLevel(level);
            level->r = lr;
            level->c = lc;
            ObjectArray_append(&level->objects, (Object*)&player);

            // Iterate over the level cells and create objects
            for (int r = 0; r < ROW_COUNT; ++ r) {
                for (int c = 0; c < COLUMN_COUNT; ++ c) {
                    const char s = getLevelChar(levelString, r, c);

                    // Wall and ground
                    if (s == '*' || s == 'x') {
                        const ObjectTypeId type = s == '*' ? TYPE_WALL : TYPE_GROUND;
                        const ObjectTypeId type_top = s == '*' ? TYPE_WALL_TOP : TYPE_GROUND_TOP;
                        const char st = getLevelChar(levelString, r - 1, c);
                        if (r == 0 || st == '*' || st == 'x') {
                            createStaticObject(level, type, r, c);
                        } else {
                            createStaticObject(level, type_top, r, c);
                        }
                    // Water
                    } else if (s == '~') {
                        const char st = getLevelChar(levelString, r - 1, c);
                        if (r == 0 || st == '~' || st == 'x' || st == '*') {
                            createStaticObject(level, TYPE_WATER, r, c);
                        } else {
                            createObject(level, TYPE_WATER_TOP, r, c);
                        }
                    // Pillar
                    } else if (s == '|') {
                        const char st = getLevelChar(levelString, r - 1, c);
                        const char sb = getLevelChar(levelString, r + 1, c);
                        if (r == 0 || st == '*' || st == 'x') {
                            createStaticObject(level, TYPE_PILLAR_TOP, r, c);
                        } else if (r == ROW_COUNT - 1 || sb == '*' || sb == 'x') {
                            createStaticObject(level, TYPE_PILLAR_BOTTOM, r, c);
                        } else {
                            createStaticObject(level, TYPE_PILLAR, r, c);
                        }
                    // Spike
                    } else if (s == '^') {
                        const char st = getLevelChar(levelString, r - 1, c);
                        if (st == '*' || st == 'x') {
                            createStaticObject(level, TYPE_SPIKE_TOP, r, c);
                        } else {
                            createStaticObject(level, TYPE_SPIKE_BOTTOM, r, c);
                        }
                    // Other objects
                    } else if (s == '-') {
                        createStaticObject(level, TYPE_WALL_STAIR, r, c);
                    } else if (s == ',') {
                        createStaticObject(level, (c + 1) % 3 ? TYPE_GRASS : TYPE_GRASS_BIG, r, c);
                    } else if (s == '.') {
                        createStaticObject(level, TYPE_MUSHROOM1 + c % 3, r, c);
                    } else if (s == ';') {
                        createStaticObject(level, c % 2 ? TYPE_TREE1 : TYPE_TREE2, r, c);
                    } else if (s == '@') {
                        createStaticObject(level, TYPE_ROCK, r, c);
                    } else if (s == '=') {
                        createStaticObject(level, TYPE_LADDER, r, c);
                    } else if (s == 'd') {
                        createStaticObject(level, TYPE_DOOR, r, c);
                    } else if (s == 'o') {
                        createObject(level, TYPE_COIN, r, c);
                    } else if (s == 'O') {
                        createObject(level, TYPE_GEM, r, c);
                    } else if (s == 'k') {
                        createObject(level, TYPE_KEY, r, c);
                    } else if (s == 'h') {
                        createObject(level, TYPE_HEART, r, c);
                    } else if (s == 'a') {
                        createObject(level, TYPE_APPLE, r, c);
                    } else if (s == 'i') {
                        createObject(level, TYPE_PEAR, r, c);
                    } else if (s == 'S') {
                        createObject(level, TYPE_STATUARY, r, c);
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
                        Object* drop = createObject(level, TYPE_DROP, r, c);
                        drop->y = (drop->y / CELL_SIZE) * CELL_SIZE - (CELL_SIZE - drop->type->body.h) / 2 - 1;
                    } else if (s == '_') {
                        createObject(level, TYPE_PLATFORM, r, c);
                    } else if (s == '/') {
                        createObject(level, TYPE_SPRING, r, c);
                    } else if (s == '<') {
                        createStaticObject(level, TYPE_ARROW_LEFT, r, c);
                    } else if (s == '>') {
                        createStaticObject(level, TYPE_ARROW_RIGHT, r, c);
                    } else if (s == '&') {
                        createObject(level, TYPE_CLOUD1, r, c);
                    } else if (s == '!') {
                        createObject(level, TYPE_TORCH, r, c);
                    } else if (s >= '1' && s <= '9') {
                        Object* action = createObject(level, TYPE_ACTION, r, c);
                        action->data = s;
                    // Start position
                    } else if (s == 'P') {
                        startLevel.r = lr;
                        startLevel.c = lc;
                        player.y = CELL_SIZE * r;
                        player.x = CELL_SIZE * c;
                    }
                }
            }

            ObjectArray_sortByDepth(&level->objects);
        }
    }

    // Set start level
    setLevel(startLevel.r, startLevel.c);
}

void initLevels()
{
    ensure(strlen(levelsString) == LEVEL_COUNTY * LEVEL_COUNTX * ROW_COUNT * COLUMN_COUNT,
           "The levels string does not match the levels count or size.");

    initLevelsFromString(levelsString);

    for (int r = 0; r < LEVEL_COUNTY; r++) {
        for (int c = 0; c < LEVEL_COUNTX; c++) {
            levels[r][c].init = changeSprites_Underground;
        }
    }

    // Special objects can be created here
}


// There must be exactly LEVEL_COUNTX * LEVEL_COUNTY levels here

static const char* levelsString =

"                    "  " *     b       b    "
"  ooooooo S ooooooo "  " d                  "
"=*******************"  "**** _       ****=**"
"=                   "  "*                =  "
"=    o    o    o    "  "*                =  "
"=                   "  "*    ooo    p    =  "
"***     _        **="  "*oo ----  **********"
"                   ="  "*-- -               "
"    f  o     o     ="  "*   -  p  ooo       "
"              f    ="  "*     --=-----  o  O"
"=**       _      ***"  "*       =       -  -"
"=                   "  "*   b   =           "
"=    o    o    o    "  "*       =           "
"=      s            "  "*       =   g       "
"****************  -*"  "*=***************   "

"*           *   o -*"  " =                O "
"*           *   - o*"  " =               ---"
"*    o o o  *   o -*"  " =          oo      "
"*  k    e   >   - o*"  " =  g oooo       goo"
"*******=*****     -*"  " **********    *****"
"*      =    *-  -  *"  "                    "
"* h    =    *      *"  "   o o o    /       "
"*    -----  *      *"  "          ------    "
"*         - * o e o*"  "               -  ks"
"*          **=******"  " oo  o / o     -----"
"*-          *=      "  " ***=*****          "
"*-  o o/ og *=      "  "    =               "
"*- **********=      "  "    =           o   "
"*-   ooo    <=      "  " P  = ooooo  s **~~~"
"********************"  "*****************~~~";

// Experiments

/*
static const char* levelsString =

"    &           &   "  "                    "  "                  * "  "  `                *"  "*                  *"
" P        &         "  "                    "  " _              *   "  "        o s       o*"  "*          o       *"
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
"xxxx  xxxxxxxxxxxxxx"  "x    xxxxxxxxxxxxxxx"  "xxxxxx  x      xxxx*"  "**   b        =   **"  "***         -    ***"
"xxxx  xxxxxxxxxxxxxx"  "x @    xxxxxxxxxxxxx"  "xxxx             xxx"  "x*  o   o     =   **"  "**       x        **"
"xxxx  xxxxxxxxxxxxxx"  "xxxxx     | b       "  " |                xx"  "x***********      xx"  "xx       |        xx"
"xxx    xxxxxxxxxxxxx"  "x  xxx    |         "  "e|                xx"  "xx                xx"  "xxx      |      .xxx"
"          .         "  "  k xxx~~xxx~~xx~~xx"  "xxx~~~~~~~~~~~~~~~xx"  "xx~~~~~~~~~~~~~~~~xx"  "xxxx~~~~~x~~~~~~xxxx"
"xxxxxxxxxxxxxxxxxxxx"  "xxxxxxx~~xxx~~xx~~xx"  "xxxx~~~~~~~~~~~~~~xx"  "xx~~~~~~~~~~~~~~~~xx"  "xxxx~~~~~x~~~~~~xxxx";
//*/

/*
static const char* levelsString =

//         0                       1                       2                       3                       4                      5
"                    "  "         &          "  "                    "  "                    "  "                    " "                    "
"                    "  "o  ooo            & "  "                    "  "                    "  "                    " "                    "
"                    "  "------   oo  xxx    "  "                    "  "                    "  "                    " "                    "
"                    "  " f  -  -xxxxx       "  "                    "  "                    "  "                    " "                    "
"                    "  "    ---             "  "                    "  "                    "  "                    " "                    "
"                    "  "   o-o     g   o    "  "                    "  "                    "  "                    " "                    "
"                    "  "------    xxx xxx  -"  "                    "  "                    "  "                    " "                    " // 0
"                    "  "    -  ---         -"  "                    "  "                    "  "                    " "                    "
"                    "  "    ----       o   -"  "                    "  "                    "  "                    " "                    "
"                    "  "  -----   --  xxxx -"  "   o  o  o          "  "                    "  "                    " "                    "
"                    "  "x    g             -"  "   -  -  -   oo     "  "                    "  "                    " "                    "
"                    "  " xxxxxxxxxx        -"  "      -     ---  oo "  "                    "  "                    " "                    "
"                    "  " ,,   ,   ,,xx,;,P -"  "      -          -- "  "                    "  "                    " "                    "
"                    "  "xxxxxxxxxxxxxxxxxxxx"  "   xxxxxxxxxxxxx    "  "                    "  "                    " "                    "
"                    "  "xxxxxxxxxxxxxxxxxxxx"  "                    "  "                    "  "                    " "***********=********"

"                    "  "                    "  "              ******"  "********************"  "********************" "***********=********"
"     &      &       "  "     &              "  "   &          ******"  "**                **"  "**               ***" "***********=********"
"                    "  "               &    "  "        &           "  "                    "  "                    " "           =        "
"        &           "  "                    "  "              *     "  "                    "  "             g      " "           =        "
"                    "  "                    "  "              ******"  "********************"  "**********=*********" "********************"
"                    "  "                    "  "              ******"  "**               |  "  "          =         " "                    "
"                    "  "                    "  "              ******"  "**               |  "  "    !     =    !    " "                    " // 1
"                    "  " ,;                 "  "              ******"  "**               |  "  "    g     =         " "                    "
"xxxxxxxxxxxxxxxxxxxx"  "xxxxxxxxx           "  "              ******"  "**              ****"  "********************" "***********         "
"xxxxxxxxxxxxxxxxxxxx"  "xxxxxxxx            "  "              ******"  "**             *****"  "********************" "************        "
"xxxxxxxxxxxxxxxxxxxx"  "xxxxxxx             "  "              ******"  "**            ******"  "***      **  `   `  " "          ***       "
"xxxxxxxxxxxxxxxxxxxx"  "xxx  |              "  "              ***   "  "      !      *******"  "**       *          " "     !     ***      "
"xxxxxxxxxxxxxxxxxxxx"  "x    |    @ ,,,     "  "     ,;,      d     "  "            ********"  "***1*   2d          " "                    "
"xxxxxxxxxxxxxxxxxxxx"  "x    xxxxxxxxxxxxxxx"  "xxxxxxxxxxxxxx******"  "********************"  "**** ***************" "********************"
"xxxxxxxxxxxxxxxxxxxx"  "x  xxxxxxxxxxxxxxxxx"  "xxxxxxxxxxxxxx******"  "********************"  "**** ***************" "********************"

"xxxxxxxxxxxxxxxxxxxx"  "xx  xxxxxxxxxxxxxxxx"  "xxxxxxxxxxxxxx******"  "********************"  "**** ***************" "********************"
"xxxxxxxxxxxxxxxxxxxx"  "xxx      xx  xx xxxx"  "xxxxxxxxxxxxxxxxxxxx"  "xxxxxxxxxxxxxxxxxxxx"  "xxxx xxxxxxxxxxxxxxx" "xxxxxxxxxxxxxxxxxxxx"
"xxxxxxxxxxxxxxxxxxxx"  "xxxxxxx      |      "  "      |        |    "  "   x  x `  xxxxxxxxx"  "xx    xxxxxxxxxxxxxx" "xxxxxxxxxxxxxxxxxxxx"
"xxxxxxxxxxxxxxxxxxxx"  "xxxxxxxxx .  |    xx"  "x    xxx       |  xx"  "x  `      xxxxx` xxx"  "x       xxxx   x  xx" "xxxxxxxxxxxxxxxxxxxx"
"xxxxxxxxxxxxxxxxxxxx"  "xxxxxxxxxxxxxxxxxxxx"  "xxx xxxxxx   xxxxxxx"  "xx   b       ^   xxx"  "xxx      ^  b  ^   x" "x    xxxxxxxxxxxxxxx"
"xxxxxxxxxxxxxxxxxxxx"  "xxxx  xxxxxxxxxxxxxx"  "xxxxxxxxxxxxxxxxxxxx"  "xxxxxx xxxx     xxxx"  "xxxxxx           .  " "       xxxxxxx  xxxx"
"xxxxxxxxxxxxxxxxxxxx"  "xxx    xxxxxxxxxxxxx"  "xxxxxxxxxxxxxxxxxxxx"  "xxxxxxxxxxxxx    xxx"  "xxxxxxxxxxxxx  xxxxx" "x    xxxxxxxx    xxx"
"xxxxxxxxxxxxxxxxxxxx"  "xx    xxxxxxxxxxxxxx"  "xxxxxxxxxxxxxxxxxxxx"  "xxxxxxxxxxxxxxx   xx"  "xxxxxxxxxxx     xxxx" "xxx xxxxxxxx        " // 2
"xxxxxxxxxxxxxxxxxxxx"  "xxxxxxxxxxxxxxxxxxxx"  "xxxxxxxxxxxxxxxxxxxx"  "xxxxxxxxx  |    xxxx"  "xxx   x  |   x    xx" "xxxxxxxxxxxxxx    xx"
"xxxxxxxxxxxxxxx xxxx"  "xxxxxxxxxx  xxxxxxxx"  "xxx xxxxxxxxxxx  xxx"  "xxxxxxxx   |   xxxxx"  "xxxx     |  xxx   xx" "xxxxxxxxxxxxxxxx xxx"
"xxxxxxxxxxx ^    xxx"  "xxx  xx      xx  xx "  "xx   xx   xx  ^   xx"  "xxx ^ x   xxxx  xxxx"  "x        xxxxxxxxxxx" "xxxxxxxxxxx` xxxxxxx"
"xxxxxxxxxx          "  " ^    ^  xx         "  "     ^             x"  "x        xxxxxx     "  "   xxxxxxxxxxxxxxxxx" "xxxxxxxxx     xxxxxx"
"xxxxxxxxxxxxxxxx    "  "   r    xxxx        "  "        xxxxx     xx"  "xx    . xxxxxxxx xxx"  "xx  xxxxxxxxxxxxxxx " " ^ xxxxx   xxxxxxxxx"
"xxxxxxxxxxxxxxxxxxxx"  "xxxxxxxxxxxxx~~xx~~x"  "~~xx~~xxxxxxxx . xxx"  "xxx  xxxxxxxxxxxxxxx"  "xx~~xxxxxxxxxxxxxx  " "      .  xxxxxxxxxxx"
"xxxxxxxxxxxxxxxxxxxx"  "xxxxxxxxxxxx~~~~~~~~"  "~~~~~~xxxxxxxxxxxxxx"  "xxxxxxxxxxxxxxxxxxxx"  "xx~~xxxxxxxxxxxxxxxx" "x xx xxxxxxxxxxxxxxx";
//*/
