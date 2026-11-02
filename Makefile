# Сборка игры «Объезд»
# Linux:   make
# Windows: mingw32-make

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2

SRC = src/main.c src/vm.c src/forth.c
HDR = src/vm.h src/forth.h

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

game$(EXE): $(SRC) $(HDR)
	$(CC) $(CFLAGS) -o $@ $(SRC)

# Модульные тесты
test_vm$(EXE): tests/test_vm.c tests/check.h src/vm.c src/vm.h
	$(CC) $(CFLAGS) -o $@ tests/test_vm.c src/vm.c

test_forth$(EXE): tests/test_forth.c tests/check.h src/vm.c src/forth.c $(HDR)
	$(CC) $(CFLAGS) -o $@ tests/test_forth.c src/vm.c src/forth.c

test: test_vm$(EXE) test_forth$(EXE)
	$(RUN)test_vm$(EXE)
	$(RUN)test_forth$(EXE)

clean:
	-$(RM) game$(EXE) test_vm$(EXE) test_forth$(EXE)

.PHONY: all test clean
