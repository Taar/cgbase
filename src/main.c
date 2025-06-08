#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include "terminal.h"
#include "logger.h"

int main (int argc, char *argv[]) {
    size_t error = init_logging("application.log");
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
    term.c_iflag     = 0;
    term.c_cc[VMIN]  = 0;
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
    static unsigned int max_col = 40;
    static unsigned int max_row = 30;

    unsigned int row = 1;
    unsigned int col = 1;
    unsigned int col_stop = max_col;
    unsigned int row_stop = max_row;

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
    screen->padding = (padding_t){ .top = 1, .right = 1, .bottom = 1, .left = 1 };
    screen->cursor = (cursor_t){ .row = 2, .col = 2 };
    screen->max_row = max_row;
    screen->max_col = max_col;

    terminal_cursor_move_to(screen->cursor.row, screen->cursor.col);
    fflush(stdout);

    // Need to flush stdin just in case the user pressed a button while the
    // animation was running
    fflush(stdin);
    unsigned char key[1];
    u_int8_t running = 1;
    size_t result;
    while (running == 1) {
        result = read(fileno(stdin), key, 1);
        if (result == 0) {
            continue;
        }
        log_message("Key: %x", key[0]);
        switch (key[0]) {
            case '\x1b': {
                /*
                 * TODO
                 * Need a better way to validate more complex key presses.
                 * I believe using a trie data structure would work best here.
                 */

                do {
                    result = read(fileno(stdin), key, 1);
                    log_message("Next: %zu %c %x", result, key[0], key[0]);
                } while (result == 1);

                switch (key[0]) {
                    case UP:
                        screen_move_cursor(screen, UP);
                        terminal_cursor_move_to(screen->cursor.row, screen->cursor.col);
                        break;
                    case DOWN:
                        screen_move_cursor(screen, DOWN);
                        terminal_cursor_move_to(screen->cursor.row, screen->cursor.col);
                        break;
                    case RIGHT:
                        // This is no updating the screen and I am not sure why
                        screen_move_cursor(screen, RIGHT);
                        terminal_cursor_move_to(screen->cursor.row, screen->cursor.col);
                        break;
                    case LEFT:
                        screen_move_cursor(screen, LEFT);
                        terminal_cursor_move_to(screen->cursor.row, screen->cursor.col);
                        break;
                    case 'P':
                        running = 0;
                        break;
                }
                break;
            }
            default:
                terminal_reset();
                terminal_set_foreground(&color_pink);
                // Need to move the cursor back one, this cannot be disabled
                fprintf(stdout, "%c\033[D", key[0]);
                screen_move_cursor(screen, RIGHT);
                terminal_cursor_move_to(screen->cursor.row, screen->cursor.col);
                terminal_reset();
                fflush(stdout);
                break;
        }
        // If no data was read, then the stdin needs to be flushed otherwise
        // the last character will stay in the buffer. At least that's what appears
        // to happen.
        if (result == 0) {
            fflush(stdin);
        }
    }

    // Do I need to free this if the program is going to close anyways?
    // Won't the OS release that memory? Is it good hygiene to do it anyways?
    free(screen);

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
