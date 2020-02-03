#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <cx16.h>

const int xDungeonSize = 80;
const int yDungeonSize = 60;
const int minRoomSize = 8;

typedef unsigned char byte;

typedef enum
{
    dir_horizontal,
    dir_vertical
} direction;

typedef struct _room
{
    byte x0, y0, x1, y1;
    struct _room *subRoom1;
    struct _room *subRoom2;
} room;

void dumpRoom(room *aRoom)
{
    byte i;
    for (i = aRoom->x0; i < aRoom->x1; ++i)
    {
        cputcxy(i, aRoom->y0, '*');
        cputcxy(i, aRoom->y1, '*');
    }
    for (i = aRoom->y0; i < aRoom->y1; ++i)
    {
        cputcxy(aRoom->x0, i, '*');
        cputcxy(aRoom->x1, i, '*');
    }
}

void dumpAllRooms(room *startRoom)
{
    if (!startRoom)
    {
        return;
    }
    if (startRoom->subRoom1 == NULL)
    {
        dumpRoom(startRoom);
    }
    dumpAllRooms(startRoom->subRoom1);
    dumpAllRooms(startRoom->subRoom2);
}

room *newRoom(byte x0, byte y0, byte x1, byte y1)
{
    room *aRoom;
    aRoom = (room *)malloc(sizeof(room));
    aRoom->x0 = x0;
    aRoom->y0 = y0;
    aRoom->x1 = x1;
    aRoom->y1 = y1;
    aRoom->subRoom1 = NULL;
    aRoom->subRoom2 = NULL;
    // printf("new room %d,%d,%d,%d at %x\n", x0, y0, x1, y1, aRoom);
    return aRoom;
}

byte isLeafRoom(room *aRoom)
{
    return (aRoom->subRoom1 == NULL && aRoom->subRoom2 == NULL);
}

void splitRoom(room *aRoom)
{
    direction splitDir;
    byte splitPoint;
    byte roomWidth;
    byte roomHeight;
    int variance;
    room *subroom1 = NULL;
    room *subroom2 = NULL;

    if (!aRoom)
        return;

    variance = minRoomSize/2;

    roomWidth = aRoom->x1 - aRoom->x0;
    roomHeight = aRoom->y1 - aRoom->y0;

    splitDir = (rand() & 1) ? dir_horizontal : dir_vertical;

    if ((splitDir == dir_horizontal) && roomWidth >= (3 * minRoomSize))
    {
        splitPoint = aRoom->x0 + roomWidth / 2;
        splitPoint += (-variance + rand() % ((variance*2)+1));

        subroom1 = newRoom(aRoom->x0, aRoom->y0, splitPoint, aRoom->y1);
        subroom2 = newRoom(splitPoint, aRoom->y0, aRoom->x1, aRoom->y1);
    }
    else if ((splitDir == dir_vertical) && roomHeight >= (3 * minRoomSize))
    {
        splitPoint = aRoom->y0 + roomHeight / 2;
        splitPoint += (-variance + rand() % ((variance*2)+1));

        subroom1 = newRoom(aRoom->x0, aRoom->y0, aRoom->x1, splitPoint);
        subroom2 = newRoom(aRoom->x0, splitPoint, aRoom->x1, aRoom->y1);
    }

    aRoom->subRoom1 = subroom1;
    aRoom->subRoom2 = subroom2;
    splitRoom(subroom1);
    splitRoom(subroom2);
}

room *createRooms(void)
{
    room *startRoom;
    startRoom = newRoom(0, 0, xDungeonSize - 1, yDungeonSize - 1);
    splitRoom(startRoom);
    return startRoom;
}

void freeRooms(room *aRoom)
{
    if (!aRoom)
    {
        return;
    }
    freeRooms(aRoom->subRoom1);
    freeRooms(aRoom->subRoom2);
    free(aRoom);
}

void main()
{
    room *aRoom = NULL;
    textcolor(COLOR_GREEN);
    bgcolor(COLOR_BLACK);
    do
    {
        freeRooms(aRoom);
        clrscr();
        aRoom = createRooms();
        dumpAllRooms(aRoom);
    } while (cgetc() != 'q');
}
