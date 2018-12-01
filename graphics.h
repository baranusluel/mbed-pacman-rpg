#ifndef GRAPHICS_H
#define GRAPHICS_H

void init_sprites();
void add_key_to_player();
void transpose(int *sprite, int *out);
void flip_lr(int *sprite, int *out);
void flip_ud(int *sprite, int *out);
void copy(int *sprite, int *out);

/**
 * Draws the player. This depends on the player state, so it is not a DrawFunc.
 */
void draw_player(int u, int v, int key, int dir);

void draw_ghost(int u, int v, int color, int fleeing);

/**
 * Takes a string image and draws it to the screen. The string is 121 characters
 * long, and represents an 11x11 tile in row-major ordering (across, then down,
 * like a regular multi-dimensional array). The available colors are:
 *      R = Red
 *      Y = Yellow
 *      G = Green
 *      D = Brown ("dirt")
 *      5 = Light grey (50%)
 *      3 = Dark grey (30%)
 *      Any other character is black
 * More colors can be easily added by following the pattern already given.
 */
void draw_img(int u, int v, const char* img);

/**
 * DrawFunc functions. 
 * These can be used as the MapItem draw functions.
 */
void draw_nothing(int u, int v);
void draw_wall(int u, int v);
void draw_dot(int u, int v);
void draw_tree(int u, int v);
void draw_portal(int u, int v);
void draw_prize(int u, int v);
void draw_door(int u, int v);

/**
 * Draw the upper status bar.
 */
void draw_upper_status(int x, int y, int isOmni, int map, int power, int fleeing, int questState);

/**
 * Draw the lower status bar.
 */ 
void draw_lower_status(int map);

/**
 * Draw the border for the map.
 */
void draw_border();

void draw_dead();
void draw_game_over();

#endif // GRAPHICS_H
