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


screen_t *create_screen(int x, int width, int y, int height) {
    screen_t *screen = malloc(sizeof(screen_t));
    if (screen == NULL) {
        return NULL;
    }

    // TODO: These should probably result in an error code
    screen->x = x <= 0 ? 1 : x;
    screen->width = width <= 0 ? 1 : width;
    screen->y = y <= 0 ? 1 : y;
    screen->height = height <= 0 ? 1 : height;
    screen->cursor = (cursor_t){
        .row = 0,
        .col = 0
    };
    screen->borders = (borders_t){
        .animation = { .state = ANIMATION_NOOP, .current_time = 0.0, .animation_time = 0.0 },
        .buffer = NULL,
        .top = (border_side_t){
            .bg_color = (rgb_t) {.color = 0x000000},
            .fg_color = (rgb_t) {.color = 0x000000},
            .start_index = 0,
            .end_index = width - 1,
            .character = 0x00
        },
        .right = (border_side_t){
            .bg_color = (rgb_t) {.color = 0x000000},
            .fg_color = (rgb_t) {.color = 0x000000},
            .start_index = width,
            .end_index = width + height - 1,
            .character = 0x00
        },
        .bottom = (border_side_t){
            .bg_color = (rgb_t) {.color = 0x000000},
            .fg_color = (rgb_t) {.color = 0x000000},
            .start_index = width + height,
            .end_index = (width * 2) + height - 1,
            .character = 0x00
        },
        .left = (border_side_t){
            .bg_color = (rgb_t) {.color = 0x000000},
            .fg_color = (rgb_t) {.color = 0x000000},
            .start_index = (width * 2) + height,
            .end_index = (width * 2) + (height * 2) - 1,
            .character = 0x00
        },
    };

    screen->borders.buffer_length = (width * 2) + (height * 2);
    u_int8_t *buffer = calloc(
        screen->borders.buffer_length,
        sizeof(u_int8_t)
    );
    if (buffer == NULL) {
        free(screen);
        return NULL;
    }

    screen->borders.buffer = buffer;

    return screen;
}

