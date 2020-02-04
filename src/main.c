#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <cbm.h>
#include <unistd.h>

const int xDungeonSize = 80;
const int yDungeonSize = 60;
const int minFrameSize = 3;

typedef unsigned char byte;

typedef enum
{
    dir_horizontal,
    dir_vertical
} direction;

typedef struct _frame
{
    byte x0, y0, x1, y1;
    struct _frame *subframe1;
    struct _frame *subframe2;
} frame;

void dumpFrame(frame *aFrame)
{
    byte i;
    revers(1);
    for (i = aFrame->x0; i <= aFrame->x1; ++i)
    {
        cputcxy(i, aFrame->y0, ' ');
        cputcxy(i, aFrame->y1, ' ');
    }
    for (i = aFrame->y0; i <= aFrame->y1; ++i)
    {
        cputcxy(aFrame->x0, i, ' ');
        cputcxy(aFrame->x1, i, ' ');
    }
    revers(0);
}

void dumpAllFrames(frame *startFrame)
{
    if (!startFrame)
    {
        return;
    }
    if (startFrame->subframe1 == NULL)
    {
        dumpFrame(startFrame);
    }
    dumpAllFrames(startFrame->subframe1);
    dumpAllFrames(startFrame->subframe2);
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
    // printf("new frame %d,%d,%d,%d at %x\n", x0, y0, x1, y1, aFrame);
    return aFrame;
}

byte isLeaf(frame *aFrame)
{
    return (aFrame->subframe1 == NULL && aFrame->subframe2 == NULL);
}

void splitFrame(frame *aFrame)
{
    direction splitDir;
    byte splitPoint;
    byte frameWidth;
    byte frameHeight;
    int variance;
    frame *subframe1 = NULL;
    frame *subframe2 = NULL;

    if (!aFrame)
        return;

    variance = minFrameSize/2;

    frameWidth = aFrame->x1 - aFrame->x0;
    frameHeight = aFrame->y1 - aFrame->y0;

    splitDir = (rand() & 1) ? dir_horizontal : dir_vertical;

    if ((splitDir == dir_horizontal) && frameWidth >= (3 * minFrameSize))
    {
        splitPoint = aFrame->x0 + frameWidth / 2;
        splitPoint += (-variance + rand() % ((variance*2)+1));

        subframe1 = newFrame(aFrame->x0, aFrame->y0, splitPoint, aFrame->y1);
        subframe2 = newFrame(splitPoint, aFrame->y0, aFrame->x1, aFrame->y1);
    }
    else if ((splitDir == dir_vertical) && frameHeight >= (3 * minFrameSize))
    {
        splitPoint = aFrame->y0 + frameHeight / 2;
        splitPoint += (-variance + rand() % ((variance*2)+1));

        subframe1 = newFrame(aFrame->x0, aFrame->y0, aFrame->x1, splitPoint);
        subframe2 = newFrame(aFrame->x0, splitPoint, aFrame->x1, aFrame->y1);
    }

    aFrame->subframe1 = subframe1;
    aFrame->subframe2 = subframe2;
    splitFrame(subframe1);
    splitFrame(subframe2);
}

frame *createFrames(void)
{
    frame *startFrame;
    startFrame = newFrame(0, 0, xDungeonSize - 1, yDungeonSize - 1);
    splitFrame(startFrame);
    return startFrame;
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

void main()
{
    frame *aFrame = NULL;
    textcolor(COLOR_GREEN);
    bgcolor(COLOR_BLACK);
    bordercolor(COLOR_BLACK);
    do
    {
        deallocFrames(aFrame);
        clrscr();
        aFrame = createFrames();
        dumpAllFrames(aFrame);
        sleep(1);
    } while (1);
}
