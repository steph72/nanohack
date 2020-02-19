extern const int kErrNoRoomForDungeon;

typedef unsigned char byte;

typedef struct _room
{
    byte x0, y0, x1, y1;
} room;

typedef struct
{
    byte width;
    byte height;
    int numRooms;
    int surfaceCount;
    byte *canvas;
} dungeonDescriptor;

dungeonDescriptor *createDungeon(byte width,
                                 byte height,
                                 byte minRoomCount,
                                 byte minRoomSize,
                                 byte minSurfaceSize);

void deallocDungeon(dungeonDescriptor *desc); 