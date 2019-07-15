TARGET=sdl_platformer
SOURCES=$(wildcard *.c)
HEADERS=$(wildcard *.h)
SDL=-I/usr/include/SDL2 -lSDL2 -lSDL2_ttf
MATH=-lm

all: $(SOURCES) $(HEADERS)
	cc $(SOURCES) $(SDL) $(MATH) -o $(TARGET)

clean:
	rm -f $(TARGET)