void free_screen(screen_t *screen) {
    if (screen == NULL) {
        return;
    }

    // NOTE: This method does not free the inner_buffer

    if (screen->borders.buffer != NULL) {
        free(screen->borders.buffer);
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

    return screen->height - (BORDER_WIDTH * 2);
}

int screen_get_column_count(screen_t *screen) {
    if (screen == NULL) {
        log_message("WARNING: screen is a NULL pointer");
        return -1;
    }

    return screen->width - (BORDER_WIDTH * 2);
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
            if (screen->cursor.row + 1 > screen->height - 1 - (BORDER_WIDTH * 2)) {
                break;
            }
            screen->cursor.row += 1;
            break;
        }
        case RIGHT: {
            if (screen->cursor.col + 1 > screen->width - 1 - (BORDER_WIDTH * 2)) {
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

    return screen->cursor.row + screen->y + BORDER_WIDTH;
}

int screen_get_absolute_cursor_col(screen_t *screen) {
    if (screen == NULL) {
        log_message("screen_get_absolute_cursor_col: screen ptr is NULL");
        return -1;
    }
    return screen->cursor.col + screen->x + BORDER_WIDTH;
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
        int result = fill_screen_border_buffer(screen);
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

        int result = update_screen_border_buffer(screen);
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

int fill_screen_border_buffer(screen_t *screen) {
    if (screen == NULL) {
        log_message("WARNING: fill_screen_border_buffer - screen was a NULL ptr");
        return -1;
    }

    if (screen->borders.buffer == NULL) {
        log_message("WARNING: fill_screen_border_buffer - screen border buffer was a NULL ptr");
        return -1;
    }

    for (int buffer_index = 0; buffer_index < screen->borders.buffer_length; ++buffer_index) {
        u_int8_t character;
        if (
            buffer_index >= screen->borders.top.start_index &&
            buffer_index <= screen->borders.top.end_index
        ) {
             character = screen->borders.top.character;
        } else if (
            buffer_index >= screen->borders.right.start_index &&
            buffer_index <= screen->borders.right.end_index
        ) {
             character = screen->borders.right.character;
        } else if (
            buffer_index >= screen->borders.bottom.start_index &&
            buffer_index <= screen->borders.bottom.end_index
        ) {
             character = screen->borders.bottom.character;
        } else if (
            buffer_index >= screen->borders.left.start_index &&
            buffer_index <= screen->borders.left.end_index
        ) {
             character = screen->borders.left.character;
        } else {
            // This should never happen
            log_message("Warning: FILL - Border index is outside of border start/end indicies");
            return - 1;
        }
        screen->borders.buffer[buffer_index] = character;
    }
    return 1;
}

int update_screen_border_buffer(screen_t *screen) {
    if (screen == NULL) {
        log_message("WARNING: update_screen_border_buffers - screen was a NULL ptr");
        return -1;
    }

    // Should I check if this could overflow. I'm not sure how to do that
    long long total = (screen->width * 2) + (screen->height * 2) - 1;
    double current = floor(
        total * screen->borders.animation.current_time / screen->borders.animation.animation_time
    );

    // TODO: Check null values for buffers
    int index = (int)current;
    u_int8_t character;
    if (
        current >= screen->borders.top.start_index &&
        current <= screen->borders.top.end_index
    ) {
         character = screen->borders.top.character;
    } else if (
        current >= screen->borders.right.start_index &&
        current <= screen->borders.right.end_index
    ) {
         character = screen->borders.right.character;
    } else if (
        current >= screen->borders.bottom.start_index &&
        current <= screen->borders.bottom.end_index
    ) {
         character = screen->borders.bottom.character;
    } else if (
        current >= screen->borders.left.start_index &&
        current <= screen->borders.left.end_index
    ) {
         character = screen->borders.left.character;
    } else {
        // This should never happen
        log_message(
            "Warning: UPDATE - Border index is outside of border start/end indicies - %d %f",
            index,
            current
        );
        return - 1;
    }
    if (screen->borders.buffer[index] == 0x00) {
        screen->borders.buffer[index] = character;
        return 1;
    }
    return 0;
}

// TODO: This method could use some refactoring
// NOTE: The desired look is that each border draws in a clock-wise manner without
// any over lapping. Each border should start in a corner.
// TODO: This isn't working as intended. Need to revisit this.
void screen_draw(screen_t *screen) {
    int index;

    // Move cursor to the start of the screen which will also be the start
    // of the top border
    terminal_cursor_move_to(screen->y,  screen->x);

    for (index = screen->borders.top.start_index; index <= screen->borders.top.end_index; ++index) {
        if (screen->borders.buffer[index] != 0x00) {
            if (
                index == screen->borders.top.start_index ||
                screen->borders.buffer[index - 1] != screen->borders.top.character
            ) {
                terminal_set_background(&screen->borders.top.bg_color);
                terminal_set_foreground(&screen->borders.top.fg_color);
            }
            fprintf(stdout, "%c", screen->borders.buffer[index]);
            continue;
        }
        if (index == screen->borders.top.start_index || screen->borders.buffer[index - 1] != 0x00) {
            terminal_reset();
        }
        fprintf(stdout, "%c", ' ');
    }

    for (index = screen->borders.right.start_index; index <= screen->borders.right.end_index; ++index) {
        terminal_cursor_move_to(
            screen->y + (index - screen->borders.right.start_index),
            screen->width + screen->x - 1  // TODO: why is this off by one?
        );
        if (screen->borders.buffer[index] != 0x00) {
            if (
                index == screen->borders.right.start_index ||
                screen->borders.buffer[index - 1] != screen->borders.right.character
            ) {
                terminal_set_background(&screen->borders.right.bg_color);
                terminal_set_foreground(&screen->borders.right.fg_color);
            }
            fprintf(stdout, "%c", screen->borders.buffer[index]);
            continue;
        }
        if (index == screen->borders.right.start_index || screen->borders.buffer[index - 1] != 0x00) {
            terminal_reset();
        }
        fprintf(stdout, "%c", ' ');
    }

    // NOTE: The rendering doesn't need to follow the animation ... pls stop thinking that it does
    terminal_cursor_move_to(
        (screen->height - 1) + screen->y,  // TODO: why is this off by one?
        screen->x
    );
    for (index = screen->borders.bottom.end_index; index >= screen->borders.bottom.start_index; --index) {
        if (screen->borders.buffer[index] == 0x00) {
            if (index == screen->borders.bottom.end_index || screen->borders.buffer[index + 1] != 0x00) {
                terminal_reset();
            }
            fprintf(stdout, "%c", ' ');
            continue;
        }

        if (
            screen->borders.buffer[index] == screen->borders.bottom.character &&
            screen->borders.buffer[index + 1] != screen->borders.bottom.character
        ) {
            terminal_set_background(&screen->borders.bottom.bg_color);
            terminal_set_foreground(&screen->borders.bottom.fg_color);
        }
        fprintf(stdout, "%c", screen->borders.buffer[index]);
    }
    terminal_reset();

    for (index = screen->borders.left.start_index; index <= screen->borders.left.end_index; ++index) {
        terminal_cursor_move_to(
            screen->y + (screen->borders.left.end_index - index),
            screen->x
        );

        if (screen->borders.buffer[index] == 0x00) {
            if (index == screen->borders.left.start_index || screen->borders.buffer[index - 1] != 0x00) {
                terminal_reset();
            }
            fprintf(stdout, "%c", ' ');
            continue;
        }

        if (
            screen->borders.buffer[index] == screen->borders.left.character &&
            screen->borders.buffer[index - 1] != screen->borders.left.character
        ) {
            terminal_set_background(&screen->borders.left.bg_color);
            terminal_set_foreground(&screen->borders.left.fg_color);
        }
        fprintf(stdout, "%c", screen->borders.buffer[index]);
    }

    terminal_reset();
}
