#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define CELL_N      0x01
#define CELL_NE     0x02
#define CELL_E      0x04
#define CELL_SE     0x08
#define CELL_S      0x10
#define CELL_SW     0x20
#define CELL_W      0x40
#define CELL_NW     0x80

typedef union
{
    uint32_t raw;
    struct
    {
        uint8_t hasAnt : 1;
        uint8_t antState : 4;
        uint8_t : 3;

        uint8_t pheromoneStrength : 8;

        uint8_t hasFood : 1;
        uint8_t : 7;

        uint8_t isWall : 1;
        uint8_t : 7;
    } parts;
} aaCell;

typedef struct
{
    aaCell* grid;
    struct
    {
        uint16_t width;
        uint16_t height;
    } aaProps;
} aaGrid;

uint8_t bitcount (uint32_t n)  {
   uint8_t count = 0 ;
   while (n)  {
      count++ ;
      n &= (n - 1) ;
   }
   return count;
}

void processCell(aaGrid* gridp, uint16_t x, uint16_t y);
void setupGrid(aaGrid* gridp);
void printGrid(aaGrid* gridp);
aaCell* getCell(aaGrid* gridp, uint16_t x, uint16_t y);
uint8_t getValidNeighbors(aaGrid* gridp, uint16_t x, uint16_t y);
uint8_t chooseRandomDirection(uint8_t dirMask);

int main(void)
{
    aaCell gridcells[400];
    aaGrid grid;
    grid.grid = gridcells;
    grid.aaProps.width = 20;
    grid.aaProps.height = 20;

    setupGrid(&grid);

    getCell(&grid, 2, 2)->parts.hasAnt = 1;

    printf("%x\n", chooseRandomDirection(0x5A));
    printf("%x\n", chooseRandomDirection(0x5A));
    printf("%x\n", chooseRandomDirection(0x5A));
    printf("%x\n", chooseRandomDirection(0x5A));
    printf("%x\n", chooseRandomDirection(0x5A));

    printGrid(&grid);
    return 0;
}

/**
    return a pointer to aaCell for the requested cell, or NULL if the indices are out of bounds
*/
aaCell* getCell(aaGrid* gridp, uint16_t x, uint16_t y)
{
    if(x > gridp->aaProps.width)
        return NULL;
    if(y > gridp->aaProps.height)
        return NULL;
    return gridp->grid + ((y * gridp->aaProps.width) + x);
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
            else if(gridp->grid[i*gridp->aaProps.width + j].parts.hasFood)
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

uint8_t getValidNeighbors(aaGrid* gridp, uint16_t x, uint16_t y)
{
    uint8_t ret = 0;
    aaCell* testCell = NULL;
    if(y > 0)
    {
        testCell = getCell(gridp, x, y - 1);
        if(!(testCell->parts.hasAnt || testCell->parts.isWall))
            ret |= CELL_N;
    }
    if(y > 0 && x > 0)
    {
        testCell = getCell(gridp, x - 1, y - 1);
        if(!(testCell->parts.hasAnt || testCell->parts.isWall))
            ret |= CELL_NW;
    }
    if(x > 0)
    {
        testCell = getCell(gridp, x - 1, y);
        if(!(testCell->parts.hasAnt || testCell->parts.isWall))
            ret |= CELL_W;
    }
    if(y < (gridp->aaProps.height-1) && x > 0)
    {
        testCell = getCell(gridp, x - 1, y + 1);
        if(!(testCell->parts.hasAnt || testCell->parts.isWall))
            ret |= CELL_SW;
    }
    if(y < (gridp->aaProps.height-1))
    {
        testCell = getCell(gridp, x, y + 1);
        if(!(testCell->parts.hasAnt || testCell->parts.isWall))
            ret |= CELL_S;
    }
    if(y < (gridp->aaProps.height-1) && x < (gridp->aaProps.width-1))
    {
        testCell = getCell(gridp, x + 1, y + 1);
        if(!(testCell->parts.hasAnt || testCell->parts.isWall))
            ret |= CELL_SE;
    }
    if(x < (gridp->aaProps.width-1))
    {
        testCell = getCell(gridp, x + 1, y);
        if(!(testCell->parts.hasAnt || testCell->parts.isWall))
            ret |= CELL_E;
    }
    if(y > 0 && x < (gridp->aaProps.width-1))
    {
        testCell = getCell(gridp, x + 1, y - 1);
        if(!(testCell->parts.hasAnt || testCell->parts.isWall))
            ret |= CELL_NE;
    }
    return ret;
}

uint8_t chooseRandomDirection(uint8_t dirMask)
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

void processCell(aaGrid* gridp,  uint16_t x, uint16_t y)
{
    if(getCell(gridp, x, y)->parts.hasAnt)
    {
        uint8_t validNeighbors = getValidNeighbors(gridp, x, y);
    }
}

void setupGrid(aaGrid* gridp)
{
    uint16_t i,j;
    for(i = 0; i < gridp->aaProps.height; i++)
    {
        for(j = 0; j < gridp->aaProps.width; j++)
        {
            gridp->grid[i*gridp->aaProps.width + j].raw = 0;
        }
        gridp->grid[i*gridp->aaProps.height].parts.isWall = 1;
        gridp->grid[i*gridp->aaProps.height +gridp->aaProps.width - 1].parts.isWall = 1;
    }

    for(j = 0; j < gridp->aaProps.width; j++)
    {
        gridp->grid[j].parts.isWall = 1;
        gridp->grid[j+((gridp->aaProps.height-1)*gridp->aaProps.width)].parts.isWall = 1;
    }
}
