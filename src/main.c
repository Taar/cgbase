#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include "database.h"
#include "keys.h"
#include "terminal.h"
#include "screen.h"
#include "logger.h"

int main (int argc, char *argv[]) {
    int error = init_logging("application.log");
    if (error < 0) {
        // TODO: Send message to stderr?
        exit(-1);
    }
    log_message("STARTING ...");

    // Need to save the current terminal settings so that they
    // can be restored after the application closes
    struct termios old_term;
    tcgetattr(0, &old_term);

    struct termios term;
    cfmakeraw(&term);
    term.c_iflag = 0;
    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &term);

    terminal_cursor_home();
    terminal_clear_screen();

    rgb_t color_pink = new_color(0xFF77FF);
    rgb_t color_blue = new_color(0x1133FF);

    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    terminal_hide_cursor();
    fflush(stdout);

    static size_t max_col = 40;
    static size_t max_row = 20;

    screen_t *screen = create_screen(
        max_col,
        max_row,
        1,
        1
    );
    if (screen == NULL) {
        // TODO: need to handle this case
    }
    set_screen_animation(screen, 500.0);
    set_screen_border_side(
        screen,
        BORDER_TOP,
        color_pink,
        color_blue,
        ' '
    );
    set_screen_border_side(
        screen,
        BORDER_RIGHT,
        color_pink,
        color_blue,
        '!'
    );
    set_screen_border_side(
        screen,
        BORDER_BOTTOM,
        color_pink,
        color_blue,
        '#'
    );
    set_screen_border_side(
        screen,
        BORDER_LEFT,
        color_pink,
        color_blue,
        '-'
    );

    database_t *database = new_database(
        screen_get_row_count(screen),
        screen_get_column_count(screen)
    );
    if (database == NULL) {
        log_message("database is a NULL ptr");
        // TODO: need to handle this case
    }
    log_message("database rows: %d", database->rows);
    log_message("database columns: %d", database->columns);

    terminal_cursor_move_to(screen->cursor.row, screen->cursor.col);
    fflush(stdout);

    key_code_t *root = create_key_code_tree();
    log_message("ROOT: %d - %d", root->capacity, root->size);

    // Need to flush stdin just in case the user pressed a button while the
    // animation was running
    fflush(stdin);

    key_press_t *key_press = malloc(sizeof(key_press_t));
    if (key_press == NULL) {
        log_message("key_press is a NULL ptr");
        // TODO: handle this case ...
    }
    key_press->is_special = true;
    key_press->key = NOOP_KEY;

    clock_t last_render = clock();
    clock_t now = last_render;

    double delta = 0.0;

    bool running = true;
    bool should_redraw = false;

    while (running) {
        now = clock();
        delta = (now - last_render) / (CLOCKS_PER_SEC / 1000.0);
        last_render = now;

        handle_input(key_press, root);

        should_redraw = false;

        if (key_press->is_special && key_press->key == F1_KEY) {
            running = false;
            continue;
        } else if (!key_press->is_special) {
            database_update_at(
                database,
                screen->cursor.row,
                screen->cursor.col,
                key_press->key
            );
            screen_move_cursor(screen, RIGHT);
            should_redraw = true;
        }

        // TODO: Could the update functions return a value to let
        // the rest of the project know that it should draw?
        // If there are no update then there is no reason to draw?
        int result = screen_update(screen, key_press, delta);
        if (result < 0) {
            log_message("Error during screen update");
            break;
        } else if (result > 0) {
            should_redraw = true;
        }

        if (should_redraw) {
            terminal_hide_cursor();

            screen_draw(screen);

            terminal_set_foreground(&color_pink);

            // TODO: Move this to its own function
            // TODO: Used absolute screen functions to help draw
            for (int row_index = 0; row_index < database->rows; ++row_index) {
                // NOTE: only need to move the cursor to the beginning of the line
                // since adding a new character to the buffer will move the cursor
                // to the right automatically
                terminal_cursor_move_to(
                    screen->row_position + 1 + row_index,
                    screen->col_position + 1
                );
                u_int8_t *line = database->lines[row_index];
                if (line == NULL) {
                    log_message("LINE NULL at %d", row_index);
                    break;
                }
                for (int col_index = 0; col_index < database->columns; ++col_index) {
                    u_int8_t character = line[col_index];
                    if (character == 0x0) {
                        character = ' ';
                    }
                    // Don't need to move the cursor after each character since it will
                    // automatically move to the right. Just need to re-position it for
                    // each row.
                    fprintf(stdout, "%c", character);
                }
            }

            terminal_cursor_move_to(
                screen_get_absolute_cursor_row(screen),
                screen_get_absolute_cursor_col(screen)
            );
            terminal_unhide_cursor();
            fflush(stdout);
        }
    }

    // Do I need to free this if the program is going to close anyways?
    // Won't the OS release that memory? Is it good hygiene to do it anyways?
    free_screen(screen);
    free(key_press);
    free_database(database);
    free_key_code(root);

    // Reset everything and clear the screen before restoring the old
    // terminal. Just makes the TUI feel less jank
    terminal_reset();
    terminal_cursor_home();
    terminal_clear_screen();
    fflush(stdout);

    // Need to make sure to close the log file
    close_logging();

    // IMPORTANT: Need to restore the terminal to the original settings
    tcsetattr(0, TCSANOW, &old_term);
    return 0;
}
