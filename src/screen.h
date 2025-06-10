#pragma once

#include <sys/types.h>

typedef struct Padding {
    u_int8_t left;
    u_int8_t bottom;
    u_int8_t right;
    u_int8_t top;
} padding_t;

typedef struct Cursor {
    u_int32_t row;
    u_int32_t col;
} cursor_t;

typedef struct Screen {
    cursor_t cursor;
    u_int32_t max_row;
    u_int32_t max_col;
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

u_int32_t screen_get_row_index(screen_t *screen);
u_int32_t screen_get_row_count(screen_t *screen);
u_int32_t screen_get_row_max(screen_t *screen);
u_int32_t screen_get_row_min(screen_t *screen);

u_int32_t screen_get_column_index(screen_t *screen);
u_int32_t screen_get_column_count(screen_t *screen);
u_int32_t screen_get_column_max(screen_t *screen);
u_int32_t screen_get_column_min(screen_t *screen);

void screen_move_cursor(screen_t *screen, direction_e direction);
