#include "rules.h"
#include "raylib.h"
#include "util.h"
#include <stdbool.h>
#include <stdio.h>

typedef struct Board {
    const size_t nTiles;
    TerrainTile *tiles;
} Board;

#define N_TILES 19
static TerrainTile tiles[N_TILES];

#define INTERSECTIONS_HEIGHT 12
#define INTERSECTIONS_WIDTH 11
static Intersection *intersections[INTERSECTIONS_HEIGHT][INTERSECTIONS_WIDTH];

#define INTERSECTION_STACK_SIZE 255
static Intersection intersectionStack[INTERSECTION_STACK_SIZE];
static size_t intersectionStackPointer = 0;
Intersection *intersectionAdd(size_t x, size_t y, Civilization *owner)
{
    if (intersectionStackPointer >= INTERSECTION_STACK_SIZE) {
        fprintf(stderr, "ERROR: out of game intersection memory");
        exit(1);
    }

    if (intersections[y][x] != NULL) {
        return intersections[y][x];
    }

    intersectionStack[intersectionStackPointer] =
        (Intersection){.owner = owner};
    intersections[y][x] = &intersectionStack[intersectionStackPointer];
    intersectionStackPointer += 1;

    return intersections[y][x];
}

ResourceType getResourceType(Intersection *intersection);

const char *terrainTypeGetString(TerrainType t)
{
    switch (t) {
    case FOREST:
        return "Forest";
    case PASTURE:
        return "Pasture";
    case FIELDS:
        return "Fields";
    case HILLS:
        return "Hills";
    case MOUNTAINS:
        return "Mountains";
    case DESERT:
        return "Desert";
    case TERRAIN_NONE:
        return "Error: invalid terrain type";
    }

    return "error";
}

/* TerrainType pickTerrainType(USizeList *options) { */
/* } */

// See Illustration Q in Catan almanac
const size_t TILE_PROB_SEQUENCE[] = {5, 2, 6,  3, 8, 10, 9, 12, 11,
                                     4, 8, 10, 9, 4, 5,  6, 3,  11};

/**
 * Removes and returns a random (using raylib rand function) element from an
 * array Updates the arr and len pointers to resize array and remove element
 *
 * DOES NOT PRESERVE ORDER
 * */
TerrainType terrainTypeRemoveRandom(TerrainType *arr, size_t *len)
{
    if (len == 0) {
        return TERRAIN_NONE;
    }

    size_t index = GetRandomValue(0, *len - 1);

    TerrainType data = arr[index];
    arr[index] = arr[*len - 1];
    *len -= 1;

    return data;
}

Intersection *intersectionGet(size_t x, size_t y)
{
    return intersections[y][x];
}

void initalizeGameBoard()
{

    const size_t tileHeadLocations[N_TILES][2] = {
        {3, 0}, {7, 0}, {5, 0},

        {2, 2}, {4, 2}, {6, 2}, {8, 2},

        {1, 4}, {3, 4}, {5, 4}, {7, 4}, {9, 4},

        {2, 6}, {4, 6}, {6, 6}, {8, 6},

        {3, 8}, {5, 8}, {7, 8},
    };

    TerrainType tileOptions[] = {
        FOREST,  FOREST,    FOREST,    FOREST,    PASTURE, PASTURE, PASTURE,
        PASTURE, FIELDS,    FIELDS,    FIELDS,    FIELDS,  HILLS,   HILLS,
        HILLS,   MOUNTAINS, MOUNTAINS, MOUNTAINS, DESERT};
    size_t tileOptionsLen = DUM_ARRLEN(tileOptions);

    for (size_t i = 0; i < N_TILES; i++) {
        size_t x = tileHeadLocations[i][0], y = tileHeadLocations[i][1];

        TerrainType terrainType =
            terrainTypeRemoveRandom((void *)&tileOptions, &tileOptionsLen);

        tiles[i] = (TerrainTile){
            .terrainType = terrainType,
            .rollNumber =
                terrainType == DESERT
                    ? 0
                    : TILE_PROB_SEQUENCE[i], // desert tile needs special case
            .intersections =
                {
                    intersectionAdd(x + 0, y + 0, NULL),
                    intersectionAdd(x + 1, y + 1, NULL),
                    intersectionAdd(x + 1, y + 2, NULL),
                    intersectionAdd(x + 0, y + 3, NULL),
                    intersectionAdd(x - 1, y + 2, NULL),
                    intersectionAdd(x - 1, y + 1, NULL),
                },
        };
    }
}

void uSizeListPrint(USizeList *head)
{
    head = head->next;
    printf("{%ld", head->data);
    while (head->next) {
        printf(", %ld", head->next->data);
        head = head->next;
    }
    printf("}\n");
}

void usizeArrayPrint(size_t *arr, size_t len)
{
    printf("{%ld", arr[0]);
    for (size_t i = 1; i < len; i++) {
        printf(", %ld", arr[i]);
    }
    printf("}\n");
}

ResourceType terrainTileGetResourceType(TerrainTile *tile)
{
    return (ResourceType)tile->terrainType;
}

CivResources intersectionGetResources(Intersection *intersection)
{
    CivResources res = {0};
    for (size_t i = 0; i < N_TILES; i++) {
        TerrainTile tile = tiles[i];
        for (size_t j = 0; j < 6; j++) {
            if (tile.intersections[j] == intersection) {
                // This cast only works because we defined both enums to have
                // the same values
                switch (terrainTileGetResourceType(&tile)) {
                case BRICK:
                    res.brick += 1;
                    break;
                case LUMBER:
                    res.lumber += 1;
                    break;
                case ORE:
                    res.ore += 1;
                    break;
                case GRAIN:
                    res.grain += 1;
                    break;
                case WOOL:
                    res.wool += 1;
                    break;
                case TRASH:
                    res.trash += 1;
                    break;
                }
            }
        }
    }
    return res;
}

int boardPlaceSettlement(Intersection *intersection, Civilization *civ)
{
    if (intersection->owner) {
        return -1;
    }

    intersection->owner = civ;

    return 0;
}

void printGameBoard()
{
    // TODO write to an array of char
    printf("     ");
    for (int i = 0; i < INTERSECTIONS_WIDTH; i++) {
        printf("%c", i + 'A');
        printf("\t");
    }
    printf("\n");
    for (int i = 0; i < INTERSECTIONS_WIDTH; i++) {
        printf("--------");
    }
    printf("\n");

    for (int y = 0; y < INTERSECTIONS_HEIGHT; y++) {
        printf("%02d │ ", y);
        for (int x = 0; x < INTERSECTIONS_WIDTH; x++) {
            if (intersectionGet(x, y) == NULL) {
                printf("%s", " ");
            } else {
                CivResources res =
                    intersectionGetResources(intersectionGet(x, y));
                if (res.brick) {
                    printf("\x1b[31m");
                    printf("B");
                    printf("\x1b[0m");
                }
                if (res.lumber) {
                    printf("\x1b[32m");
                    printf("L");
                    printf("\x1b[0m");
                }
                if (res.ore) {
                    printf("\x1b[90m");
                    printf("O");
                    printf("\x1b[0m");
                }
                if (res.grain) {
                    printf("\x1b[93m");
                    printf("G");
                    printf("\x1b[0m");
                }
                if (res.wool) {
                    printf("\x1b[37m");
                    printf("W");
                    printf("\x1b[0m");
                }
                if (res.trash) {
                    printf("\x1b[33m");
                    printf("0");
                    printf("\x1b[0m");
                }
            }
            printf("\t");
        }
        printf("\n   │\n   │\n");
    }
}
