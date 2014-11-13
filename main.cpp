#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define USE_SDL

#ifdef USE_SDL
#include <SDL2/SDL.h>
#include "SDL_DisplayInterface.h"
#endif


#define CELL_N          0x01
#define CELL_NE         0x02
#define CELL_E          0x04
#define CELL_SE         0x08
#define CELL_S          0x10
#define CELL_SW         0x20
#define CELL_W          0x40
#define CELL_NW         0x80

#define ANT_SEARCH      0x01    // Vs. returning to home
#define ANT_ACTED       0x02

typedef enum
{
    ANT_BIAS_NONE = 0,
    ANT_BIAS_N = 1,
    ANT_BIAS_NE = 2,
    ANT_BIAS_E = 3,
    ANT_BIAS_SE = 4,
    ANT_BIAS_S = 5,
    ANT_BIAS_SW = 6,
    ANT_BIAS_W = 7,
    ANT_BIAS_NW = 8
} ant_bias_e;

typedef union
{
    uint32_t raw;
    struct
    {
        // Ant params
        uint8_t hasAnt :                1;
        uint8_t antState :              2;
        uint8_t antBias :               4;
        uint8_t :                       1;

        uint8_t targetDir :             8;

        // Pheromone strength
        uint8_t pheromoneStrength :     8;

        // Non-moving params
        uint8_t isFood :                1;
        uint8_t isWall :                1;
        uint8_t isNest :                1;
        uint8_t :                       5;

    } parts;
} aaCell;

typedef struct
{
    int32_t x;
    int32_t y;
} gridPoint;

typedef uint8_t dir_t;

typedef struct
{
    aaCell* grid;
    struct
    {
        int32_t width;
        int32_t height;
    } aaProps;
} aaGrid;

uint8_t bitcount (uint32_t n)
{
    uint8_t count = 0 ;
    while (n)
    {
        count++ ;
        n &= (n - 1) ;
    }
    return count;
}

#ifdef USE_SDL
//---------------SDL STUFF--------------------
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
SDL_Surface * surface;
SDL_Window * window;

SDL_DisplayInterface di(&surface);

SDL_Event gameEvent;
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//---------------SDL STUFF--------------------
#endif //USE_SDL

void processCell(aaGrid* gridp, gridPoint pt);
void setupGrid(aaGrid* gridp);
void printGrid(aaGrid* gridp);
aaCell* getCell(aaGrid* gridp, gridPoint pt);
aaCell* getCellRelative(aaGrid* gridp, gridPoint pt, dir_t dir);
dir_t getValidNeighbors(aaGrid* gridp, gridPoint pt);
dir_t chooseRandomDirection(dir_t dirMask);
void updateGrid(aaGrid* gridp);
dir_t getDirBiasMask(uint8_t bias);
void SDLDisplayGrid(aaGrid* gridp);

