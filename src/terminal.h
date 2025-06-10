#pragma once

#include <sys/types.h>

typedef union RGB {
    // Allows for the struct to be filled using a single number like
    // 0xF225F1. For me this makes the colors more readable since that
    // is what I am used to.
    u_int32_t color;
    // RGB bytes are reserved because my system is little endian *<:o)
    // This is done so that assign a number to the color field can be
    // done in the expected order eg. 0xFF0000 otherwise the value
    // would have to be written in reserve eg. 0x0000FF
    // I want this cause my editor displays the color for those hex
    // value and if I had to write them in reserve the color wouldn't
    // match which makes me sad :(
    struct {
        u_int8_t b;
        u_int8_t g;
        u_int8_t r;
        // Since the only number size that fits 3 u_int8_t is u_int32_t
        // the last byte will be unused which is why it's labeled as
        // pad(ding).
        u_int8_t pad;
    } components;
} rgb_t;

rgb_t new_color(u_int32_t hex_color);

int terminal_set_foreground(rgb_t *color);
int terminal_set_background(rgb_t *color);
int terminal_reset();
int terminal_hide_cursor();
int terminal_unhide_cursor();
int terminal_cursor_home();
int terminal_clear_screen();
int terminal_cursor_move_to(u_int32_t row, u_int32_t col);
