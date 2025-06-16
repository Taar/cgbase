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

    /*
     * TODO:
     * Intro animation stuff.
     * Would be better to avoid using usleep and use a loop with
     * time diff? Like how you would with a game loop?
     */
    static size_t max_col = 40;
    static size_t max_row = 30;

    /*
     * TODO: Add title animation stuff pls
    terminal_reset();
    terminal_cursor_move_to(
        1,
        (max_col / 2) - 3  // 3 being the half the length of the project name
    );
    terminal_set_foreground(&color_pink);
    fprintf(stdout, "cg");
    terminal_reset();
    fprintf(stdout, "'");
    terminal_set_foreground(&color_blue);
    fprintf(stdout, "BASE");
    terminal_reset();
    terminal_unhide_cursor();
    fflush(stdout);
    */

    /*
     * End beginning animation stuff
     */

    screen_t *screen = malloc(sizeof(screen_t));
    if (screen == NULL) {
        // TODO: need to handle this case
    }
    screen->padding = (padding_t){
        .top = 1,
        .top_color = color_pink,
        .right = 1,
        .right_color = color_pink,
        .bottom = 1,
        .bottom_color = color_pink,
        .left = 1,
        .left_color = color_pink,
    };
    // NOTE: .row and .col need to be relative to the terminal
    // padding + 1 basically
    screen->cursor = (cursor_t){ .row = 2, .col = 2 };
    screen->max_row = max_row;
    screen->max_col = max_col;
    screen->animation_time = 500.0;
    screen->current_time = 0.0;

    database_t *database = new_database(
        screen_get_row_count(screen),
        screen_get_column_count(screen)
    );
    if (database == NULL) {
        log_message("database is a NULL ptr");
        // TODO: need to handle this case
    }

    terminal_cursor_move_to(screen->cursor.row, screen->cursor.col);
    fflush(stdout);

    key_code_t *root = create_key_code_tree();
    log_message("ROOT: %d - %d", root->capacity, root->size);

    // Need to flush stdin just in case the user pressed a button while the
    // animation was running
    fflush(stdin);

    bool running = true;

    clock_t last_render = clock();
    double accumulator = 0.0;
    // TODO: This should make it so that the render is only happening 4 times a second
    // Maybe it would be better to only render if there is a change (on input)?
    double slice = 16.7 * 15;

    key_press_t *key_press = malloc(sizeof(key_press_t));
    if (key_press == NULL) {
        log_message("key_press is a NULL ptr");
        // TODO: handle this case ...
    }
    key_press->is_special = true;
    key_press->key = NOOP_KEY;

    while (running) {

        clock_t now = clock();
        double delta = (now - last_render) / (CLOCKS_PER_SEC / 1000.0);
        last_render = now;

        handle_input(key_press, root);

        if (key_press->is_special && key_press->key == F1_KEY) {
            running = false;
            continue;
        } else if (!key_press->is_special) {
            database_update_at(
                database,
                screen_get_row_index(screen),
                screen_get_column_index(screen),
                key_press->key
            );
            screen_move_cursor(screen, RIGHT);
        }

        screen_update(screen, key_press, delta);
        screen_draw(screen);

        accumulator += delta;
        if (accumulator < slice) {
            accumulator += delta;
            continue;
        }
        accumulator -= slice;

        // draw part of the loop

        terminal_hide_cursor();
        terminal_set_foreground(&color_pink);

        for (int row_index = 0; row_index < database->rows; ++row_index) {
            terminal_cursor_move_to(
                screen->padding.right + 1 + row_index,
                screen->padding.top + 1
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

        terminal_cursor_move_to(screen->cursor.row, screen->cursor.col);
        terminal_unhide_cursor();
        fflush(stdout);
    }

    // Do I need to free this if the program is going to close anyways?
    // Won't the OS release that memory? Is it good hygiene to do it anyways?
    free(screen);
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
