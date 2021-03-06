// Project includes
#include "globals.h"
#include "hardware.h"
#include "map.h"
#include "graphics.h"
#include "speech.h"
#include <stdlib.h>

// Functions in this file
int get_action(GameInputs inputs);
int update_game(int action);
void npcTalk(int ghost);
void draw_game(int init);
void init_maps();
void init_powerups();
void init_npcs(int map);
void update_npcs();
void handle_npc_collision(int ghost);
int sign(int x);
int main();

/**
 * The main game state. Must include Player locations and previous locations for
 * drawing to work properly. Other items can be added as needed.
 */
struct {
    int x,y;    // Current locations
    int px, py; // Previous locations
    int questState; // 0: start (need power), 1: portal is open, 2: finished quest, 3: has key
    int dir; // Direction (0: right, 1: down, 2: left, 3: up)
    int pdir; // Previous direction
    int isOmni; // Whether player is in omnipotent mode
    int pisOmni;
    int power;
    int ppower;
} Player;

struct Ghost {
    int x, y;
    int px, py;
    int color;
    int dead;
};
static Ghost ghosts[3];
static int ghosts_fleeing;

static int getGhost(int x, int y) {
    for (int i = 0; i < 3; i++) {
        if (!ghosts[i].dead && ghosts[i].x == x && ghosts[i].y == y) {
            return i;
        }
    }
    return -1;
}

static int getGhostP(int x, int y) {
    for (int i = 0; i < 3; i++) {
        if (!ghosts[i].dead && ghosts[i].px == x && ghosts[i].py == y) {
            return i;
        }
    }
    return -1;
}

const char* ghost_msg_1[] = {"Hello Pac-Man! I", "can't help you,", "but you can try", "talking to the", "blue ghost."};
int ghost_msg_1_length = 5;
const char* ghost_msg_2[] = {"Hello Pac-Man!", "You don't have", "enough power to", "start your quest", "yet! Come back", "with 10 power."};
int ghost_msg_2_length = 6;
const char* ghost_msg_3[] = {"Welcome back!", "I am opening", "a portal for you.", "Let's see if", "you live up to", "your name..."};
int ghost_msg_3_length = 6;
const char* ghost_msg_4[] = {"The portal is", "already open!"};
int ghost_msg_4_length = 2;
const char* ghost_msg_5[] = {"Congratulations,", "you won!", "You can now", "get your prize."};
int ghost_msg_5_length = 4;
const char* ghost_msg_6[] = {"You already won", "the game! Just", "go to the exit."};
int ghost_msg_6_length = 3;

const char* door_msg[] = {"You are not", "worthy of the", "prize cherry yet!"};
int door_msg_length = 3;

/**
 * Given the game inputs, determine what kind of update needs to happen.
 * Possible return values are defined below.
 */
#define NO_ACTION 0
#define ACTION_BUTTON 1
#define MENU_BUTTON 2
#define GO_LEFT 3
#define GO_RIGHT 4
#define GO_UP 5
#define GO_DOWN 6
int get_action(GameInputs inputs)
{
    if (inputs.b4)
        return ACTION_BUTTON;

    Player.pisOmni = Player.isOmni;
    if (inputs.b1)
        Player.isOmni = !Player.isOmni;

    Player.pdir = Player.dir;

    if (abs(inputs.ax) > abs(inputs.ay)) {
         if (inputs.ax < -0.2) {
            Player.dir = 2;
            return GO_LEFT;
        } else if (inputs.ax > 0.2) {
            Player.dir = 0;
            return GO_RIGHT;
        }
    } else {
        if (inputs.ay < -0.2) {
            Player.dir = 1;
            return GO_DOWN;
        } else if (inputs.ay > 0.2) {
            Player.dir = 3;
            return GO_UP;
        } 
    }
    return NO_ACTION;
}

/**
 * Update the game state based on the user action. For example, if the user
 * requests GO_UP, then this function should determine if that is possible by
 * consulting the map, and update the Player position accordingly.
 *
 * Return values are defined below. FULL_DRAW indicates that for this frame,
 * draw_game should not optimize drawing and should draw every tile, even if
 * the player has not moved.
 */
