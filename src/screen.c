#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "screen.h"
#include "logger.h"
#include "keys.h"
#include "terminal.h"


screen_t *create_screen(int max_col, int max_row, int col_postion, int row_position) {
    screen_t *screen = malloc(sizeof(screen_t));
    if (screen == NULL) {
        return NULL;
    }

    // TODO: These should probably result in an error code
    screen->max_col = max_col <= 0 ? 1 : max_col;
    screen->max_row = max_row <= 0 ? 1 : max_row;
    screen->col_position = col_postion <= 0 ? 1 : col_postion;
    screen->row_position = row_position <= 0 ? 1 : row_position;
    screen->cursor = (cursor_t){
        .row = 0,
        .col = 0
    };
    screen->borders = (borders_t){
        .animation = { .state = ANIMATION_NOOP, .current_time = 0.0, .animation_time = 0.0 },
        .top = (border_side_t){
            .buffer = NULL,
            .bg_color = (rgb_t) {.color = 0x000000},
            .fg_color = (rgb_t) {.color = 0x000000},
            .character = 0x00
        },
        .right = (border_side_t){
            .buffer = NULL,
            .bg_color = (rgb_t) {.color = 0x000000},
            .fg_color = (rgb_t) {.color = 0x000000},
            .character = 0x00
        },
        .bottom = (border_side_t){
            .buffer = NULL,
            .bg_color = (rgb_t) {.color = 0x000000},
            .fg_color = (rgb_t) {.color = 0x000000},
            .character = 0x00
        },
        .left = (border_side_t){
            .buffer = NULL,
            .bg_color = (rgb_t) {.color = 0x000000},
            .fg_color = (rgb_t) {.color = 0x000000},
            .character = 0x00
        },
    };
    return screen;
}

void free_screen(screen_t *screen) {
    if (screen == NULL) {
        return;
    }

    // NOTE: This method does not free the inner_buffer

    if (screen->borders.top.buffer != NULL) {
        free(screen->borders.top.buffer);
    }
    if (screen->borders.right.buffer != NULL) {
        free(screen->borders.right.buffer);
    }
    if (screen->borders.bottom.buffer != NULL) {
        free(screen->borders.bottom.buffer);
    }
    if (screen->borders.left.buffer != NULL) {
        free(screen->borders.left.buffer);
    }

    free(screen);
}

int set_screen_border_side(
    screen_t *screen,
    border_side_e border,
    rgb_t bg_color,
    rgb_t fg_color,
    u_int8_t character
) {
    if (screen == NULL) {
        return -1;
    }

    border_side_t *border_side;
    switch (border) {
        // TODO: Maybe this should throw an error?
        case BORDER_LEFT:
            border_side = &screen->borders.left;
            break;
        case BORDER_RIGHT:
            border_side = &screen->borders.right;
            break;
        case BORDER_TOP:
            border_side = &screen->borders.top;
            break;
        case BORDER_BOTTOM:
            border_side = &screen->borders.bottom;
            break;
    }

    border_side->buffer = NULL;

    u_int8_t *buffer;
    switch (border) {
        case BORDER_LEFT:
        case BORDER_RIGHT:
            buffer = calloc(screen->max_row - 1, sizeof(u_int8_t));
            break;
        case BORDER_TOP:
        case BORDER_BOTTOM:
            buffer = calloc(screen->max_col - 1, sizeof(u_int8_t));
            break;
    }

    if (buffer == NULL) {
        return -1;
    }
    border_side->buffer = buffer;

    border_side->character = character;
    border_side->fg_color = fg_color;
    border_side->bg_color = bg_color;

    return 0;
}

int set_screen_animation(
    screen_t *screen,
    double animation_time
) {
    if (screen == NULL) {
        return -1;
    }

    screen->borders.animation.animation_time = animation_time;
    screen->borders.animation.current_time = 0.0;
    screen->borders.animation.state = ANIMATION_RUNNING;
    return 0;
}

int screen_get_row_count(screen_t *screen) {
    if (screen == NULL) {
        log_message("WARNING: screen is a NULL pointer");
        return -1;
    }

    return screen->max_row - (BORDER_WIDTH * 2);
}

int screen_get_column_count(screen_t *screen) {
    if (screen == NULL) {
        log_message("WARNING: screen is a NULL pointer");
        return -1;
    }

    return screen->max_col - (BORDER_WIDTH * 2);
}

void screen_move_cursor(screen_t *screen, direction_e direction) {
    if (screen == NULL) {
        log_message("WARNING: screen is a NULL pointer");
        return;
    }

    // NOTE: Remember that the cursor is relative to the screen
    switch (direction) {
        case UP: {
            if (screen->cursor.row <= 0) {
                break;
            }
            screen->cursor.row -= 1;
            break;
        }
        case DOWN: {
            if (screen->cursor.row + 1 > screen->max_row - 1 - (BORDER_WIDTH * 2)) {
                break;
            }
            screen->cursor.row += 1;
            break;
        }
        case RIGHT: {
            if (screen->cursor.col + 1 > screen->max_col - 1 - (BORDER_WIDTH * 2)) {
                break;
            }
            screen->cursor.col += 1;
            break;
        }
        case LEFT: {
            if (screen->cursor.col <= 0) {
                break;
            }
            screen->cursor.col -= 1;
            break;
        }
    }
    log_message("Cursor at: col %d row %d", screen->cursor.col, screen->cursor.row);
}

