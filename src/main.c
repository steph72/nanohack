#include <cbm.h>
#include <conio.h>
#include <stdlib.h>

#include "dungeon.h"

#ifdef __CX16__
const int xDungeonSize = 80;
const int yDungeonSize = 58;
const int minDungeonRoomCount = 10;
const int minRoomSize = 2;
#else
const int xDungeonSize = 40;
const int yDungeonSize = 24;
const int minDungeonRoomCount = 0;
const int minRoomSize = 2;
#endif

const char signs[] = {' ', '.', '#', 'X', '*'};

void dumpDungeon(dungeonDescriptor *desc)
{
    byte *canvas;
    byte width;
    byte height;
    register byte x, y;

    canvas = desc->canvas;
    width = desc->width;
    height = desc->height;

    clrscr();
    for (x = 0; x < width; ++x)
    {
        for (y = 0; y < height; ++y)
        {
            cputcxy(x, y, canvas[x + (width * y)]);
        }
    }
}

unsigned int debugMem(void)
{
    unsigned int t;
    int *m;
    m = malloc(1024);
    t = (unsigned int)m;
    free(m);
    return t;
}

void main()
{
    dungeonDescriptor *aDungeon;

    textcolor(0x55);
    bgcolor(0);
    bordercolor(0x15);
    srand(501);
    do
    {
        clrscr();
        aDungeon = createDungeon(xDungeonSize,
                                 yDungeonSize,
                                 minDungeonRoomCount,
                                 minRoomSize);
        dumpDungeon(aDungeon);
        gotoxy(0, 24);
        cputhex16(debugMem());
        cputc(' ');
        cputhex8(aDungeon->numRooms);
        cputc(' ');
        deallocDungeon(aDungeon);
        cputc(' ');
        cputhex16(debugMem());
        cgetc();

    } while (1);
}