#ifdef USE_SDL
int main(int argc, char* argv[])
#else
int main(void)
#endif
{
    int gridWidth = 600, gridHeight = 600;
    aaCell gridcells[gridWidth * gridHeight];
    aaGrid grid;
    grid.grid = gridcells;
    grid.aaProps.width = gridWidth;
    grid.aaProps.height = gridHeight;

#ifdef USE_SDL
    //---------------SDL STUFF--------------------
    //vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    SDL_Init(SDL_INIT_EVERYTHING);

    window = SDL_CreateWindow("Game Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, gridWidth, gridHeight, 0);
    surface = SDL_GetWindowSurface(window);
    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //---------------SDL STUFF--------------------
#endif //USE_SDL

    setupGrid(&grid);

    //getCell(&grid, {2,2})->parts.hasAnt = 1;

    for(int i = (gridWidth/2 - 30); i < (gridWidth/2 + 30); i++)
        for(int j = (gridHeight/2 - 30); j < (gridHeight/2 + 30); j++)
            getCell(&grid, {i,j})->parts.hasAnt = 1;
    bool update = false;
    while(true)
    {
#ifdef USE_SDL
        SDL_PumpEvents();
        //if(update)
        //{

        SDL_LockSurface(surface);
        SDLDisplayGrid(&grid);
        SDL_UnlockSurface(surface);
        updateGrid(&grid);

        //    update = false;
        //}

        if(SDL_PollEvent(&gameEvent))
        {
            if(gameEvent.type == SDL_QUIT)
                break;
            //if(gameEvent.type == SDL_KEYDOWN && gameEvent.key.keysym.scancode == SDL_SCANCODE_SPACE)
            //    update = true;
            //if(gameEvent.type == SDL_MOUSEMOTION)
            //{
            //    di.drawPoint(gameEvent.motion.x, gameEvent.motion.y, {0,0,0});
            //}
        }

        SDL_UpdateWindowSurface(window);
#else
        printGrid(&grid);
        updateGrid(&grid);
        getc(stdin);
#endif
    }

#ifdef USE_SDL
    //---------------SDL STUFF--------------------
    //vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    SDL_FreeSurface(surface);

    SDL_Quit();
    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //---------------SDL STUFF--------------------
#endif //USE_SDL

    return 0;
}

/**
    return a pointer to aaCell for the requested cell, or NULL if the indices are out of bounds
*/
aaCell* getCell(aaGrid* gridp, gridPoint pt)
{
    if(pt.x > gridp->aaProps.width || pt.x < 0 || pt.y > gridp->aaProps.height || pt.y < 0)
        return NULL;
    return gridp->grid + ((pt.y * gridp->aaProps.width) + pt.x);
}

bool isRelativeCellValid(aaGrid* gridp, gridPoint pt, dir_t dir)
{
    return (getCellRelative(gridp, pt, dir) != NULL);
}

aaCell* getCellRelative(aaGrid* gridp, gridPoint pt, dir_t dir)
{
    uint8_t count = bitcount(dir);
    if(count == 0)
        return getCell(gridp, pt);
    else if(count != 1)
        return NULL;
    switch(dir)
    {
    case CELL_N:
        pt.y -= 1;
        return getCell(gridp, pt);
        break;
    case CELL_NE:
        pt.x += 1;
        pt.y -= 1;
        return getCell(gridp, pt);
        break;
    case CELL_E:
        pt.x += 1;
        return getCell(gridp, pt);
        break;
    case CELL_SE:
        pt.x += 1;
        pt.y += 1;
        return getCell(gridp, pt);
        break;
    case CELL_S:
        pt.y += 1;
        return getCell(gridp, pt);
        break;
    case CELL_SW:
        pt.x -= 1;
        pt.y += 1;
        return getCell(gridp, pt);
        break;
    case CELL_W:
        pt.x -= 1;
        return getCell(gridp, pt);
        break;
    case CELL_NW:
        pt.x -= 1;
        pt.y -= 1;
        return getCell(gridp, pt);
        break;
    }
    return NULL;
}

#ifdef USE_SDL
void SDLDisplayGrid(aaGrid* gridp)
{
    uint16_t i, j;
    for(i = 0; i < gridp->aaProps.height; i++)
    {
        for(j = 0; j < gridp->aaProps.width; j++)
        {
            aaCell* currentCell = getCell(gridp, {j, i});
            if(currentCell->parts.isWall)
                di.drawPoint(j, i, {255,255,255});
            else if(currentCell->parts.isFood)
                di.drawPoint(j, i, {0,0,255});
            else if(currentCell->parts.hasAnt)
                di.drawPoint(j, i, {255,0,0});
            else if(currentCell->parts.pheromoneStrength)
                di.drawPoint(j, i, {currentCell->parts.pheromoneStrength, 0, currentCell->parts.pheromoneStrength});
            else
                di.drawPoint(j, i, {0,0,0});
        }
    }
}
#endif //USE_SDL

void printGrid(aaGrid* gridp)
{
    uint16_t i, j;
    for(i = 0; i < gridp->aaProps.height; i++)
    {
        for(j = 0; j < gridp->aaProps.width; j++)
        {
            if(gridp->grid[i*gridp->aaProps.width + j].parts.isWall)
                putc('#', stdout);
            else if(gridp->grid[i*gridp->aaProps.width + j].parts.isFood)
                putc('@', stdout);
            else if(gridp->grid[i*gridp->aaProps.width + j].parts.hasAnt)
                putc('A', stdout);
            else
                putc('.', stdout);
            putc(' ', stdout);
        }
        putc('\n', stdout);
    }
}

dir_t getValidNeighbors(aaGrid* gridp, gridPoint pt)
{
    uint8_t ret = 0;
    aaCell* testCell = NULL;

    testCell = getCell(gridp, {pt.x, pt.y - 1});
    if(!(testCell == NULL || testCell->parts.hasAnt || testCell->parts.isWall))
        ret |= CELL_N;
    testCell = getCell(gridp, {pt.x - 1, pt.y - 1});
    if(!(testCell == NULL || testCell->parts.hasAnt || testCell->parts.isWall))
        ret |= CELL_NW;
    testCell = getCell(gridp, {pt.x - 1, pt.y});
    if(!(testCell == NULL || testCell->parts.hasAnt || testCell->parts.isWall))
        ret |= CELL_W;
    testCell = getCell(gridp, {pt.x - 1, pt.y + 1});
    if(!(testCell == NULL || testCell->parts.hasAnt || testCell->parts.isWall))
        ret |= CELL_SW;
    testCell = getCell(gridp, {pt.x, pt.y + 1});
    if(!(testCell == NULL || testCell->parts.hasAnt || testCell->parts.isWall))
        ret |= CELL_S;
    testCell = getCell(gridp, {pt.x + 1, pt.y + 1});
    if(!(testCell == NULL || testCell->parts.hasAnt || testCell->parts.isWall))
        ret |= CELL_SE;
    testCell = getCell(gridp, {pt.x + 1, pt.y});
    if(!(testCell == NULL || testCell->parts.hasAnt || testCell->parts.isWall))
        ret |= CELL_E;
    testCell = getCell(gridp, {pt.x + 1, pt.y - 1});
    if(!(testCell == NULL || testCell->parts.hasAnt || testCell->parts.isWall))
        ret |= CELL_NE;

    return ret;
}

dir_t chooseRandomDirection(dir_t dirMask)
{
    if(dirMask == 0)
        return 0;
    uint8_t bits = bitcount(dirMask);
    uint8_t selection = (rand()%bits)+1;
    uint8_t sbit = 0;
    while(selection > 0)
    {
        if(dirMask & 0x01)
            selection--;
        sbit++;
        dirMask >>= 1;
    }
    return 1<<(sbit-1);
}

dir_t getDirBiasMask(uint8_t bias)
{
    if(bias == ANT_BIAS_NONE)
        return 0xFF;
    // Create a buffer with 5 bits at the bottom
    uint16_t tempMask = 0x0E;
    // Rotate the buffer around an 8-bit block by (bias-1)
    tempMask <<= (bias - 1);
    tempMask |= ((tempMask & 0x3) << 8);
    tempMask |= ((tempMask & 0xc00) >> 8);
    // shift, truncate, return
    return (dir_t)(tempMask >> 2);
}

uint8_t gridOrderCounter = 0;

void updateGrid(aaGrid* gridp)
{
    uint16_t i,j;
    if(gridOrderCounter & 0x01)
    {
        for(i = 0; i < gridp->aaProps.height; i++)
        {
            for(j = 0; j < gridp->aaProps.width; j++)
            {
                if(gridOrderCounter & 0x02)
                    processCell(gridp, {j, i});
                else
                    processCell(gridp, {gridp->aaProps.width - 1 - j, gridp->aaProps.height - 1 - i});
            }
        }
    }
    else
    {

        for(j = 0; j < gridp->aaProps.width; j++)
        {
            for(i = 0; i < gridp->aaProps.height; i++)
            {
                if(gridOrderCounter & 0x02)
                    processCell(gridp, {j, i});
                else
                    processCell(gridp, {gridp->aaProps.width - 1 - j, gridp->aaProps.height - 1 - i});
            }
        }
    }

    // Post-process (clean up states)
    for(i = 0; i < gridp->aaProps.height; i++)
    {
        for(j = 0; j < gridp->aaProps.width; j++)
        {
            aaCell* currentCell = getCell(gridp, {j, i});
            currentCell->parts.antState &= ~ANT_ACTED;
            if(currentCell->parts.pheromoneStrength > 0)
                currentCell->parts.pheromoneStrength -= 1;
        }
    }

    gridOrderCounter = (gridOrderCounter + 1) % 4;
}

uint8_t getLowestBitPos(uint32_t input)
{
    if(input == 0)
        return 0xFF;
    uint8_t pos = 0;
    while(!(input & (1<<pos)))
        pos++;
    return pos;
}

void processCell(aaGrid* gridp, gridPoint pt)
{
    aaCell* currentCell = getCell(gridp, pt);
    if(currentCell->parts.hasAnt && !(currentCell->parts.antState & ANT_ACTED))
    {
        // Find valid neighbors
        dir_t validNeighbors = getValidNeighbors(gridp, pt);
        // Compute the bias mask based upon the last movement direction
        dir_t biasMask = getDirBiasMask(currentCell->parts.antBias);
        // Mask the valid neighbors by the bias mask
        validNeighbors &= biasMask;
        // Choose a random move direction
        dir_t moveDir = chooseRandomDirection(validNeighbors);
        if(moveDir)
        {
            aaCell* targetCell = getCellRelative(gridp, pt, moveDir);
            targetCell->parts.hasAnt = 1;
            targetCell->parts.antState = currentCell->parts.antState | ANT_ACTED;
            targetCell->parts.antBias = (getLowestBitPos(moveDir)+1);

            currentCell->parts.antBias = 0;
            currentCell->parts.antState = 0;
            currentCell->parts.hasAnt = 0;

            uint16_t newPStrength = currentCell->parts.pheromoneStrength;
            newPStrength += 50;
            currentCell->parts.pheromoneStrength = (newPStrength > 255)?255:newPStrength;
        }
        else
        {
            currentCell->parts.antState |= ANT_ACTED;
            currentCell->parts.antBias = 0;
        }
        //printf("Valid neighbors: %02x chosen: %02x\n", validNeighbors, moveDir);
    }
}

void setupGrid(aaGrid* gridp)
{
    uint16_t i,j;
    for(i = 0; i < gridp->aaProps.height; i++)
    {
        for(j = 0; j < gridp->aaProps.width; j++)
        {
            getCell(gridp, {j, i})->raw = 0;
        }
    }

    for(i = 0; i < gridp->aaProps.height; i++)
    {
        getCell(gridp, {0, i})->parts.isWall = 1;
        getCell(gridp, {gridp->aaProps.width - 1, i})->parts.isWall = 1;
    }

    for(j = 0; j < gridp->aaProps.width; j++)
    {
        getCell(gridp, {j, 0})->parts.isWall = 1;
        getCell(gridp, {j, gridp->aaProps.height - 1})->parts.isWall = 1;
    }
}