int screen_get_absolute_cursor_row(screen_t *screen) {
    if (screen == NULL) {
        log_message("screen_get_absolute_cursor_row: screen ptr is NULL");
        return -1;
    }

    return screen->cursor.row + screen->row_position + BORDER_WIDTH;
}

int screen_get_absolute_cursor_col(screen_t *screen) {
    if (screen == NULL) {
        log_message("screen_get_absolute_cursor_col: screen ptr is NULL");
        return -1;
    }
    return screen->cursor.col + screen->col_position + BORDER_WIDTH;
}

// TODO: Have update return a value that indicates if a draw is required
// 1 = draw, 0 = nodraw, -1 = error
int screen_update(screen_t *screen, key_press_t *key_press, double delta) {
    if (screen == NULL) {
        log_message("screen_update: screen ptr is NULL");
        return -1;
    }

    if (key_press == NULL) {
        log_message("screen_update: key press ptr is NULL");
        return -1;
    }

    // TODO: Need to update the padding buffers if they exist
    // NOTE: Should update the buffer with the correct visual position
    // EG. for the bottom row, the first char should be place at the last item
    // in the buffer. This way the render method doesn't need extra weird logic
    if (
        screen->borders.animation.state == ANIMATION_NOOP
    ) {
        int result = fill_screen_border_buffers(screen);
        if (result < 0) {
            screen->borders.animation.state = ANIMATION_COMPLETE;
            return -1;
        }
        screen->borders.animation.state = ANIMATION_COMPLETE;
        return result;
    } else if (
        screen->borders.animation.state == ANIMATION_RUNNING &&
        screen->borders.animation.current_time < screen->borders.animation.animation_time
    ) {
        screen->borders.animation.current_time += delta;

        int result = update_screen_border_buffers(screen);
        if (result < 0) {
            screen->borders.animation.state = ANIMATION_COMPLETE;
            return -1;
        }

        if (screen->borders.animation.current_time >= screen->borders.animation.animation_time) {
            screen->borders.animation.current_time = screen->borders.animation.animation_time;
            screen->borders.animation.state = ANIMATION_COMPLETE;
        }

        return result;
    }

    if (!key_press->is_special) {
        return 0;
    }

    switch(key_press->key) {
        case UP_ARROW_KEY:
            screen_move_cursor(screen, UP);
            return 1;
        case DOWN_ARROW_KEY:
            screen_move_cursor(screen, DOWN);
            return 1;
        case RIGHT_ARROW_KEY:
            screen_move_cursor(screen, RIGHT);
            return 1;
        case LEFT_ARROW_KEY:
            screen_move_cursor(screen, LEFT);
            return 1;
        default:
            break;
    }
    return 0;
}

int fill_screen_border_buffers(screen_t *screen) {
    if (screen == NULL) {
        log_message("WARNING: update_screen_border_buffers - screen was a NULL ptr");
        return -1;
    }

    for (int row_index = 0; row_index < screen->max_row - 1; ++row_index) {
        if (screen->borders.right.buffer != NULL) {
            screen->borders.right.buffer[row_index] = screen->borders.right.character;
        }
        if (screen->borders.left.buffer != NULL) {
            screen->borders.left.buffer[row_index] = screen->borders.left.character;
        }
    }
    for (int col_index = 0; col_index < screen->max_col - 1; ++col_index) {
        if (screen->borders.top.buffer != NULL) {
            screen->borders.top.buffer[col_index] = screen->borders.top.character;
        }
        if (screen->borders.bottom.buffer != NULL) {
            screen->borders.bottom.buffer[col_index] = screen->borders.bottom.character;
        }
    }
    return 1;
}

