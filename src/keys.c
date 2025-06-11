#include <stdlib.h>
#include <sys/types.h>

#include "keys.h"
#include "errors.h"

key_code_t *create_key_code(u_int8_t key, size_t capacity, special_key_t special_key) {
    key_code_t *key_code = malloc(sizeof(key_code_t));
    if (key_code == NULL) {
        return NULL;
    }

    key_code->key = key;
    key_code->special_key = special_key;

    if (capacity == 0) {
        key_code->is_leaf = 1;
        key_code->children = NULL;
        return key_code;
    }

    key_code->is_leaf = 0;

    key_list_t *key_list = create_key_list(capacity);
    if (key_list == NULL) {
        free(key_code);
        return NULL;
    }
    key_code->children = key_list;

    return key_code;
}

void free_key_code(key_code_t *key_code) {
    if (key_code == NULL) {
        return;
    }

    if (key_code->is_leaf == 0) {
        key_list_t *children = key_code->children;
        if (children->size > 0) {
            for (size_t i = 0; i < children->size; ++i) {
                free_key_code(children->keys[i]);
            }
        }
        free(children);
    }

    free(key_code);
    return;
}

key_list_t *create_key_list(size_t capacity) {
    if (capacity == 0) {
        return NULL;
    }

    key_list_t *key_list = malloc(sizeof(key_code_t));
    if (key_list == NULL) {
        return NULL;
    }

    key_code_t **children = calloc(capacity, sizeof(key_code_t *));
    if (children == NULL) {
        free(key_list);
        return NULL;
    }

    key_list->keys = children;
    key_list->capacity = capacity;
    key_list->size = 0;
    return key_list;
}

int key_code_add_child(key_code_t *parent, key_code_t *child) {
    if (parent == NULL || child == NULL) {
        return ERROR_NULL_PTR;
    }

    if (parent->is_leaf > 0) {
        return ERROR_CANNOT_ADD_TO_LEAF_NODE;
    }

    if (parent->children->size == parent->children->capacity) {
        parent->children->capacity *= 2;
        key_code_t **children = reallocarray(
            parent->children->keys,
            parent->children->capacity,
            sizeof(key_code_t *)
        );
        if (children == NULL) {
            return ERROR_NULL_PTR;
        }
        parent->children->keys = children;
    }

    parent->children->keys[parent->children->size] = child;
    parent->children->size += 1;

    return 0;
}

int key_code_find_by_index(key_code_t *key_code, u_int8_t key) {
    if (key_code == NULL || key_code->is_leaf) {
        return -1;
    }

    for (size_t i = 0; i < key_code->children->size; ++i) {
        key_code_t *child = key_code->children->keys[i];
        if (child != NULL && child->key == key) {
            return i;
        }
    }

    return -1;
}

key_code_t *key_code_get_by_index(key_code_t *key_code, int index) {
    if (key_code == NULL || key_code->is_leaf || index >= key_code->children->size) {
        return NULL;
    }

    return key_code->children->keys[index];
}

#define KEY_COUNT 9
#define MAX_KEY_BYTES 6
#define BASE_CAPACITY 8
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
        {27, 79, 80, 0x00, F1_KEY}
    };

    key_code_t *current_key_code = root;
    for (size_t key_index = 0; key_index < KEY_COUNT; ++key_index) {
        for (size_t byte_index = 0; byte_index < MAX_KEY_BYTES; ++byte_index) {
            u_int8_t byte = keys[key_index][byte_index];
            if (byte == 0x00) {
                current_key_code->special_key = keys[key_index][byte_index + 1];
                break;
            }
            int index = key_code_find_by_index(current_key_code, byte);
            if (index >= 0) {
                current_key_code = key_code_get_by_index(current_key_code, index);
                if (current_key_code == NULL) {
                    free_key_code(root);
                    return NULL;
                }
            } else if (index < 0) {
                key_code_t *new_key_code = create_key_code(
                    byte,
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
