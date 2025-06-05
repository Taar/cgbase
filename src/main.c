#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "terminal.h"

int main (int argc, char *argv[]) {
    // Need to save the current terminal settings so that they
    // can be restored after the application closes
    struct termios old_term;
    tcgetattr(0, &old_term);

    struct termios term;
    cfmakeraw(&term);
    tcsetattr(0, TCSANOW, &term);

    terminal_cursor_home();
    terminal_clear_screen();

    rgb_t pink = new_color(0xFF77FF);
    rgb_t blue = new_color(0x1133FF);

    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    fflush(stdout);

    useconds_t pause = 5000;
    int max_col = 60;
    int max_row = 30;

    // Creates a "square" pattern with each square getting smaller
    for (int square = 0; square < 12; ++square) {
        if ((square & 0x1) == 0) {
            terminal_set_background(&blue);
        } else {
            terminal_set_background(&pink);
        }

        int row = 1 + square;
        int col = 1 + square;
        int col_stop = max_col - square;
        int row_stop = max_row - square;

        for (; col <= col_stop; ++col) {
            fprintf(stdout, "\033[%d;%dH", row, col);
            fprintf(stdout, " ");
            fflush(stdout);
            usleep(pause);
        }
        for (; row <= row_stop; ++row) {
            fprintf(stdout, "\033[%d;%dH", row, col);
            fprintf(stdout, " ");
            fflush(stdout);
            usleep(pause);
        }
        for (; col > 1 + square; --col) {
            fprintf(stdout, "\033[%d;%dH", row, col);
            fprintf(stdout, " ");
            fflush(stdout);
            usleep(pause);
        }
        for (; row > 1 + square; --row) {
            fprintf(stdout, "\033[%d;%dH", row, col);
            fprintf(stdout, " ");
            fflush(stdout);
            usleep(pause);
        }
    }

    terminal_reset();
    fprintf(
        stdout,
        "\033[%d;%dH",
        max_row / 2,
        (max_col / 2) - 3  // 3 being the half the length of the project name
    );
    terminal_set_foreground(&pink);
    fprintf(stdout, "cg");
    terminal_reset();
    fprintf(stdout, "'");
    terminal_set_foreground(&blue);
    fprintf(stdout, "BASE");
    terminal_reset();

    // Need to flush stdin just in case the user pressed a button while the
    // animation was running
    fflush(stdin);
    char key;
    while (1) {
        key = getchar();
        if (key == 'q') {
            break;
        }
    }

    // Reset everything and clear the screen before restoring the old
    // terminal. Just makes the TUI feel less jank
    terminal_reset();
    terminal_cursor_home();
    terminal_clear_screen();
    fflush(stdout);

    // IMPORTANT: Need to restore the terminal to the original settings
    tcsetattr(0, TCSANOW, &old_term);
    return 0;
}