int update_screen_border_buffers(screen_t *screen) {
    if (screen == NULL) {
        log_message("WARNING: update_screen_border_buffers - screen was a NULL ptr");
        return -1;
    }

    // Should I check if this could overflow. I'm not sure how to do that
    long long total = (screen->max_col * 2) + (screen->max_row * 2);
    double current = floor(
        total * screen->borders.animation.current_time / screen->borders.animation.animation_time
    );

    // TODO: Check null values for buffers
    u_int8_t *buffer;
    int index;
    int top_max = screen->max_col - 1;
    int right_max = screen->max_col + screen->max_row - 1;
    int bottom_max = screen->max_row + (screen->max_col * 2) - 1;
    if (current <= top_max) {
        buffer = screen->borders.top.buffer;
        index = (int)current;
        if (index > screen->max_col || index < 0) {
            log_message("WARNING: Top border index was out of bounds: %f : %d", current, index);
            return -1;
        }
        if (buffer[index] != screen->borders.top.character) {
            buffer[index] = screen->borders.top.character;
            return 1;
        }
    } else if (
        current > top_max &&
        current <= right_max
    ) {
        buffer = screen->borders.right.buffer;
        index = (int)current - screen->max_col;
        if (index > screen->max_row || index < 0) {
            log_message("WARNING: Right border index was out of bounds: %f : %d", current, index);
            return -1;
        }
        if (buffer[index] != screen->borders.right.character) {
            buffer[index] = screen->borders.right.character;
            return 1;
        }
    } else if (
        current > right_max &&
        current <= bottom_max
    ) {
        buffer = screen->borders.bottom.buffer;
        index = screen->max_col + (right_max - (int)current);
        if (index > screen->max_col || index < 0) {
            log_message("WARNING: bottom border index was out of bounds: %f : %d", current, index);
            return -1;
        }
        if (buffer[index] != screen->borders.bottom.character) {
            buffer[index] = screen->borders.bottom.character;
            return 1;
        }
    } else if (
        current > bottom_max &&
        current < total
    ) {
        buffer = screen->borders.left.buffer;
        index = screen->max_row + (bottom_max - (int)current);
        if (index > screen->max_row || index < 0) {
            log_message("WARNING: Left border index was out of bounds: %f : %d", current, index);
            return -1;
        }
        if (buffer[index] != screen->borders.left.character) {
            buffer[index] = screen->borders.left.character;
            return 1;
        }
    }
    return 0;
}

// TODO: This method could use some refactoring
// NOTE: The desired look is that each border draws in a clock-wise manner without
// any over lapping. Each border should start in a corner.
void screen_draw(screen_t *screen) {
    int index;

    // Move cursor to the start of the screen which will also be the start
    // of the top border
    terminal_cursor_move_to(screen->row_position,  screen->col_position);

    for (index = 0; index < screen->max_col - 1; ++index) {
        if (screen->borders.top.buffer[index] != 0x00) {
            if (index == 0 || screen->borders.top.buffer[index - 1] != screen->borders.top.character) {
                terminal_set_background(&screen->borders.top.bg_color);
                terminal_set_foreground(&screen->borders.top.fg_color);
            }
            fprintf(stdout, "%c", screen->borders.top.buffer[index]);
            continue;
        }
        if (index == 0 || screen->borders.top.buffer[index - 1] != 0x00) {
            terminal_reset();
        }
        fprintf(stdout, "%c", ' ');
    }

    for (index = 0; index < screen->max_row - 1; ++index) {
        terminal_cursor_move_to(
            screen->row_position + index,
            screen->max_col + screen->col_position - 1  // TODO: why is this off by one?
        );
        if (screen->borders.right.buffer[index] != 0x00) {
            if (index == 0 || screen->borders.right.buffer[index - 1] != screen->borders.right.character) {
                terminal_set_background(&screen->borders.right.bg_color);
                terminal_set_foreground(&screen->borders.right.fg_color);
            }
            fprintf(stdout, "%c", screen->borders.right.buffer[index]);
            continue;
        }
        if (index == 0 || screen->borders.right.buffer[index - 1] != 0x00) {
            terminal_reset();
        }
        fprintf(stdout, "%c", ' ');
    }

    terminal_cursor_move_to(
        screen->max_row + screen->row_position - 1,  // TODO: why is this off by one?
        screen->col_position + 1
    );
    for (index = 0; index < screen->max_col - 1; ++index) {
        if (screen->borders.bottom.buffer[index] != 0x00) {
            if (index == 0 || screen->borders.bottom.buffer[index - 1] != screen->borders.bottom.character) {
                terminal_set_background(&screen->borders.bottom.bg_color);
                terminal_set_foreground(&screen->borders.bottom.fg_color);
            }
            fprintf(stdout, "%c", screen->borders.bottom.buffer[index]);
            continue;
        }
        if (index == 0 || screen->borders.bottom.buffer[index - 1] != 0x00) {
            terminal_reset();
        }
        fprintf(stdout, "%c", ' ');
    }

    for (index = 0; index < screen->max_row - 1; ++index) {
        terminal_cursor_move_to(
            screen->row_position + index + 1,
            screen->col_position
        );
        if (screen->borders.left.buffer[index] != 0x00) {
            if (index == 0 || screen->borders.left.buffer[index - 1] != screen->borders.left.character) {
                terminal_set_background(&screen->borders.left.bg_color);
                terminal_set_foreground(&screen->borders.left.fg_color);
            }
            fprintf(stdout, "%c", screen->borders.left.buffer[index]);
            continue;
        }
        if (index == 0 || screen->borders.left.buffer[index - 1] != 0x00) {
            terminal_reset();
        }
        fprintf(stdout, "%c", ' ');
    }

    terminal_reset();
}
