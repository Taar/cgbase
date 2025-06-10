#include <stdlib.h>

#include "screen.h"
#include "logger.h"

u_int32_t screen_get_row_index(screen_t * screen) {
    if (screen == NULL) {
        return -1;
    }

    return screen->cursor.row - (screen->padding.left + screen->padding.right);
}

u_int32_t screen_get_row_count(screen_t * screen) {
    if (screen == NULL) {
        return -1;
    }

    return screen->max_row - (screen->padding.left + screen->padding.right);
}

u_int32_t screen_get_row_max(screen_t * screen) {
    if (screen == NULL) {
        return -1;
    }

    return screen->cursor.row - screen->padding.right;
}

u_int32_t screen_get_row_min(screen_t * screen) {
    if (screen == NULL) {
        return -1;
    }

    return 1 + screen->padding.left;
}

u_int32_t screen_get_column_index(screen_t * screen) {
    if (screen == NULL) {
        return -1;
    }

    return screen->cursor.col - (screen->padding.top + screen->padding.bottom);
}

u_int32_t screen_get_column_count(screen_t * screen) {
    if (screen == NULL) {
        return -1;
    }

    return screen->max_col - (screen->padding.top + screen->padding.bottom);
}

u_int32_t screen_get_column_max(screen_t * screen) {
    if (screen == NULL) {
        return -1;
    }

    return screen->cursor.col - screen->padding.bottom;
}

u_int32_t screen_get_column_min(screen_t * screen) {
    if (screen == NULL) {
        return -1;
    }

    return 1 + screen->padding.top;
}

void screen_move_cursor(screen_t *screen, direction_e direction) {
    if (screen == NULL) {
        return;
    }

    log_message(
        "Before: %d %d %x",
        screen->cursor.row,
        screen->cursor.col,
        direction
    );

    switch (direction) {
        case UP: {
            if (screen->cursor.row - 1 <= screen->padding.top) {
                break;
            }
            screen->cursor.row -= 1;
            break;
        }
        case DOWN: {
            if (screen->cursor.row + 1 >= screen->max_row - screen->padding.bottom) {
                break;
            }
            screen->cursor.row += 1;
            break;
        }
        case RIGHT: {
            if (screen->cursor.col + 1 >= screen->max_col - screen->padding.right) {
                break;
            }
            screen->cursor.col += 1;
            break;
        }
        case LEFT: {
            if (screen->cursor.col - 1 <= screen->padding.left) {
                break;
            }
            screen->cursor.col -= 1;
            break;
        }
    }

    log_message(
        "After: %d %d %x",
        screen->cursor.row,
        screen->cursor.col,
        direction
    );
}
