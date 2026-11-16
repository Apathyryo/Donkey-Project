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

# 32-битная сборка для Linux (процессор i386), нужен пакет gcc-multilib
game32: $(SRC) src/plat_unix.c $(HDR)
	$(CC) $(CFLAGS) -m32 -o $@ $(SRC) src/plat_unix.c

# Модульные тесты
test_vm$(EXE): tests/test_vm.c tests/check.h src/vm.c src/vm.h
	$(CC) $(CFLAGS) -o $@ tests/test_vm.c src/vm.c

test_forth$(EXE): tests/test_forth.c tests/check.h src/vm.c src/forth.c $(HDR)
	$(CC) $(CFLAGS) -o $@ tests/test_forth.c src/vm.c src/forth.c

test: test_vm$(EXE) test_forth$(EXE)
	$(RUN)test_vm$(EXE)
	$(RUN)test_forth$(EXE)

# Фаззинг. В Linux лучше запускать с проверкой памяти:
# make fuzz FUZZFLAGS="-g -fsanitize=address,undefined -fno-sanitize-recover=all"
FUZZFLAGS =

run_fuzz$(EXE): tests/fuzz.c src/vm.c src/forth.c $(HDR)
	$(CC) $(CFLAGS) $(FUZZFLAGS) -o $@ tests/fuzz.c src/vm.c src/forth.c

fuzz: run_fuzz$(EXE)
	$(RUN)run_fuzz$(EXE)

# Веб-версия: один файл game.html, скрипт игры встраивается внутрь.
# Путь к компилятору можно задать: make web EMCC=путь/к/emcc
EMCC = emcc

web: game.html

game.html: $(SRC) src/plat_web.c $(HDR) src/shell.html scripts/game.fs
	$(EMCC) -O2 -o game.html $(SRC) src/plat_web.c --shell-file src/shell.html --embed-file scripts/game.fs -sSINGLE_FILE=1

clean:
	-$(RM) game$(EXE) game32 test_vm$(EXE) test_forth$(EXE) run_fuzz$(EXE) game.html

.PHONY: all test fuzz web clean