#define NO_RESULT 0
#define GAME_OVER 1
#define FULL_DRAW 2
int update_game(int action)
{
    int result = NO_RESULT;

    // Save player previous location before updating
    Player.px = Player.x;
    Player.py = Player.y;
    Player.ppower = Player.power;

    MapItem* item;
    int itemX, itemY;

    // Do different things based on the each action.
    // You can define functions like "go_up()" that get called for each case.
    switch(action)
    {
        case GO_UP:
        {
            item = get_north(Player.x, Player.y);
            if (item && !item->walkable && !Player.isOmni)
                break;
            if (getGhost(Player.x, Player.y-1) >= 0 && !Player.isOmni) {
                handle_npc_collision(getGhost(Player.x, Player.y-1));
                break;
            }
            Player.y--;
            break;
        }
        case GO_LEFT:
        {
            item = get_west(Player.x, Player.y);
            if (item && !item->walkable && !Player.isOmni)
                break;
            if (getGhost(Player.x-1, Player.y) >= 0 && !Player.isOmni) {
                handle_npc_collision(getGhost(Player.x-1, Player.y));
                break;
            }
            Player.x--;
            break;
        }
        case GO_DOWN:
        {
            item = get_south(Player.x, Player.y);
            if (item && !item->walkable && !Player.isOmni)
                break;
            if (getGhost(Player.x, Player.y+1) >= 0 && !Player.isOmni) {
                handle_npc_collision(getGhost(Player.x, Player.y+1));
                break;
            }
            Player.y++;
            break;
        }
        case GO_RIGHT:
        {
            item = get_east(Player.x, Player.y);
            if (item && !item->walkable && !Player.isOmni)
                break;
            if (getGhost(Player.x+1, Player.y) >= 0 && !Player.isOmni) {
                handle_npc_collision(getGhost(Player.x+1, Player.y));
                break;
            }
            Player.x++;
            break;
        }
        case ACTION_BUTTON:
        {
            item = get_north(Player.x, Player.y);
            itemX = Player.x; itemY = Player.y - 1;
            if (!item || item->type == WALL) {
                item = get_south(Player.x, Player.y);
                itemX = Player.x; itemY = Player.y + 1;
            }
            if (!item || item->type == WALL) {
                item = get_west(Player.x, Player.y);
                itemX = Player.x - 1; itemY = Player.y;
            }
            if (!item || item->type == WALL) {
                item = get_east(Player.x, Player.y);
                itemX = Player.x + 1; itemY = Player.y;
            }
            if (item && item->type == PORTAL) {
                PortalData* data = (PortalData*)(item->data);
                set_active_map(data->tm);
                init_npcs(data->tm);
                Player.x = data->tx;
                Player.y = data->ty;
                result = FULL_DRAW;
            } else if (item && item->type == DOOR) {
                if (Player.questState == 3) {
                    map_erase(itemX, itemY);
                } else {
                    long_speech(door_msg, door_msg_length);
                }
                result = FULL_DRAW;
            } else if (get_active_map_index() == 0) {
                int ghost = -1;
                int tmp;
                tmp = getGhost(Player.x+1, Player.y);
                ghost = tmp >= 0 ? tmp : ghost;
                tmp = getGhost(Player.x-1, Player.y);
                ghost = tmp >= 0 ? tmp : ghost;
                tmp = getGhost(Player.x, Player.y+1);
                ghost = tmp >= 0 ? tmp : ghost;
                tmp = getGhost(Player.x, Player.y-1);
                ghost = tmp >= 0 ? tmp : ghost;
                if (ghost < 0)
                    break;
                npcTalk(ghost);
                result = FULL_DRAW;
            }
            break;
        }
        case MENU_BUTTON: break;
        default:        break;
    }
    item = get_here(Player.x, Player.y);
    if (item && item->type == DOT) {
        Player.power++;
        map_erase(Player.x, Player.y);
        if (get_active_map_index() == 1) {
            ghosts_fleeing = 100;
        }
        result = FULL_DRAW;
    } else if (item && item->type == PRIZE) {
        result = GAME_OVER;
    }
    return result;
}

