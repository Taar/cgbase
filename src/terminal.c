#include <stdio.h>
#include "terminal.h"

rgb_t new_color(u_int32_t hex_color) {
    rgb_t color;
    color.color = hex_color;
    // Set the padding to zeros. Honestly not sure if this is
    // needed or not when initializing the struct on the stack.
    color.components.pad = 0x00;
    return color;
}

int terminal_reset() {
    return fprintf(stdout, "\033[0m");
}

int terminal_clear_screen() {
    return printf("\033[2J\r");
}

int terminal_cursor_home() {
    return printf("\033[H\r");
}

int terminal_set_background(rgb_t *color) {
    return printf(
        "\033[48;2;%d;%d;%dm",
        color->components.r,
        color->components.g,
        color->components.b
    );
}

int terminal_set_foreground(rgb_t *color) {
    return printf(
        "\033[38;2;%d;%d;%dm",
        color->components.r,
        color->components.g,
        color->components.b
    );
}
