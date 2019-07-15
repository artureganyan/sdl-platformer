TEMPLATE    = app
CONFIG      -= qt
SOURCES     += types.c helpers.c objects.c framecontrol.c game.c levels.c main.c render.c
HEADERS     += types.h helpers.h objects.h framecontrol.h game.h levels.h main.h render.h
LIBS        += -lSDL2 -lSDL2_ttf -lm
INCLUDEPATH += /usr/include/SDL2
DISTFILES   += README.md LICENSE
