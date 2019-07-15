SDL Platformer
--------------

The simple platformer written in C and using SDL. As a C++ programmer, I wanted 
to experiment with pure C and graphics, and a game was perfect for that. But it 
turned out that I had no good idea for the game itself, so this is a working 
model rather than a complete game.

![screenshot](/image/screenshot.png)


Compilation
-----------

It requires SDL 2.0 and SDL_ttf 2.0 libraries. See https://www.libsdl.org and
https://www.libsdl.org/projects/SDL_ttf/ for downloads. On Linux you can install
them as follows:

```
sudo apt-get install libsdl2-dev libsdl2-ttf-dev
```

To compile, do:

```
cd sdl-platformer
make
```

Or you can open sdl_platformer.pro with Qt Creator and compile it there. You may
need to adjust paths in Makefile or *.pro for your system.


Credits
-------

Thanks to the authors and contributors of the following works:

The Simple DirectMedia Layer and SDL_ttf libraries by Sam Lantinga.
Links: https://www.libsdl.org, https://www.libsdl.org/projects/SDL_ttf/

The graphics "Simple broad-purpose tileset" by:

  - surt (Carl Olsson): http://opengameart.org/users/surt 
  - Sharm (Lanea Zimmerman): http://opengameart.org/users/sharm
  - vk: http://opengameart.org/users/vk
  
Link: http://opengameart.org/content/simple-broad-purpose-tileset
  
The font "Press Start 2P" by Cody "CodeMan38" Boisclair.
Link: http://zone38.net/font/#pressstart