void npcTalk(int ghost) {
    if (ghost == 0 || ghost == 1)
        long_speech(ghost_msg_1, ghost_msg_1_length);
    else if (ghost == 2) {
        if (Player.questState == 0 && Player.power < 10) {
            long_speech(ghost_msg_2, ghost_msg_2_length);
        } else if (Player.questState == 0) {
            Player.questState = 1;
            add_portal(39, 47, 1, 5, 5);
            long_speech(ghost_msg_3, ghost_msg_3_length);
        } else if (Player.questState == 1) {
            long_speech(ghost_msg_4, ghost_msg_4_length);
        } else if (Player.questState == 2) {
            Player.questState = 3;
            long_speech(ghost_msg_5, ghost_msg_5_length);
            add_key_to_player();
        } else if (Player.questState == 3) {
            long_speech(ghost_msg_6, ghost_msg_6_length);
        }
    }
}

/**
 * Entry point for frame drawing. This should be called once per iteration of
 * the game loop. This draws all tiles on the screen, followed by the status
 * bars. If init is zero, this function will optimize drawing by only
 * drawing tiles that have changed from the previous frame.
 */
void draw_game(int init)
{
    // Draw game border first
    if(init) draw_border();

    // Iterate over all visible map tiles
    for (int i = -5; i <= 5; i++) // Iterate over columns of tiles
    {
        for (int j = -4; j <= 4; j++) // Iterate over one column of tiles
        {
            // Here, we have a given (i,j)

            // Compute the current map (x,y) of this tile
            int x = i + Player.x;
            int y = j + Player.y;

            // Compute the previous map (px, py) of this tile
            int px = i + Player.px;
            int py = j + Player.py;

            // Compute u,v coordinates for drawing
            int u = (i+5)*11 + 3;
            int v = (j+4)*11 + 15;

            // Figure out what to draw
            DrawFunc draw = NULL;
            if (i == 0 && j == 0) // Only draw the player on init
            {
                if (init || Player.dir != Player.pdir)
                    draw_player(u, v, Player.questState, Player.dir);
                // Never overwrite (0,0) with anything else
                continue;
            }
            else if (x >= 0 && y >= 0 && x < map_width() && y < map_height()) // Current (i,j) in the map
            {
                int ghost;
                if ((ghost = getGhost(x, y)) >= 0) {
                    if (init || x != px || y != py
                        || ghosts[ghost].x != ghosts[ghost].px || ghosts[ghost].y != ghosts[ghost].py) {
                        draw_ghost(u, v, ghosts[ghost].color, ghosts_fleeing);
                    }
                    continue;
                } else if ((ghost = getGhost(px, py)) >= 0 && (init || x != px || y != py)) {
                    draw = draw_nothing;
                } else if ((ghost = getGhostP(x, y)) >= 0 && (init
                            || ghosts[ghost].x != ghosts[ghost].px || ghosts[ghost].y != ghosts[ghost].py)) {
                    draw = draw_nothing;
                } else if ((ghost = getGhostP(px, py)) >= 0 && (init || x != px || y != py
                            || ghosts[ghost].x != ghosts[ghost].px || ghosts[ghost].y != ghosts[ghost].py)) {
                    draw = draw_nothing;
                }

                MapItem* curr_item = get_here(x, y);
                MapItem* prev_item = get_here(px, py);
                if (init || curr_item != prev_item) // Only draw if they're different
                {
                    if (curr_item) // There's something here! Draw it
                    {
                        draw = curr_item->draw;
                    }
                    else // There used to be something, but now there isn't
                    {
                        draw = draw_nothing;
                    }
                }
            }
            else if (init) // If doing a full draw, but we're out of bounds, draw the walls.
            {
                draw = draw_wall;
            }

            // Actually draw the tile
            if (draw) draw(u, v);
        }
    }

    // Draw status bars
    if (Player.x != Player.px || Player.y != Player.py
        || Player.isOmni != Player.pisOmni || Player.power != Player.ppower
        || ghosts_fleeing % 10 == 9)
        draw_upper_status(Player.x, Player.y, Player.isOmni, get_active_map_index(), Player.power, ghosts_fleeing, Player.questState);
    if (init)
        draw_lower_status(get_active_map_index());
}


/**
 * Initialize the main world map. Add walls around the edges, interior chambers,
 * and plants in the background so you can see motion.
 */
