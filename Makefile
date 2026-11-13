# Сборка игры «Объезд»
# Linux:   make
# Windows: mingw32-make

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2

SRC = src/main.c src/core.c src/vm.c src/forth.c
HDR = src/vm.h src/forth.h src/core.h src/platform.h

# для каждой ОС свой файл платформы
ifeq ($(OS),Windows_NT)
    SHELL = cmd.exe
    EXE = .exe
    RM = del /Q
    RUN =
    PLAT = src/plat_win.c
else
    EXE =
    RM = rm -f
    RUN = ./
    PLAT = src/plat_unix.c
endif

all: game$(EXE)

game$(EXE): $(SRC) $(PLAT) $(HDR)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(PLAT)

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
