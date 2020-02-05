#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <cbm.h>
#include <unistd.h>
#include <string.h>

const int kErrNoRoomForDungeon = 0x10;

const int xDungeonSize = 40;
const int yDungeonSize = 25;
const int dungeonMinFrameSize = 5;
const int minDungeonRoomCount = 8;

const char signs[] = {' ', '.', '#', 'X'};
typedef unsigned char byte;

typedef enum
{
    de_nothing,
    de_floor,
    de_wall,
    de_rock
} dungeonElement;

typedef enum
{
    dir_horizontal,
    dir_vertical
} direction;

typedef struct _room
{
    byte x0, y0, x1, y1;
} room;

typedef struct _frame
{
    byte x0, y0, x1, y1;
    struct _frame *subframe1;
    struct _frame *subframe2;
    room *frameRoom;
} frame;

dungeonElement *gDungeon;

void dumpDungeon()
{
    register byte x, y;
    for (x = 0; x < xDungeonSize; ++x)
    {
        for (y = 0; y < yDungeonSize; ++y)
        {
            cputcxy(x, y, signs[gDungeon[x + (xDungeonSize * y)]]);
        }
    }
}

frame *newFrame(byte x0, byte y0, byte x1, byte y1)
{
    frame *aFrame;
    aFrame = (frame *)malloc(sizeof(frame));
    aFrame->x0 = x0;
    aFrame->y0 = y0;
    aFrame->x1 = x1;
    aFrame->y1 = y1;
    aFrame->subframe1 = NULL;
    aFrame->subframe2 = NULL;
    aFrame->frameRoom = NULL;
    return aFrame;
}

byte isLeaf(frame *aFrame)
{
    return (aFrame->subframe1 == NULL && aFrame->subframe2 == NULL);
}

void splitFrame(frame *aFrame, byte minFrameSize)
{
    direction splitDir;
    byte splitPoint;
    byte frameWidth;
    byte frameHeight;
    int frameSizeThreshhold;
    frame *subframe1 = NULL;
    frame *subframe2 = NULL;

    if (!aFrame)
    {
        return;
    }

    frameSizeThreshhold = (minFrameSize * 35) / 10;

    frameWidth = aFrame->x1 - aFrame->x0;
    frameHeight = aFrame->y1 - aFrame->y0;

    splitDir = (rand() & 1) ? dir_horizontal : dir_vertical;

    if ((splitDir == dir_horizontal) && frameWidth >= frameSizeThreshhold)
    {
        splitPoint = aFrame->x0 + minFrameSize + (rand() % (frameWidth - (minFrameSize * 2)));
        subframe1 = newFrame(aFrame->x0, aFrame->y0, splitPoint, aFrame->y1);
        subframe2 = newFrame(splitPoint, aFrame->y0, aFrame->x1, aFrame->y1);
    }
    else if ((splitDir == dir_vertical) && frameHeight >= frameSizeThreshhold)
    {
        splitPoint = aFrame->y0 + minFrameSize + (rand() % (frameHeight - (minFrameSize * 2)));

        subframe1 = newFrame(aFrame->x0, aFrame->y0, aFrame->x1, splitPoint);
        subframe2 = newFrame(aFrame->x0, splitPoint, aFrame->x1, aFrame->y1);
    }

    aFrame->subframe1 = subframe1;
    aFrame->subframe2 = subframe2;
    splitFrame(subframe1, minFrameSize);
    splitFrame(subframe2, minFrameSize);
}

void deallocFrames(frame *aFrame)
{
    if (!aFrame)
    {
        return;
    }
    deallocFrames(aFrame->subframe1);
    deallocFrames(aFrame->subframe2);
    if (aFrame->frameRoom)
    {
        free(aFrame->frameRoom);
    }
    free(aFrame);
}

int _countLeafFrames(frame *startFrame, int currentCount)
{

    if (isLeaf(startFrame))
    {
        return currentCount + 1;
    }
    else
    {
        return _countLeafFrames(startFrame->subframe1, currentCount) +
               _countLeafFrames(startFrame->subframe2, currentCount);
    }
}

int countLeafFrames(frame *startFrame)
{
    return _countLeafFrames(startFrame, 0);
}

frame *createFrames(byte width, byte height, byte minFrameCount)
{
    frame *startFrame = NULL;
    do
    {
        deallocFrames(startFrame);
        startFrame = newFrame(0, 0, width - 1, height - 1);
        splitFrame(startFrame, dungeonMinFrameSize);
    } while (countLeafFrames(startFrame) < minFrameCount);
    return startFrame;
}

void createRoomForFrame(frame *aFrame)
{
    room *newRoom;
    byte frameWidth;
    byte frameHeight;
    byte x, y;

    frameWidth = aFrame->x1 - aFrame->x0;
    frameHeight = aFrame->y1 - aFrame->y0;

    newRoom = (room *)malloc(sizeof(room));
    newRoom->x0 = aFrame->x0 + 1 + (rand() % ((frameWidth / 2) - 1));
    newRoom->y0 = aFrame->y0 + 1 + (rand() % ((frameHeight / 2) - 1));
    newRoom->x1 = aFrame->x1 - 1 - (rand() % ((frameWidth / 2) - 1));
    newRoom->y1 = aFrame->y1 - 1 - (rand() % ((frameHeight / 2) - 1));
    aFrame->frameRoom = newRoom;

    for (x = newRoom->x0; x <= newRoom->x1; ++x)
    {
        for (y = newRoom->y0; y <= newRoom->y1; ++y)
        {
            if (x == newRoom->x0 || x == newRoom->x1 || y == newRoom->y0 || y == newRoom->y1)
            {
                gDungeon[x + (y * xDungeonSize)] = de_wall;
            }
            else
            {
                gDungeon[x + (y * xDungeonSize)] = de_floor;
            }
        }
    }
}

void createRooms(frame *startFrame)
{
    if (isLeaf(startFrame))
    {
        createRoomForFrame(startFrame);
    }
    else
    {
        createRooms(startFrame->subframe1);
        createRooms(startFrame->subframe2);
    }
}

void initDungeon()
{
    gDungeon = (dungeonElement *)malloc(xDungeonSize * yDungeonSize * sizeof(dungeonElement));
    if (!gDungeon)
    {
        exit(kErrNoRoomForDungeon);
    }
}

void clearDungeon()
{
    bzero(gDungeon, xDungeonSize * yDungeonSize * sizeof(gDungeon));
}

void main()
{
    frame *aFrame = NULL;
    textcolor(0x55);
    bgcolor(0);
    bordercolor(0);
    clrscr();
    initDungeon();
    do
    {
        deallocFrames(aFrame);
        aFrame = createFrames(xDungeonSize, yDungeonSize, minDungeonRoomCount);
        clearDungeon();
        createRooms(aFrame);
        clrscr();
        dumpDungeon();
        //dumpAllFrames(aFrame);
        gotoxy(0, 0);
        cputhex8(countLeafFrames(aFrame));
        sleep(1);
    } while (1);
}
