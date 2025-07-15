#pragma once

#include <sys/types.h>
#include <stdbool.h>

#include "keys.h"
#include "terminal.h"

#define BORDER_WIDTH 1

typedef struct Screen screen_t;

typedef enum BORDER_SIDE {
    BORDER_TOP,
    BORDER_RIGHT,
    BORDER_BOTTOM,
    BORDER_LEFT
} border_side_e;

typedef enum ANIMATION_STATE {
    ANIMATION_RUNNING,
    ANIMATION_COMPLETE,
    ANIMATION_NOOP
} animation_state_e;

typedef struct Animation {
    double animation_time;
    double current_time;
    animation_state_e state;
} animation_t;

// TODO: This should be called Border and Padding stuff
// should be something different ... *<:O)8
typedef struct BorderSide {
    rgb_t bg_color;
    rgb_t fg_color;
    int start_index;
    int end_index;
    u_int8_t character;
} border_side_t;

int set_screen_border_side(
    screen_t *screen,
    border_side_e border,
    rgb_t bg_color,
    rgb_t fg_color,
    u_int8_t character
);

int update_screen_border_buffer(screen_t *screen);
int fill_screen_border_buffer(screen_t *screen);

typedef struct Borders {
    animation_t animation;
    border_side_t top;
    border_side_t right;
    border_side_t left;
    border_side_t bottom;
    u_int8_t *buffer;
    int buffer_length;
} borders_t;

int set_screen_animation(
    screen_t *screen,
    double animation_time
);

// NOTE: Cursor should be relative to it's parent screen
typedef struct Cursor {
    int row;
    int col;
} cursor_t;

typedef struct Screen {
    borders_t borders;
    cursor_t cursor;
    int x;
    int width;
    int y;
    int height;
} screen_t;

// Each direction is set to the number (char) that will be placed within the
// stdin buffer when the corresponding arrow key is pressed.
typedef enum DIRECTION {
    UP = 'A',
    DOWN = 'B',
    RIGHT = 'C',
    LEFT = 'D'
} direction_e;

screen_t *create_screen(int x, int width, int y, int height);
void free_screen(screen_t *screen);

// TODO: Should control the border drawing animation by using the delta
// and loading_percent and animation_time
int screen_update(screen_t *screen, key_press_t *key_press, double delta);
void screen_move_cursor(screen_t *screen, direction_e direction);

void screen_draw(screen_t *screen);

int screen_get_row_count(screen_t *screen);
int screen_get_column_count(screen_t *screen);

int screen_get_absolute_cursor_row(screen_t *screen);
int screen_get_absolute_cursor_col(screen_t *screen);

int screen_row_to_absolute(screen_t *screen, int row_index);
int screen_col_to_absolute(screen_t *screen, int col_index);
