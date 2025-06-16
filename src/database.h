#pragma once

#include <stdint.h>
#include <sys/types.h>

typedef struct Database {
    int rows;
    int columns;
    u_int8_t **lines;
} database_t;

database_t *new_database(int rows, int columns);
void free_database(database_t *database);

int database_update_at(
    database_t *database,
    int row,
    int column,
    u_int8_t character
);

void database_draw(database_t *database);
