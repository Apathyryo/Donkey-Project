# Сборка игры «Объезд»
# Linux:   make
# Windows: mingw32-make

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2

SRC = src/main.c

ifeq ($(OS),Windows_NT)
    SHELL = cmd.exe
    EXE = .exe
    RM = del /Q
else
    EXE =
    RM = rm -f
endif

all: game$(EXE)

game$(EXE): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC)

clean:
	-$(RM) game$(EXE)

.PHONY: all clean
