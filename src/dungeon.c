#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <conio.h>
#include <stdio.h>

#include "dungeon.h"

// 5828

const int kErrNoRoomForDungeon = 0x10;

#define MAXTRIES 64

int _gDungeonWidth;
int _gDungeonHeight;
int _gMinRooms;
int _gMinRoomSize;
dungeonElement *_gCanvas;


typedef enum
{
    dir_horizontal,
    dir_vertical
} direction;

typedef struct {
    byte x0,y0,x1,y1;
} rect;

typedef struct _frame
{
    rect frameRect;
    rect roomRect;
    direction splitDir;
    byte hasConnectedChildren;
    struct _frame *parent;
    struct _frame *subframe1;
    struct _frame *subframe2;
    // room *frameRoom;
} frame;

frame *newFrame(byte x0, byte y0, byte x1, byte y1)
{
    frame *aFrame;
    aFrame = (frame *)malloc(sizeof(frame));
    aFrame->frameRect.x0 = x0;
    aFrame->frameRect.y0 = y0;
    aFrame->frameRect.x1 = x1;
    aFrame->frameRect.y1 = y1;
    aFrame->hasConnectedChildren = 0;
    aFrame->subframe1 = NULL;
    aFrame->subframe2 = NULL;
    aFrame->parent = NULL;
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

    frameSizeThreshhold = (minFrameSize * 35) / 10;

    frameWidth = aFrame->frameRect.x1 - aFrame->frameRect.x0;
    frameHeight = aFrame->frameRect.y1 - aFrame->frameRect.y0;

    splitDir = (rand() & 1) ? dir_horizontal : dir_vertical;
    aFrame->splitDir = splitDir;

    if ((splitDir == dir_horizontal) && frameWidth >= frameSizeThreshhold)
    {
        splitPoint = aFrame->frameRect.x0 + minFrameSize + (rand() % (frameWidth - (minFrameSize * 2)));
        subframe1 = newFrame(aFrame->frameRect.x0, aFrame->frameRect.y0, splitPoint, aFrame->frameRect.y1);
        subframe2 = newFrame(splitPoint, aFrame->frameRect.y0, aFrame->frameRect.x1, aFrame->frameRect.y1);
    }
    else if ((splitDir == dir_vertical) && frameHeight >= frameSizeThreshhold)
    {
        splitPoint = aFrame->frameRect.y0 + minFrameSize + (rand() % (frameHeight - (minFrameSize * 2)));

        subframe1 = newFrame(aFrame->frameRect.x0, aFrame->frameRect.y0, aFrame->frameRect.x1, splitPoint);
        subframe2 = newFrame(aFrame->frameRect.x0, splitPoint, aFrame->frameRect.x1, aFrame->frameRect.y1);
    }

    if (subframe1)
    {
        subframe1->parent = aFrame;
        subframe2->parent = aFrame;
        aFrame->subframe1 = subframe1;
        aFrame->subframe2 = subframe2;
        splitFrame(subframe1, minFrameSize);
        splitFrame(subframe2, minFrameSize);
    }
}

void deallocFrames(frame *aFrame)
{
    if (!aFrame)
    {
        return;
    }
    deallocFrames(aFrame->subframe1);
    deallocFrames(aFrame->subframe2);
    free(aFrame);
}

byte _countLeafFrames(frame *startFrame, byte currentCount)
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

byte countLeafFrames(frame *startFrame)
{
    return _countLeafFrames(startFrame, 0);
}

frame *createFrames(byte width, byte height, byte minFrameCount, byte minFrameSize)
{
    frame *startFrame = NULL;
    do
    {
        deallocFrames(startFrame);
        startFrame = newFrame(0, 0, width - 1, height - 1);
        splitFrame(startFrame, minFrameSize);
    } while (countLeafFrames(startFrame) < minFrameCount);
    return startFrame;
}

// create room for a given frame
// returns number of floor spaces created
int createRoomForFrame(frame *aFrame)
{
    byte roomWidth;
    byte roomHeight;
    byte randomSize;
    byte x0, y0, x1, y1;
    int numSpaces = 0;

    x0 = aFrame->frameRect.x0;
    y0 = aFrame->frameRect.y0;
    x1 = aFrame->frameRect.x1;
    y1 = aFrame->frameRect.y1;

    x0 = (x0 == 0) ? x0 : x0 + 1;
    y0 = (y0 == 0) ? y0 : y0 + 1;
    x1 = (x1 == _gDungeonWidth - 1) ? x1 : x1 - 1;
    y1 = (y1 == _gDungeonHeight - 1) ? y1 : y1 - 1;

    roomWidth = x1 - x0 - 2;
    roomHeight = y1 - y0 - 2;

    if (roomWidth > (_gMinRoomSize + 2))
    {
        randomSize = _gMinRoomSize + 1 + (rand() % (roomWidth - _gMinRoomSize));
        x0 = x0 + ((roomWidth - randomSize) / 2);
        x1 = x0 + randomSize;
    }

    if (roomHeight > (_gMinRoomSize + 2))
    {
        randomSize = _gMinRoomSize + 1 + (rand() % (roomHeight - _gMinRoomSize));
        y0 = y0 + ((roomHeight - randomSize) / 2);
        y1 = y0 + randomSize;
    }

    aFrame->roomRect.x0 = x0;
    aFrame->roomRect.x1 = x1;
    aFrame->roomRect.y0 = y0;
    aFrame->roomRect.y1 = y1;

    roomWidth = x1 - x0 - 2;
    roomHeight = y1 - y0 - 2;
    numSpaces = (roomWidth * roomHeight);

    return numSpaces;
}

