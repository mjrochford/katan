#include <stdlib.h>

typedef enum CivColor {
    NONE = 0,
    RED = 1,
    ORANGE = 2,
    WHITE = 3,
    BLUE = 4,
} CivColor;


typedef struct Civilization {
    CivColor color;
} Civilization;

typedef struct Intersection Intersection;
typedef struct Path Path;

struct Intersection {
    Civilization *owner;
};

struct Path {
    Civilization *owner;
    Intersection *aIntersection;
    Intersection *bIntersection;
};

typedef enum TerrainType {
    HILLS = 1,
    FOREST = 2,
    MOUNTAINS = 3,
    FIELDS = 4,
    PASTURE = 5,
    DESERT = 6,
} TerrainType; // Must have same values as the corresponding ResourceType enum (casting)

typedef struct TerrainTile {
    size_t rollNumber;
    TerrainType terrainType;
    Intersection *intersections[6];
} TerrainTile;

typedef enum ResourceType {
    BRICK = 1,
    LUMBER = 2,
    ORE = 3,
    GRAIN = 4,
    WOOL = 5,
    TRASH = 6,
} ResourceType; // Must have same values as the corresponding ResourceType enum (casting)

void initalizeGameBoard();
