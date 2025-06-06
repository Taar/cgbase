#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include "terminal.h"

int main (int argc, char *argv[]) {
    FILE *logFile;
    logFile = fopen("application.log", "a");

    if (logFile == NULL) {
       perror("Error opening log file.");
       return -1;
    }
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
    unsigned int max_col = 40;
    unsigned int max_row = 30;

    unsigned int row = 1;
    unsigned int col = 1;
    unsigned int col_stop = max_col;
    unsigned int row_stop = max_row;

    terminal_set_background(&pink);
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
    terminal_set_foreground(&pink);
    fprintf(stdout, "cg");
    terminal_reset();
    fprintf(stdout, "'");
    terminal_set_foreground(&blue);
    fprintf(stdout, "BASE");
    terminal_reset();

    screen_t screen = {
        .padding = { .top = 1, .right = 1, .bottom = 1, .left = 1 },
        .cursor = { .row = 2, .col = 2 },
        .max_row = max_row,
        .max_col = max_col
    };

    terminal_cursor_move_to(screen.cursor.row, screen.cursor.col);

    // Need to flush stdin just in case the user pressed a button while the
    // animation was running
    fflush(stdin);
    int key;
    u_int8_t running = 1;
    while (running == 1) {
        key = getchar();
        fprintf(logFile, "Key: %x\n", key);
        fflush(logFile);
        switch (key) {
            case '\x1b': { // Never gets here and I'm not sure why
                int next;
                do {
                    next = getchar();
                    if (next != 'O' || next != '[') {
                        break;
                    }
                    fprintf(logFile, "Next: %x\n", next);
                    fflush(logFile);
                } while (next != EOF);

                switch (next) {
                    case UP:
                        screen_move_cursor(&screen, UP);
                        terminal_cursor_move_to(screen.cursor.row, screen.cursor.col);
                        break;
                    case DOWN:
                        screen_move_cursor(&screen, DOWN);
                        terminal_cursor_move_to(screen.cursor.row, screen.cursor.col);
                        break;
                    case RIGHT:
                        screen_move_cursor(&screen, RIGHT);
                        terminal_cursor_move_to(screen.cursor.row, screen.cursor.col);
                        break;
                    case LEFT:
                        screen_move_cursor(&screen, LEFT);
                        terminal_cursor_move_to(screen.cursor.row, screen.cursor.col);
                        break;
                    case 'P':
                        running = 0;
                        break;
                }
                break;
            }
            default:
                terminal_reset();
                terminal_set_foreground(&pink);
                // Need to move the cursor back one, this cannot be disabled
                fprintf(stdout, "%c\033[D", key);
                terminal_reset();
                fflush(stdout);
                break;
        }
    }

    fclose(logFile);
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