void init_maps()
{
    // "Random" power dots
    set_active_map(0);
    //for(int i = 1; i < map_area(); i += rand() % 50 + 30)
    //{
    //    add_tree(i % map_width(), (i / map_width()) % map_height());
    //}
    for (int x = 5; x < 50; x += 5) {
        for (int y = 5; y < 50; y += 5) {
            add_tree(x, y);
        }
    }
    for(int i = map_width() + 3; i < map_area(); i += 130)
    {
        add_dot(i % map_width(), i / map_width());
    }
    add_wall(0, 0, HORIZONTAL, 1);
    //add_wall(0,              0,              HORIZONTAL, map_width());
    //add_wall(0,              map_height()-1, HORIZONTAL, map_width());
    //add_wall(0,              0,              VERTICAL,   map_height());
    //add_wall(map_width()-1,  0,              VERTICAL,   map_height());
    add_wall(43, 1, VERTICAL, 5);
    add_wall(43, 6, HORIZONTAL, 3);
    add_wall(47, 6, HORIZONTAL, 3);
    add_door(46, 6);
    add_prize(46, 2);
    pc.printf("Map 0:\r\n");
    print_map();

    set_active_map(1);
    add_wall(0, 0, HORIZONTAL, 1);
    //add_wall(0,              0,              HORIZONTAL, map_width());
    //add_wall(0,              map_height()-1, HORIZONTAL, map_width());
    //add_wall(0,              0,              VERTICAL,   map_height());
    //add_wall(map_width()-1,  0,              VERTICAL,   map_height());

    add_wall(11, 1, VERTICAL, 2);
    add_wall(11, 20, VERTICAL, 2);
    add_wall(1, 11, HORIZONTAL, 2);
    add_wall(20, 10, HORIZONTAL, 2);
    add_wall(20, 12, HORIZONTAL, 2);

    add_wall(8, 4, HORIZONTAL, 2);
    add_wall(13, 4, HORIZONTAL, 2);
    add_wall(8, 18, HORIZONTAL, 2);
    add_wall(13, 18, HORIZONTAL, 2);
    add_wall(11, 4, VERTICAL, 4);
    add_wall(11, 15, VERTICAL, 4);

    add_wall(2, 2, HORIZONTAL, 3);
    add_wall(6, 2, HORIZONTAL, 4);
    add_wall(13, 2, HORIZONTAL, 4);
    add_wall(18, 2, HORIZONTAL, 3);
    add_wall(2, 20, HORIZONTAL, 3);
    add_wall(6, 20, HORIZONTAL, 4);
    add_wall(13, 20, HORIZONTAL, 4);
    add_wall(18, 20, HORIZONTAL, 3);

    add_wall(6, 4, VERTICAL, 6);
    add_wall(8, 6, HORIZONTAL, 2);
    add_wall(16, 4, VERTICAL, 6);
    add_wall(13, 6, HORIZONTAL, 2);
    add_wall(6, 13, VERTICAL, 6);
    add_wall(8, 16, HORIZONTAL, 2);
    add_wall(16, 13, VERTICAL, 6);
    add_wall(13, 16, HORIZONTAL, 2);

    add_wall(3, 4, VERTICAL, 3);
    add_wall(3, 8, VERTICAL, 2);
    add_wall(19, 4, VERTICAL, 3);
    add_wall(19, 8, VERTICAL, 1);
    add_wall(3, 16, VERTICAL, 3);
    add_wall(3, 13, VERTICAL, 2);
    add_wall(19, 16, VERTICAL, 3);
    add_wall(19, 14, VERTICAL, 1);

    add_wall(8, 9, HORIZONTAL, 7);
    add_wall(8, 13, HORIZONTAL, 7);
    add_wall(8, 10, VERTICAL, 3);
    add_wall(14, 10, VERTICAL, 3);

    add_wall(4, 11, HORIZONTAL, 3);
    add_wall(16, 11, HORIZONTAL, 3);

    init_powerups();

    pc.printf("Map 1:\r\n");
    print_map();
}

void init_powerups() {
    add_dot(3, 21);
    add_dot(19, 21);
    add_dot(19, 1);
}

