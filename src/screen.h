#pragma once

#include <sys/types.h>

#include "keys.h"
#include "terminal.h"

typedef struct Padding {
    rgb_t top_color;
    rgb_t right_color;
    rgb_t bottom_color;
    rgb_t left_color;
    int top;
    int right;
    int bottom;
    int left;
} padding_t;

typedef struct Cursor {
    int row;
    int col;
} cursor_t;

typedef struct Screen {
    double animation_time;
    double current_time;
    cursor_t cursor;
    int max_row;
    int max_col;
    padding_t padding;
} screen_t;

// Each direction is set to the number (char) that will be placed within the
// stdin buffer when the corresponding arrow key is pressed.
typedef enum DIRECTION {
    UP = 'A',
    DOWN = 'B',
    RIGHT = 'C',
    LEFT = 'D'
} direction_e;

// TODO: Should control the border drawing animation by using the delta
// and loading_percent and animation_time
void screen_update(screen_t *screen, key_press_t *key_press, double delta);
void screen_move_cursor(screen_t *screen, direction_e direction);

void screen_draw(screen_t *screen);

size_t screen_get_row_index(screen_t *screen);
size_t screen_get_row_count(screen_t *screen);
size_t screen_get_row_max(screen_t *screen);
size_t screen_get_row_min(screen_t *screen);

size_t screen_get_column_index(screen_t *screen);
size_t screen_get_column_count(screen_t *screen);
size_t screen_get_column_max(screen_t *screen);
size_t screen_get_column_min(screen_t *screen);
