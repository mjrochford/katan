#include <stdbool.h>
#include <stdio.h>
#include "rules.h"
#include "raylib.h"

typedef struct Board {
    const size_t nTiles;
    TerrainTile *tiles;
} Board;


#define N_TILES 19
static TerrainTile tiles[N_TILES];

#define INTERSECTIONS_HEIGHT 12
#define INTERSECTIONS_WIDTH 11
static Intersection *intersections[INTERSECTIONS_HEIGHT][INTERSECTIONS_WIDTH];
Path *newPath(Intersection *a, Intersection *b, Civilization *owner);

typedef struct Point2D {
    size_t x, y;
} Point2D;

const Point2D tileHeadLocations[N_TILES] = {
    {3, 0},
    {7, 0},
    {5, 0},

    {2, 2},
    {4, 2},
    {6, 2},
    {8, 2},

    {1, 4},
    {3, 4},
    {5, 4},
    {7, 4},
    {9, 4},

    {2, 6},
    {4, 6},
    {6, 6},
    {8, 6},

    {3, 8},
    {5, 8},
    {7, 8},
};

typedef enum Result {
    EXISTS = 1,
    OK = 0,
} AddResult;

#define INTERSECTION_STACK_SIZE 255
static Intersection intersectionStack[INTERSECTION_STACK_SIZE];
static size_t intersectionStackPointer = 0;
Intersection *addIntersection(size_t x, size_t y, Civilization *owner) {
    if (intersectionStackPointer >= INTERSECTION_STACK_SIZE) {
        fprintf(stderr, "ERROR: out of game intersection memory");
        exit(1);
    }

    if (intersections[y][x] != NULL) {
        return intersections[y][x];
    }

    intersectionStack[intersectionStackPointer] = (Intersection){.owner = owner};
    intersections[y][x] = &intersectionStack[intersectionStackPointer];
    intersectionStackPointer += 1;

    return intersections[y][x];
}

ResourceType getResourceType(Intersection *intersection);

typedef struct USizeList {
    size_t data;
    struct USizeList *next;
    struct USizeList *prev;
} USizeList;


void uSizeListPrint(USizeList *head);
USizeList *uSizeListCreate(size_t *arr, size_t arrLen);
void uSizeListDestroy(USizeList *head);
USizeList *uSizeListSlice(USizeList *head, size_t index, size_t len);
size_t uSizeListLen(USizeList *list);
size_t uSizeListTakeRandom(USizeList *list);

USizeList *uSizeListCreate(size_t *arr, size_t arrLen) {
    if (arrLen == 0) {
        return NULL;
    }

    USizeList *head = MemAlloc(sizeof(USizeList));
    for (int i = arrLen - 1; i > 0; i--) {
        head->data = arr[i];
        head->prev = MemAlloc(sizeof(USizeList));
        head->prev->next = head;
        head = head->prev;
    }

    head->data = arr[0];
    return head;
}

void uSizeListDestroy(USizeList *head) {
    if (head->next) {
        uSizeListDestroy(head->next);
    }

    MemFree(head);

}

USizeList *uSizeListSlice(USizeList *head, size_t index, size_t len) {
    if (len == 0) {
        return NULL;
    }

    if (index == 0) { // TODO head should not carry data should just be a ptr
        USizeList *list = head;
        for (size_t i = 0; i < len; i++) {
            list = list->next;
        }
        USizeList *tail = list;
        tail->next = NULL;
        list->prev = NULL;
        return tail;
    }

    USizeList *innerHead = head;
    for (size_t i = 0; i < index; i++) {
        innerHead = innerHead->next;
    }

    USizeList *innerTail = innerHead;
    for (size_t i = 0; i < len - 1; i++) {
        innerTail = innerTail->next;
    }

    if (innerHead->next == NULL) {
        innerHead->prev->next = NULL;
        innerHead->prev = NULL;
        return innerHead;
    }

    innerHead->prev->next = innerTail->next;
    innerTail->next->prev = innerHead->prev;
    innerTail->next = NULL;
    innerHead->prev = NULL;

    return innerHead;
}

