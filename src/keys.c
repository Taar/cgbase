#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "keys.h"
#include "errors.h"
#include "logger.h"

key_code_t *create_key_code(u_int8_t key, int capacity, special_key_t special_key) {
    key_code_t *key_code = malloc(sizeof(key_code_t));
    if (key_code == NULL) {
        return NULL;
    }

    key_code->key = key;
    key_code->special_key = special_key;
    key_code->capacity = capacity;
    key_code->size = 0;

    if (key_code->capacity == 0) {
        key_code->children = NULL;
        return key_code;
    }

    key_code_t **children = calloc(capacity, sizeof(key_code_t *));
    if (children == NULL) {
        free(key_code);
        return NULL;
    }
    key_code->children = children;

    return key_code;
}

void free_key_code(key_code_t *key_code) {
    if (key_code == NULL) {
        return;
    }

    if (key_code->capacity == 0) {
        key_code_t **children = key_code->children;
        if (key_code->size > 0) {
            for (int i = 0; i < key_code->size; ++i) {
                free_key_code(children[i]);
            }
        }
        free(*children);
    }

    free(key_code);
    return;
}

int key_code_add_child(key_code_t *parent, key_code_t *child) {
    if (parent == NULL || child == NULL) {
        return ERROR_NULL_PTR;
    }

    if (parent->capacity == 0) {
        return ERROR_CANNOT_ADD_TO_LEAF_NODE;
    }

    if (parent->size == parent->capacity) {
        parent->capacity *= 2;
        key_code_t **children = reallocarray(
            parent->children,
            parent->capacity,
            sizeof(key_code_t *)
        );
        if (children == NULL) {
            return ERROR_NULL_PTR;
        }
        parent->children = children;
    }

    parent->children[parent->size] = child;
    parent->size += 1;

    return 0;
}

int key_code_find_by_index(key_code_t *key_code, u_int8_t key) {
    if (key_code == NULL || key_code->capacity == 0) {
        return -1;
    }

    for (int i = 0; i < key_code->size; ++i) {
        key_code_t *child = key_code->children[i];
        if (child != NULL && child->key == key) {
            return i;
        }
    }

    return -1;
}

key_code_t *key_code_get_by_index(key_code_t *key_code, int index) {
    if (key_code == NULL || key_code->capacity == 0 || index >= key_code->size) {
        return NULL;
    }

    return key_code->children[index];
}

void handle_input(key_press_t *key_press, key_code_t *root) {
    if (key_press == NULL) {
        log_message("handle_input: key press ptr is NULL");
        return;
    }

    if (root == NULL) {
        log_message("handle_input: root ptr is NULL");
        return;
    }

    u_int8_t key[1];
    size_t result = 0;
    key_code_t *current_key_code = root;
    bool key_code_not_found = false;

    while ((result = read(fileno(stdin), key, 1)) > 0) {
        log_message("stdin: %d - %x", result, key[0]);

        int child_index = key_code_find_by_index(current_key_code, key[0]);

        if (child_index < 0) {
            key_code_not_found = true;
            break;
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
            current_key_code = root;
            break;
        }

        log_message("Key code found: %x", key_code->key);
        current_key_code = key_code;
    }

    if (current_key_code == root && key_code_not_found) {
        key_press->key = key[0];
        key_press->is_special = false;
        return;
    }

    key_press->key = current_key_code->key;
    key_press->is_special = true;
}

#define KEY_COUNT 10
#define MAX_KEY_BYTES 6
#define BASE_CAPACITY 2
key_code_t *create_key_code_tree() {
    key_code_t *root = create_key_code(
        0x00,
        BASE_CAPACITY,
        NOOP_KEY
    );
    if (root == NULL) {
        return NULL;
    }

    // Being super lazy here *<:o)
    // There is probably a better way to do this but I don't want to spend the
    // figuring it out atm. Doesn't seem like it would be worth it
    // The special key is always after 0x00 which also defines the end of the
    // key presses byte code
    u_int8_t keys[KEY_COUNT][MAX_KEY_BYTES] = {
        {9, 0x00, NOOP_KEY},
        {13, 0x00, NOOP_KEY},
        {127, 0x00, NOOP_KEY},
        {27, 0x00, NOOP_KEY},
        {27, 91, 65, 0x00, UP_ARROW_KEY},
        {27, 91, 66, 0x00, DOWN_ARROW_KEY},
        {27, 91, 67, 0x00, RIGHT_ARROW_KEY},
        {27, 91, 68, 0x00, LEFT_ARROW_KEY},
        {27, 79, 80, 0x00, F1_KEY},
        {27, 79, 81, 0x00, F2_KEY}
    };

    key_code_t *current_key_code = root;
    for (size_t key_index = 0; key_index < KEY_COUNT; ++key_index) {
        for (size_t byte_index = 0; byte_index < MAX_KEY_BYTES; ++byte_index) {
            u_int8_t key = keys[key_index][byte_index];
            if (key == 0x00) {
                current_key_code->special_key = keys[key_index][byte_index + 1];
                break;
            }

            int index = key_code_find_by_index(current_key_code, key);
            if (index >= 0) {
                current_key_code = key_code_get_by_index(current_key_code, index);
                if (current_key_code == NULL) {
                    free_key_code(root);
                    return NULL;
                }
            } else if (index < 0) {
                key_code_t *new_key_code = create_key_code(
                    key,
                    BASE_CAPACITY,
                    NOOP_KEY
                );
                if (new_key_code == NULL) {
                    free_key_code(root);
                    return NULL;
                }
                key_code_add_child(current_key_code, new_key_code);
                current_key_code = new_key_code;
            }
        }
        current_key_code = root;
    }

    return root;
}
