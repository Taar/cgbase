#include <stdio.h>
#include <stdlib.h>

#include "database.h"
#include "errors.h"

database_t *new_database(size_t rows, size_t columns){
    database_t *database = malloc(sizeof(database_t));
    if (database == NULL) {
        return NULL;
    }

    database->columns = columns;
    database->rows = rows;
    database->lines = NULL;

    u_int8_t **lines = calloc(database->rows, sizeof(u_int8_t *));
    if (lines == NULL) {
        free(database);
        return NULL;
    }

    database->lines = lines;

    for (size_t row_index = 0; row_index < database->rows; ++row_index) {
        u_int8_t *line = calloc(database->columns, sizeof(u_int8_t));
        if (line == NULL) {
            for (size_t i = 0; i <= row_index; ++i) {
                u_int8_t *line = database->lines[i];
                free(line);
            }
            free(database->lines);
            free(database);
            return NULL;
        }

        database->lines[row_index] = line;
    }

    return database;
}

int database_update_at(
    database_t *database,
    size_t row,
    size_t column,
    u_int8_t character
) {
    if (database == NULL) {
        return ERROR_NULL_PTR;
    }

    if (row > database->rows) {
        return ERROR_ROW_OUT_OF_BOUNDS;
    }

    if (column > database->columns) {
        return ERROR_COLUMN_OUT_OF_BOUNDS;
    }

    u_int8_t *line = database->lines[row];
    line[column] = character;

    return 0;
}

void free_database(database_t *database) {
    if (database == NULL) {
        return;
    }
    for (size_t i = 0; i < database->rows; ++i) {
        free(database->lines[i]);
    }
    free(database->lines);
    free(database);
    return;
}