void instantiateRoomInDungeon(frame *aFrame)
{
    register byte x;
    register byte y;
    byte x0, x1, y0, y1;

    x0 = aFrame->roomRect.x0;
    y0 = aFrame->roomRect.y0;
    x1 = aFrame->roomRect.x1;
    y1 = aFrame->roomRect.y1;

    for (x = x0; x <= x1; ++x)
    {
        for (y = y0; y <= y1; ++y)
        {
            _gCanvas[x + (y * _gDungeonWidth)] =
                ((x == x0 || x == x1 || y == y0 || y == y1)) ? de_wall : de_floor;
        }
    }
}

void instantiateRooms(frame *startFrame)
{
    if (!startFrame)
    {
        return;
    }
    if (isLeaf(startFrame))
    {
        instantiateRoomInDungeon(startFrame);
    }
    else
    {
        instantiateRooms(startFrame->subframe1);
        instantiateRooms(startFrame->subframe2);
    }
}

int createRooms(frame *startFrame)
{
    int numSpaces = 0;

    if (isLeaf(startFrame))
    {
        numSpaces += createRoomForFrame(startFrame);
    }
    else
    {
        numSpaces += createRooms(startFrame->subframe1);
        numSpaces += createRooms(startFrame->subframe2);
    }
    return numSpaces;
}

byte midX(rect *aRect) {
    return aRect->x0 + ((aRect->x1 - aRect->x0)/2);
}

byte midY(rect *aRect) {
    return aRect->y0 + ((aRect->y1 - aRect->y0)/2);
}


void connectRects(rect *rect1, rect *rect2) {

    byte xc1,yc1,xc2,yc2;

    xc1 = midX(rect1);
    yc1 = midY(rect1);
    xc2 = midX(rect2);
    yc2 = midY(rect2);

    printf("%d %d %d %d \n",xc1,yc1,xc2,yc2);

    // _gCanvas[xc2 + (yc2 * _gDungeonWidth)] = de_rock;

}


void connectRoomToSibling(frame *startFrame)
{

    rect *rect1 = NULL;
    rect *rect2 = NULL;

    frame *parent = NULL;
    frame *otherFrame = NULL;

    rect1 = &(startFrame->roomRect);
    parent = startFrame->parent;

    if (parent->subframe1 == startFrame)
    {
        otherFrame = parent->subframe2;
    }
    else
    {
        otherFrame = parent->subframe1;
    }

    if (isLeaf(otherFrame)) {
        rect2 = &(otherFrame->roomRect);
    } else {
        rect2 = &(otherFrame->frameRect);
    }

    connectRects(rect1,rect2);
}

void createHallwaysBetweenRooms(frame *startFrame)
{

    if (isLeaf(startFrame))
    {
        connectRoomToSibling(startFrame);
    }
    else
    {
        createHallwaysBetweenRooms(startFrame->subframe1);
        createHallwaysBetweenRooms(startFrame->subframe2);
    }
}

void createHallways(frame *startFrame)
{
    createHallwaysBetweenRooms(startFrame);
    cgetc();
}

dungeonDescriptor *createDungeon(byte width,
                                 byte height,
                                 byte minRoomCount,
                                 byte minRoomSize,
                                 byte minSurfaceSize)
{
    int surfaceSize;
    unsigned int tries;
    byte currentRoomIdx = 0;

    frame *startFrame = NULL;

    // create dungeon structure and canvas array
    dungeonDescriptor *ddesc = (dungeonDescriptor *)malloc(sizeof(dungeonDescriptor));
    ddesc->width = width;
    ddesc->height = height;
    ddesc->canvas = (dungeonElement *)malloc(width * height * sizeof(dungeonElement));

    if (!(ddesc->canvas))
    {
        exit(kErrNoRoomForDungeon);
    }

    bzero(ddesc->canvas, width * height * sizeof(dungeonElement));

    _gDungeonHeight = height;
    _gDungeonWidth = width;
    _gMinRooms = minRoomCount;
    _gMinRoomSize = minRoomSize;
    _gCanvas = ddesc->canvas;

    /*
            min size for frame is
              minRoomSize
              + 2 spaces for walls
              + 1 space for room between walls
        */

    do
    {
        tries = 0;
        deallocFrames(startFrame);
        startFrame = createFrames(width,
                                  height,
                                  minRoomCount,
                                  minRoomSize + 3);
        do
        {
            surfaceSize = createRooms(startFrame);
            ++tries;
            gotoxy(0, 0);
            cputhex16(tries);
        } while (surfaceSize < minSurfaceSize && tries < MAXTRIES);
    } while (tries >= MAXTRIES);

    ddesc->numRooms = countLeafFrames(startFrame);
    ddesc->surfaceCount = surfaceSize;

    // fill in room list
    instantiateRooms(startFrame);
    createHallways(startFrame);
    deallocFrames(startFrame);

    return ddesc;
}

void deallocDungeon(dungeonDescriptor *desc)
{
    if (desc->canvas)
    {
        free(desc->canvas);
    }
    free(desc);
}