#include <stdlib.h>

#include "screen.h"
#include "logger.h"
#include "keys.h"

size_t screen_get_row_index(screen_t *screen) {
    if (screen == NULL) {
        log_message("WARNING: screen is a NULL pointer");
        return 0;
    }

    return screen->cursor.row - (screen->padding.left + screen->padding.right);
}

size_t screen_get_row_count(screen_t *screen) {
    if (screen == NULL) {
        log_message("WARNING: screen is a NULL pointer");
        return 0;
    }

    return screen->max_row - (screen->padding.left + screen->padding.right);
}

size_t screen_get_row_max(screen_t *screen) {
    if (screen == NULL) {
        log_message("WARNING: screen is a NULL pointer");
        return 0;
    }

    return screen->cursor.row - screen->padding.right;
}

size_t screen_get_row_min(screen_t *screen) {
    if (screen == NULL) {
        log_message("WARNING: screen is a NULL pointer");
        return 0;
    }

    return 1 + screen->padding.left;
}

size_t screen_get_column_index(screen_t *screen) {
    if (screen == NULL) {
        log_message("WARNING: screen is a NULL pointer");
        return 0;
    }

    return screen->cursor.col - (screen->padding.top + screen->padding.bottom);
}

size_t screen_get_column_count(screen_t *screen) {
    if (screen == NULL) {
        log_message("WARNING: screen is a NULL pointer");
        return 0;
    }

    return screen->max_col - (screen->padding.top + screen->padding.bottom);
}

size_t screen_get_column_max(screen_t *screen) {
    if (screen == NULL) {
        log_message("WARNING: screen is a NULL pointer");
        return 0;
    }

    return screen->cursor.col - screen->padding.bottom;
}

size_t screen_get_column_min(screen_t *screen) {
    if (screen == NULL) {
        log_message("WARNING: screen is a NULL pointer");
        return 0;
    }

    return 1 + screen->padding.top;
}

void screen_move_cursor(screen_t *screen, direction_e direction) {
    if (screen == NULL) {
        log_message("WARNING: screen is a NULL pointer");
        return;
    }

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
}

void screen_update(screen_t *screen, key_press_t *key_press, double delta) {
    if (screen == NULL) {
        log_message("handle_input: screen ptr is NULL");
        return;
    }

    if (key_press == NULL) {
        log_message("handle_input: key press ptr is NULL");
        return;
    }

    if (!key_press->is_special) {
        return;
    }

    switch(key_press->key) {
        case UP_ARROW_KEY:
            screen_move_cursor(screen, UP);
            break;
        case DOWN_ARROW_KEY:
            screen_move_cursor(screen, DOWN);
            break;
        case RIGHT_ARROW_KEY:
            screen_move_cursor(screen, RIGHT);
            break;
        case LEFT_ARROW_KEY:
            screen_move_cursor(screen, LEFT);
            break;
        default:
            break;
    }
}
