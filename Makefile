# Сборка игры «Объезд»
# Linux:   make
# Windows: mingw32-make

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2

SRC = src/main.c src/vm.c

ifeq ($(OS),Windows_NT)
    SHELL = cmd.exe
    EXE = .exe
    RM = del /Q
    RUN =
else
    EXE =
    RM = rm -f
    RUN = ./
endif

all: game$(EXE)

game$(EXE): $(SRC) src/vm.h
	$(CC) $(CFLAGS) -o $@ $(SRC)

# Модульные тесты
run_tests$(EXE): tests/test_vm.c src/vm.c src/vm.h
	$(CC) $(CFLAGS) -o $@ tests/test_vm.c src/vm.c

test: run_tests$(EXE)
	$(RUN)run_tests$(EXE)

clean:
	-$(RM) game$(EXE) run_tests$(EXE)

.PHONY: all test clean
