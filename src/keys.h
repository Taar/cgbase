#pragma once

#include <sys/types.h>
#include <stdbool.h>

typedef enum SpecialKey {
    NOOP_KEY = 0x00,
    ESCAPE_KEY = 27,
    UP_ARROW_KEY = 65,
    DOWN_ARROW_KEY = 66,
    RIGHT_ARROW_KEY = 67,
    LEFT_ARROW_KEY = 68,
    F1_KEY = 80
} special_key_t;

typedef struct KeyCode key_code_t;

typedef struct KeyPress {
    u_int8_t key;
    bool is_special;
} key_press_t;

void handle_input(key_press_t *key_press, key_code_t *root);

// NOTE: leaf nodes will have a capacity and size of zero
typedef struct KeyCode {
    key_code_t **children;
    size_t capacity;
    size_t size;
    special_key_t special_key;
    u_int8_t key;
} key_code_t;

key_code_t *create_key_code(u_int8_t key, size_t capacity, special_key_t special_key);
key_code_t *create_key_code_tree();

int key_code_add_child(key_code_t *parent, key_code_t *child);
// TODO: fix the return value. The highest index can be the max of size_t which
// can fit a positive value that is larger than an int. Should this be a long long?
// I think it's ok for now cause the list won't ever have that many values
int key_code_find_by_index(key_code_t *key_code, u_int8_t key);
key_code_t *key_code_get_by_index(key_code_t *key_code, size_t index);

void free_key_code(key_code_t *key_code);
