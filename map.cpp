#include "map.h"

#include "globals.h"
#include "graphics.h"

/**
 * The Map structure. This holds a HashTable for all the MapItems, along with
 * values for the width and height of the Map.
 */
struct Map {
    HashTable* items;
    int w, h;
};

#define MAIN_MAP_WIDTH    50
#define MAIN_MAP_HEIGHT   50
#define MAIN_MAP_BUCKETS 100
#define QUEST_MAP_WIDTH   23
#define QUEST_MAP_HEIGHT  23
#define QUEST_MAP_BUCKETS 50

/**
 * Storage area for the maps.
 * This is a global variable, but can only be access from this file because it
 * is static.
 */
static Map map[2];
static int active_map;

/**
 * The first step in HashTable access for the map is turning the two-dimensional
 * key information (x, y) into a one-dimensional unsigned integer.
 * This function should uniquely map (x,y) onto the space of unsigned integers.
 */
static unsigned XY_KEY(int X, int Y) {
    // Fold wall coordinates to same key for memory savings
    if (X == 0 || X == map_width() - 1 || Y == 0 || Y == map_height() - 1)
        return 0;
    return Y * map_width() + X;
}

/**
 * This is the hash function actually passed into createHashTable. It takes an
 * unsigned key (the output of XY_KEY) and turns it into a hash value (some
 * small non-negative integer).
 */
unsigned main_map_hash(unsigned key)
{
    return key % MAIN_MAP_BUCKETS;
}
unsigned quest_map_hash(unsigned key)
{
    return key % QUEST_MAP_BUCKETS;
}

void maps_init()
{
    map[0].items = createHashTable(main_map_hash, MAIN_MAP_BUCKETS);
    map[0].w = MAIN_MAP_WIDTH;
    map[0].h = MAIN_MAP_HEIGHT;
    map[1].items = createHashTable(quest_map_hash, QUEST_MAP_BUCKETS);
    map[1].w = QUEST_MAP_WIDTH;
    map[1].h = QUEST_MAP_HEIGHT;
    active_map = 0;
}

Map* get_active_map()
{
    return &(map[active_map]);
}

int get_active_map_index() {
    return active_map;
}

Map* set_active_map(int m)
{
    active_map = m;
    return get_active_map();
}

void print_map()
{
    // As you add more types, you'll need to add more items to this array.
    char lookup[] = {'W', 'O', '.', 'X', 'p', 'D'};
    for(int y = 0; y < map_height(); y++)
    {
        for (int x = 0; x < map_width(); x++)
        {
            MapItem* item = get_here(x,y);
            if (item) pc.printf("%c", lookup[item->type]);
            else pc.printf(" ");
        }
        pc.printf("\r\n");
    }
}

int map_width()
{
    return get_active_map()->w;
}

int map_height()
{
    return get_active_map()->h;
}

int map_area()
{
    return map_width() * map_height();
}

MapItem* get_north(int x, int y)
{
    return get_here(x, y-1);
}

MapItem* get_south(int x, int y)
{
    return get_here(x, y+1);
}

MapItem* get_east(int x, int y)
{
    return get_here(x+1, y);
}

MapItem* get_west(int x, int y)
{
    return get_here(x-1, y);
}

MapItem* get_here(int x, int y)
{
    return (MapItem*) getItem(get_active_map()->items, XY_KEY(x, y));
}

void map_erase(int x, int y)
{
    MapItem* item = get_here(x, y);
    if (item && item->data)
        free(item->data);
    deleteItem(get_active_map()->items, XY_KEY(x, y));
}

void add_wall(int x, int y, int dir, int len)
{
    for(int i = 0; i < len; i++)
    {
        MapItem* w1 = (MapItem*) malloc(sizeof(MapItem));
        if (!w1)
            pc.printf("OUT OF MEMORY");
        w1->type = WALL;
        w1->draw = draw_wall;
        w1->walkable = false;
        w1->data = NULL;
        unsigned key = (dir == HORIZONTAL) ? XY_KEY(x+i, y) : XY_KEY(x, y+i);
        void* val = insertItem(get_active_map()->items, key, w1);
        if (val) free(val); // If something was already there, free it
    }
}

void add_dot(int x, int y)
{
    MapItem* w1 = (MapItem*) malloc(sizeof(MapItem));
    if (!w1)
        pc.printf("OUT OF MEMORY");
    w1->type = DOT;
    w1->draw = draw_dot;
    w1->walkable = true;
    w1->data = NULL;
    void* val = insertItem(get_active_map()->items, XY_KEY(x, y), w1);
    if (val) free(val); // If something was already there, free it
}

void add_tree(int x, int y)
{
    MapItem* w1 = (MapItem*) malloc(sizeof(MapItem));
    if (!w1)
        pc.printf("OUT OF MEMORY");
    w1->type = TREE;
    w1->draw = draw_tree;
    w1->walkable = true;
    w1->data = NULL;
    void* val = insertItem(get_active_map()->items, XY_KEY(x, y), w1);
    if (val) free(val); // If something was already there, free it
}

void add_portal(int x, int y, int tm, int tx, int ty)
{
    MapItem* w1 = (MapItem*) malloc(sizeof(MapItem));
    if (!w1)
        pc.printf("OUT OF MEMORY");
    w1->type = PORTAL;
    w1->draw = draw_portal;
    w1->walkable = false;
    PortalData* w2 = (PortalData*) malloc(sizeof(PortalData));
    if (!w2)
        pc.printf("OUT OF MEMORY");
    w2->tm = tm;
    w2->tx = tx;
    w2->ty = ty;
    w1->data = w2;
    void* val = insertItem(get_active_map()->items, XY_KEY(x, y), w1);
    if (val) free(val); // If something was already there, free it
}

void add_prize(int x, int y)
{
    MapItem* w1 = (MapItem*) malloc(sizeof(MapItem));
    if (!w1)
        pc.printf("OUT OF MEMORY");
    w1->type = PRIZE;
    w1->draw = draw_prize;
    w1->walkable = true;
    w1->data = NULL;
    void* val = insertItem(get_active_map()->items, XY_KEY(x, y), w1);
    if (val) free(val); // If something was already there, free it
}

void add_door(int x, int y)
{
    MapItem* w1 = (MapItem*) malloc(sizeof(MapItem));
    if (!w1)
        pc.printf("OUT OF MEMORY");
    w1->type = DOOR;
    w1->draw = draw_door;
    w1->walkable = false;
    w1->data = NULL;
    void* val = insertItem(get_active_map()->items, XY_KEY(x, y), w1);
    if (val) free(val); // If something was already there, free it
}