void init_npcs(int map) {
    ghosts[0].color = 0;
    ghosts[1].color = 1;
    ghosts[2].color = 2;
    ghosts[0].dead = 0;
    ghosts[1].dead = 0;
    ghosts[2].dead = 0;
    ghosts_fleeing = 0;
    if (map == 0) {
        ghosts[0].x = 7;
        ghosts[0].y = 6;
        ghosts[0].px = 7;
        ghosts[0].py = 6;
        ghosts[1].x = 19;
        ghosts[1].y = 15;
        ghosts[1].px = 19;
        ghosts[1].py = 15;
        ghosts[2].x = 42;
        ghosts[2].y = 47;
        ghosts[2].px = 42;
        ghosts[2].py = 47;
    } else if (map == 1) {
        ghosts[0].x = 5;
        ghosts[0].y = 17;
        ghosts[0].px = 5;
        ghosts[0].py = 17;
        ghosts[1].x = 17;
        ghosts[1].y = 5;
        ghosts[1].px = 17;
        ghosts[1].py = 5;
        ghosts[2].x = 17;
        ghosts[2].y = 17;
        ghosts[2].px = 17;
        ghosts[2].py = 17;
    }
}

void update_npcs() {
    int dx, dy = 0;
    int newx, newy = 0;
    for (int i = 0; i < 3; i++) {
        if (ghosts[i].dead)
            continue;
        ghosts[i].px = ghosts[i].x;
        ghosts[i].py = ghosts[i].y;
        dx = Player.x - ghosts[i].x;
        dx = ghosts_fleeing ? -dx : dx;
        dy = Player.y - ghosts[i].y;
        dy = ghosts_fleeing ? -dy : dy;
        newx = ghosts[i].x + sign(dx);
        if (get_here(newx, ghosts[i].y) || getGhost(newx, ghosts[i].y) >= 0)
            newx = -1;
        newy = ghosts[i].y + sign(dy);
        if (get_here(ghosts[i].x, newy) || getGhost(ghosts[i].x, newy) >= 0)
            newy = -1;

        if (newx >= 0 && newy < 0)
            ghosts[i].x = newx;
        else if (newx < 0 && newy >= 0)
            ghosts[i].y = newy;
        else if (newx >= 0 && newy >= 0) {
            if (abs(dx) > abs(dy)) {
                ghosts[i].x = newx;
            } else {
                ghosts[i].y = newy;
            }
        }
        
        if (ghosts[i].x == Player.x && ghosts[i].y == Player.y) {
            handle_npc_collision(i);
        }
    }
}

void handle_npc_collision(int ghost) {
    if (get_active_map_index() != 1)
        return;
    if (!ghosts_fleeing) {
        draw_dead();
        init_npcs(1);
        Player.x = Player.y = 5;
        init_powerups();
    } else {
        ghosts[ghost].dead = 1;
        int anyAlive = 0;
        for (int i = 0; i < 3; i++) {
            if (!ghosts[i].dead)
                anyAlive = 1;
        }
        if (!anyAlive) {
            add_portal(21, 11, 0, 38, 47);
            Player.questState = 2;
        }
    }
    draw_game(1);
}

int sign(int x) {
    return (x > 0) - (x < 0);
}

/**
 * Program entry point! This is where it all begins.
 * This function orchestrates all the parts of the game. Most of your
 * implementation should be elsewhere - this holds the game loop, and should
 * read like a road map for the rest of the code.
 */
int main()
{
    // First things first: initialize hardware
    ASSERT_P(hardware_init() == ERROR_NONE, "Hardware init failed!");

    // Initialize the maps
    maps_init();
    init_maps();
    init_sprites();

    // Initialize game state
    set_active_map(0);
    init_npcs(0);
    Player.x = Player.y = 5;
    Player.questState = Player.dir = Player.pdir = Player.isOmni = 0;

    // Initial drawing
    draw_game(true);

    int loop_cntr = 0;

    // Main game loop
    while(1)
    {
        // Timer to measure game update speed
        Timer t; t.start(); // Does t need to be declared inside loop?

        // Actuall do the game update:
        // 1. Read inputs
        // 2. Determine action (get_action)
        // 3. Update game (update_game)
        // 3b. Check for game over
        // 4. Draw frame (draw_game)
        GameInputs inputs = read_inputs();
        int action = get_action(inputs);
        int result = update_game(action);

        if (ghosts_fleeing)
            ghosts_fleeing--;
        if (get_active_map_index() == 1 && loop_cntr == 0)
            update_npcs();

        draw_game(result);
        if (result == GAME_OVER)
            break;

        loop_cntr++;
        if (loop_cntr > 2)
            loop_cntr = 0;

        // 5. Frame delay
        t.stop();
        int dt = t.read_ms();
        if (dt < 100) wait_ms(100 - dt); // Could we set frame time shorter than 100 ms?
    }
    draw_game_over();
}
