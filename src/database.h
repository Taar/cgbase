#pragma once

#include <stdint.h>
#include <sys/types.h>

typedef struct Database {
    size_t rows;
    size_t columns;
    u_int8_t **lines;
} database_t;

database_t *new_database(size_t rows, size_t columns);
int database_update_at(
    database_t *database,
    size_t row,
    size_t column,
    u_int8_t character
);
void free_database(database_t *database);
