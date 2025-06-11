#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>

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
    static useconds_t pause = 5000;
    static size_t max_col = 40;
    static size_t max_row = 30;

    size_t row = 1;
    size_t col = 1;
    size_t col_stop = max_col;
    size_t row_stop = max_row;

    terminal_set_background(&color_pink);
    for (; col <= col_stop; ++col) {
        terminal_cursor_move_to(row, col);
        fprintf(stdout, " ");
        fflush(stdout);
        usleep(pause);
    }
    for (; row <= row_stop; ++row) {
        terminal_cursor_move_to(row, col);
        fprintf(stdout, " ");
        fflush(stdout);
        usleep(pause);
    }
    for (; col > 1; --col) {
        terminal_cursor_move_to(row, col);
        fprintf(stdout, " ");
        fflush(stdout);
        usleep(pause);
    }
    for (; row > 1; --row) {
        terminal_cursor_move_to(row, col);
        fprintf(stdout, " ");
        fflush(stdout);
        usleep(pause);
    }

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

    /*
     * End beginning animation stuff
     */

    screen_t *screen = malloc(sizeof(screen_t));
    if (screen == NULL) {
        // TODO: need to handle this case
    }
    screen->padding = (padding_t){ .top = 1, .right = 1, .bottom = 1, .left = 1 };
    screen->cursor = (cursor_t){ .row = 2, .col = 2 };
    screen->max_row = max_row;
    screen->max_col = max_col;

    database_t *database = new_database(
        screen_get_row_count(screen),
        screen_get_column_count(screen)
    );
    if (database == NULL) {
        // TODO: need to handle this case
    }

    terminal_cursor_move_to(screen->cursor.row, screen->cursor.col);
    fflush(stdout);

    key_code_t *root = create_key_code_tree();
    log_message("ROOT: %d - %d", root->children->capacity, root->children->size);

    // Need to flush stdin just in case the user pressed a button while the
    // animation was running
    fflush(stdin);

    u_int8_t key[1];
    u_int8_t running = 1;
    size_t result = 0;
    key_code_t *current_key_code = root;

    clock_t last_render = clock();
    double accumulator = 0;
    double slice = 16.7 * 15;

    while (running == 1) {
        do {
            result = read(fileno(stdin), key, 1);

            if (result == 0 && current_key_code == root) {
                break;
            }
            log_message("stdin: %d - %x", result, key[0]);

            if (result == 0 && current_key_code != root) {
                log_message("SPECIAL: %x", current_key_code->special_key);
                switch (current_key_code->special_key) {
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
                    case F1_KEY:
                        running = 0;
                        break;
                    default:
                        break;
                }
                break;
            }

            log_message("Key Byte: %x", key[0]);

            int child_index = key_code_find_by_index(current_key_code, key[0]);

            log_message("Child Index: %d", child_index);

            if (child_index < 0 && current_key_code == root) {
                database_update_at(
                    database,
                    screen_get_row_index(screen),
                    screen_get_column_index(screen),
                    key[0]
                );
                screen_move_cursor(screen, RIGHT);
                continue;
            }

            key_code_t *key_code = key_code_get_by_index(
                current_key_code,
                child_index
            );
            if (key_code == NULL) {
                log_message(
                    "WARNING: NULL ptr for %x at %d",
                    current_key_code->key,
                    child_index
                );
                continue;
            }
            log_message("Key code found: %x", key_code->key);
            current_key_code = key_code;
        } while (result > 0);

        key[0] = 0x00;
        fflush(stdin);
        current_key_code = root;

        double duration = (1.0 * (clock() - last_render) / CLOCKS_PER_SEC) / 1000.0;
        accumulator += duration;
        if (accumulator < slice) {
            accumulator += duration;
            continue;
        }
        accumulator -= slice;

        // draw part of the loop

        terminal_hide_cursor();
        terminal_set_foreground(&color_pink);

        for (size_t row_index = 0; row_index < database->rows; ++row_index) {
            terminal_cursor_move_to(
                screen->padding.right + 1 + row_index,
                screen->padding.top + 1
            );
            u_int8_t *line = database->lines[row_index];
            if (line == NULL) {
                log_message("LINE NULL at %d", row_index);
                break;
            }
            for (size_t col_index = 0; col_index < database->columns; ++col_index) {
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
