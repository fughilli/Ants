#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

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
    ANT_BIAS_NONE,
    ANT_BIAS_N,
    ANT_BIAS_NE,
    ANT_BIAS_E,
    ANT_BIAS_SE,
    ANT_BIAS_S,
    ANT_BIAS_SW,
    ANT_BIAS_W,
    ANT_BIAS_NW
} ant_bias_e;

typedef union
{
    uint32_t raw;
    struct
    {
        // Ant params
        uint8_t hasAnt : 1;
        uint8_t antState : 2;
        ant_bias_e antBias : 4;
        uint8_t : 1;

        // Pheromone strength
        uint8_t pheromoneStrength : 8;

        // Non-moving params
        uint8_t isFood : 1;
        uint8_t : 7;

        uint8_t isWall : 1;
        uint8_t isNest : 1;
        uint8_t : 6;
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

void processCell(aaGrid* gridp, gridPoint pt);
void setupGrid(aaGrid* gridp);
void printGrid(aaGrid* gridp);
aaCell* getCell(aaGrid* gridp, gridPoint pt);
aaCell* getCellRelative(aaGrid* gridp, gridPoint pt, dir_t dir);
dir_t getValidNeighbors(aaGrid* gridp, gridPoint pt);
dir_t chooseRandomDirection(dir_t dirMask);
void updateGrid(aaGrid* gridp);
dir_t getDirBiasMask(ant_bias_e bias);

int main(void)
{
    int gridWidth = 20, gridHeight = 20;
    aaCell gridcells[gridWidth * gridHeight];
    aaGrid grid;
    grid.grid = gridcells;
    grid.aaProps.width = gridWidth;
    grid.aaProps.height = gridHeight;

    setupGrid(&grid);

    getCell(&grid, {2,2})->parts.hasAnt = 1;

    while(true)
    {
        printGrid(&grid);
        updateGrid(&grid);
        getc(stdin);
    }
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

dir_t getDirBiasMask(ant_bias_e bias)
{
    if(bias == ANT_BIAS_NONE)
        return 0;
    // Create a buffer with 5 bits at the bottom
    uint16_t tempMask = 0x1F;
    // Rotate the buffer around an 8-bit block by (bias-1)
    tempMask <<= (bias - 1);
    tempMask |= ((tempMask & 0x3) << 8);
    // shift, truncate, return
    return (dir_t)(tempMask >>= 2);
}

void updateGrid(aaGrid* gridp)
{
    uint16_t i,j;
    for(i = 0; i < gridp->aaProps.height; i++)
    {
        for(j = 0; j < gridp->aaProps.width; j++)
        {
            processCell(gridp, {j, i});
        }
    }

    // Post-process (clean up states)
    for(i = 0; i < gridp->aaProps.height; i++)
    {
        for(j = 0; j < gridp->aaProps.width; j++)
        {
            getCell(gridp, {j, i})->parts.antState &= ~ANT_ACTED;
        }
    }
}

void processCell(aaGrid* gridp, gridPoint pt)
{
    aaCell* currentCell = getCell(gridp, pt);
    if(currentCell->parts.hasAnt && !(currentCell->parts.antState & ANT_ACTED))
    {
        uint8_t validNeighbors = getValidNeighbors(gridp, pt);
        uint8_t moveDir = chooseRandomDirection(validNeighbors);
        if(moveDir)
        {
            aaCell* targetCell = getCellRelative(gridp, pt, moveDir);
            targetCell->parts.hasAnt = 1;
            targetCell->parts.antState = currentCell->parts.antState | ANT_ACTED;
            currentCell->parts.antState = 0;
            currentCell->parts.hasAnt = 0;
        }
        else
        {
            currentCell->parts.antState |= ANT_ACTED;
        }
        printf("Valid neighbors: %02x chosen: %02x\n", validNeighbors, moveDir);
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
