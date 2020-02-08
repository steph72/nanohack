#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <conio.h>

#include "dungeon.h"

const int kErrNoRoomForDungeon = 0x10;

#define MAXTRIES 64

int _gDungeonWidth;
int _gDungeonHeight;
int _gMinRooms;
int _gMinRoomSize;

typedef enum
{
    dir_horizontal,
    dir_vertical
} direction;

typedef struct _frame
{
    byte x0, y0, x1, y1;
    direction splitDir;
    byte connected;
    struct _frame *parent;
    struct _frame *subframe1;
    struct _frame *subframe2;
    room *frameRoom;
} frame;

frame *newFrame(byte x0, byte y0, byte x1, byte y1)
{
    frame *aFrame;
    aFrame = (frame *)malloc(sizeof(frame));
    aFrame->x0 = x0;
    aFrame->y0 = y0;
    aFrame->x1 = x1;
    aFrame->y1 = y1;
    aFrame->connected = 0;
    aFrame->subframe1 = NULL;
    aFrame->subframe2 = NULL;
    aFrame->frameRoom = NULL;
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

    frameWidth = aFrame->x1 - aFrame->x0;
    frameHeight = aFrame->y1 - aFrame->y0;

    splitDir = (rand() & 1) ? dir_horizontal : dir_vertical;
    aFrame->splitDir = splitDir;

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
    if (aFrame->frameRoom)
    {
        free(aFrame->frameRoom);
    }
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
    room *newRoom;
    byte roomWidth;
    byte roomHeight;
    byte randomSize;
    byte x0, y0, x1, y1;
    int numSpaces = 0;

    if (aFrame->frameRoom != NULL)
    {
        free(aFrame->frameRoom);
    }

    x0 = aFrame->x0;
    y0 = aFrame->y0;
    x1 = aFrame->x1;
    y1 = aFrame->y1;

    x0 = (x0 == 0) ? x0 : x0 + 1;
    y0 = (y0 == 0) ? y0 : y0 + 1;
    x1 = (x1 == _gDungeonWidth - 1) ? x1 : x1 - 1;
    y1 = (y1 == _gDungeonHeight - 1) ? y1 : y1 - 1;

    roomWidth = x1 - x0 - 2;
    roomHeight = y1 - y0 - 2;

    newRoom = (room *)malloc(sizeof(room));

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

    newRoom->x0 = x0;
    newRoom->x1 = x1;
    newRoom->y0 = y0;
    newRoom->y1 = y1;
    aFrame->frameRoom = newRoom;

    roomWidth = x1 - x0 - 2;
    roomHeight = y1 - y0 - 2;
    numSpaces = (roomWidth * roomHeight);

    return numSpaces;
}

void instantiateRoomInDungeon(room *aRoom, dungeonDescriptor *desc)
{
    register byte x;
    register byte y;
    byte x0, x1, y0, y1;
    dungeonElement *gDungeon;
    byte dungeonWidth;

    gDungeon = desc->canvas;
    dungeonWidth = desc->width;
    x0 = aRoom->x0;
    y0 = aRoom->y0;
    x1 = aRoom->x1;
    y1 = aRoom->y1;

    for (x = x0; x <= x1; ++x)
    {
        for (y = y0; y <= y1; ++y)
        {
            gDungeon[x + (y * dungeonWidth)] =
                ((x == x0 || x == x1 || y == y0 || y == y1)) ? de_wall : de_floor;
        }
    }
}

void instantiateRooms(frame *startFrame, dungeonDescriptor *ddesc)
{
    if (!startFrame)
    {
        return;
    }
    if (startFrame->frameRoom)
    {
        instantiateRoomInDungeon(startFrame->frameRoom, ddesc);
    }
    else
    {
        instantiateRooms(startFrame->subframe1, ddesc);
        instantiateRooms(startFrame->subframe2, ddesc);
    }
}

void connectRoomsForFrameUpwards(frame *aFrame, dungeonDescriptor *ddesc) {
    if (aFrame->parent) {
        // connnect two subframes and move on to parent
        aFrame->parent->subframe1->connected = 1;
        aFrame->parent->subframe2->connected = 1;
        connectRoomsForFrameUpwards(aFrame->parent,ddesc);
    }
}

void connectRooms(frame *startFrame, dungeonDescriptor *ddesc) {
    if (isLeaf(startFrame) && (startFrame->connected==0)) {
        connectRoomsForFrameUpwards(startFrame,ddesc);
    } else {
        connectRooms(startFrame->subframe1,ddesc);
        connectRooms(startFrame->subframe2,ddesc);
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

    _gDungeonHeight = height;
    _gDungeonWidth = width;
    _gMinRooms = minRoomCount;
    _gMinRoomSize = minRoomSize;

    bzero(ddesc->canvas, width * height * sizeof(dungeonElement));

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
    instantiateRooms(startFrame, ddesc);
    connectRooms(startFrame,ddesc);
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