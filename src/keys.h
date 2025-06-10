#pragma once

#include <sys/types.h>

typedef enum SpecialKey {
    NOOP_KEY = 0x00,
    ESCAPE_KEY = 0x1b,
    UP_ARROW_KEY = 0x41,
    DOWN_ARROW_KEY = 0x42,
    RIGHT_ARROW_KEY = 0x43,
    LEFT_ARROW_KEY = 0x44,
    F1_KEY = 0x50
} special_key_t;

typedef struct KeyCode key_code_t;
typedef struct KeyList key_list_t;

typedef struct KeyCode {
    key_list_t *children;
    special_key_t special_key;
    u_int8_t key;
    u_int8_t is_leaf;
} key_code_t;

key_code_t *create_key_code(u_int8_t key, size_t capacity, special_key_t special_key);
void free_key_code(key_code_t *key_code);

typedef struct KeyList {
    key_code_t **keys;
    size_t capacity;
    size_t size;
} key_list_t;

key_list_t *create_key_list(size_t capacity);
int key_code_add_child(key_code_t *parent, key_code_t *child);

key_code_t *create_key_code_tree();
