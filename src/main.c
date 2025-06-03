#include <stdio.h>
#include <termios.h>
#include "terminal.h"

int main (int argc, char *argv[]) {
    // Need to save the current terminal settings so that they
    // can be restored after the application closes
    struct termios old_term;
    tcgetattr(0, &old_term);

    struct termios term;
    cfmakeraw(&term);
    tcsetattr(0, TCSANOW, &term);

    printf("\033[H\033[2J\r");
    rgb_t pink = new_color(0xFF77FF);
    rgb_t blue = new_color(0x1133FF);
    printf(
        "\033[38;2;%d;%d;%dmcg\033[0m'\033[38;2;%d;%d;%dmBASE\033[0m\r\n",
        pink.components.r,
        pink.components.g,
        pink.components.b,
        blue.components.r,
        blue.components.g,
        blue.components.b
    );

    char key;
    while (1) {
        key = getchar();
        if (key == 'q') {
            break;
        }
    }

    // IMPORTANT: Need to restore the terminal to the original settings
    tcsetattr(0, TCSANOW, &old_term);
    return 0;
}
