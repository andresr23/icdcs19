.PHONY: clean

CC = gcc
FLAGS = -w

DEP_L = include
SRC_L = src
OBJ_L = $(SRC_L)

_DEP = cell.h
_OBJ = main.o cache.o U.o

DEP = $(patsubst %, $(DEP_L)/%, $(_DEP))
OBJ = $(patsubst %, $(OBJ_L)/%, $(_OBJ))

$(OBJ_L)/%.o: $(SRC_L)/%.c
	$(CC) $^ -c -o $@ $(FLAGS)

default: $(OBJ) $(DEP)
	$(CC) $^ -o spectre

clean:
	rm -f $(OBJ_L)/*.o
	rm -f spectre
