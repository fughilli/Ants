#include "SDL_DisplayInterface.h"
#include "SDL2/SDL.h"

static uint32_t G3D_ColorTo32BitColor(const G3D_Color& color)
{
    uint32_t ret = color.r;
    ret <<= 8;
    ret |= color.g;
    ret <<= 8;
    ret |= color.b;
    ret <<= 0;
    return ret;
}

SDL_DisplayInterface::SDL_DisplayInterface(SDL_Surface ** surface)
{
    dispSurface = surface;
}

void SDL_DisplayInterface::drawPoint(int16_t x, int16_t y, const G3D_Color& color)
{
    if(x >= 0 && x < (*dispSurface)->w && y >= 0 && y < (*dispSurface)->h)
    {
        uint32_t index = (*dispSurface)->w * y + x;
        ((uint32_t*)(*dispSurface)->pixels)[index] = G3D_ColorTo32BitColor(color);
    }
}

void SDL_DisplayInterface::clear(const G3D_Color& color)
{
    uint32_t clearColor = G3D_ColorTo32BitColor(color);

    for(int32_t i = 0; i < (((*dispSurface)->w) * ((*dispSurface)->h)); i++)
    {
        ((uint32_t*)(*dispSurface)->pixels)[i] = clearColor;
    }
}
