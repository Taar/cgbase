CC = gcc
CFLAGS = -Wall -Wextra -pedantic

SRC_DIR := src
OBJ_DIR := build

SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))

cgbase: $(OBJECTS)
	$(CC) $(CFLAGS) -Lm /usr/lib/libm.so $^ -o $(OBJ_DIR)/$@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -g3 -I$(SRC_DIR) -c $< -o $@

run:
	./build/cgbase

clean:
	rm -rf $(OBJ_DIR)
