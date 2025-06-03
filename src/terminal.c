#include <stdio.h>
#include <string.h>
#include "terminal.h"

rgb_t new_color(u_int32_t hex_color) {
    rgb_t color;
    color.color = hex_color;
    // Set the padding to zeros. Honestly not sure if this is
    // needed or not when initializing the struct on the stack.
    color.components.pad = 0x00;
    return color;
}
