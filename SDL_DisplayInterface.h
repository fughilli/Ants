#ifndef SDL_DISPLAY_INTERFACE_H
#define SDL_DISPLAY_INTERFACE_H

#include "SDL2/SDL.h"

struct G3D_Color
{
    uint8_t r,g,b;
};

class SDL_DisplayInterface
{
protected:
    SDL_Surface ** dispSurface;
public:
    SDL_DisplayInterface(SDL_Surface ** surface);
    void drawPoint(int16_t x, int16_t y, const G3D_Color& color);
    void clear(const G3D_Color& color);
};

#endif // SDL_DISPLAY_INTERFACE_H