size_t uSizeListLen(USizeList *list) { 
    int count = 0;
    while (list) { 
        list = list->next;
        count++;
    }
    return count;
}

size_t uSizeListTakeRandom(USizeList *list) {
    size_t listLen = uSizeListLen(list);

    size_t index = GetRandomValue(0, listLen - 1);
    return uSizeListSlice(list, index, 1)->data;
}

/* TerrainType pickTerrainType(USizeList *options) { */
/* } */

// See Illustration Q in Catan almanac
const size_t TILE_PROB_SEQUENCE[] = {5, 2, 6, 3, 8, 10, 9, 12, 11, 4, 8, 10, 9, 4, 5, 6, 3, 11};

void initalizeGameBoard() {
    for (size_t i = 0; i < N_TILES; i++) {
        Point2D headLocation =  tileHeadLocations[i];
        size_t x = headLocation.x, y = headLocation.y;

        tiles[i] = (TerrainTile){ 
            .terrainType = FOREST,
                .rollNumber = TILE_PROB_SEQUENCE[i], // desert tile needs special case
                .intersections = {
                    addIntersection(x + 0, y + 0, NULL),
                    addIntersection(x + 1, y + 1, NULL),
                    addIntersection(x + 1, y + 2, NULL),
                    addIntersection(x + 0, y + 3, NULL),
                    addIntersection(x - 1, y + 2, NULL),
                    addIntersection(x - 1, y + 1, NULL),
                },
        };
    }
}

void uSizeListPrint(USizeList *head) {
    printf("%ld", head->data);
    while (head->next) {
        printf(" -> %ld", head->next->data);
        head = head->next;
    }
    printf("\n");
}

void printGameBoard() {
    for (int y = 0; y < INTERSECTIONS_HEIGHT; y++) {
        for (int x = 0; x < INTERSECTIONS_WIDTH; x++) {
            if (intersections[y][x] == NULL) {
                printf("%s", " ");
            } else {
                printf("%d", getResourceType(intersections[y][x]));
            }
            printf("\t");
        }
        printf("\n\n\n");
    }

    TerrainType tileOptions[] = {FOREST, FOREST, FOREST, FOREST,
        PASTURE, PASTURE, PASTURE, PASTURE, 
        FIELDS, FIELDS, FIELDS, FIELDS,
        HILLS, HILLS, HILLS,
        MOUNTAINS, MOUNTAINS, MOUNTAINS,
        DESERT };
    size_t tempList[N_TILES];
    for (int i = 0; i < N_TILES; i++) tempList[i] = (size_t) tileOptions[i];

    USizeList *options = uSizeListCreate(tempList, N_TILES);

    while (options->next) {
        uSizeListPrint(options);
        printf("%ld\n", uSizeListTakeRandom(options));
    }
}

// DOESNT ACTAULLY WORK
//
// assumes a intersection only has one resource type when it could have 3
ResourceType getResourceType(Intersection *intersection) {
    for (size_t i = 0; i < N_TILES; i++) {
        TerrainTile tile = tiles[i];
        for (size_t j = 0; j < 6; j++) {
            if (tile.intersections[j] == intersection) {
                // This cast only works because we defined both enums to have the same values
                return (ResourceType) tile.terrainType;
            }
        }
    }
    return TRASH;
}

#define PATH_STACK_SIZE 255
static Path pathMemory[PATH_STACK_SIZE];
static size_t pathStackHead = 0;
Path *newPath(Intersection *a, Intersection *b, Civilization *owner) {
    if (pathStackHead >= PATH_STACK_SIZE) {
        fprintf(stderr, "ERROR: out of game path memory");
        exit(1);
    }
    Path *nPath = &pathMemory[pathStackHead++];
    nPath->owner = owner; 
    nPath->aIntersection = a;
    nPath->bIntersection = b;

    return &pathMemory[pathStackHead++];
}
