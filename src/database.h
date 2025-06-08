#pragma once

#include <stdint.h>
#include <sys/types.h>

# define NULL_PTR -1
# define ROW_OUT_OF_BOUNDS -2
# define COLUMN_OUT_OF_BOUNDS -3

typedef struct Database {
    u_int32_t rows;
    u_int32_t columns;
    u_int8_t **lines;
} database_t;

database_t *new_database(u_int32_t rows, u_int32_t columns);
int database_update_at(
    database_t *database,
    u_int32_t row,
    u_int32_t column,
    u_int8_t character
);
void free_database(database_t *database);
