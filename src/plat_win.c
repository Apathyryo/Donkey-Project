/* Платформа Windows: вывод в консоль, клавиатура через conio.h */
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include "platform.h"
#include "core.h"

/* в старых заголовках MinGW этой константы может не быть */
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

static HANDLE out;

void plat_init(void)
{
    CONSOLE_CURSOR_INFO cursor;
    DWORD mode;

    out = GetStdHandle(STD_OUTPUT_HANDLE);
    system("cls");
    /* спрятать курсор */
    GetConsoleCursorInfo(out, &cursor);
    cursor.bVisible = FALSE;
    SetConsoleCursorInfo(out, &cursor);
    /* включить escape-коды цвета, как в терминале Linux (Windows 10+) */
    GetConsoleMode(out, &mode);
    SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

void plat_done(void)
{
    CONSOLE_CURSOR_INFO cursor;

    GetConsoleCursorInfo(out, &cursor);
    cursor.bVisible = TRUE;
    SetConsoleCursorInfo(out, &cursor);
    printf("\033[0m\n");
}

int plat_key(void)
{
    int c;

    if (!_kbhit())
        return 0;
    c = _getch();
    /* стрелки приходят двумя кодами: 0 или 224, затем код стрелки */
    if (c == 0 || c == 224) {
        switch (_getch()) {
        case 72: return KEY_UP;
        case 80: return KEY_DOWN;
        case 77: return KEY_RIGHT;
        case 75: return KEY_LEFT;
        }
        return 0;
    }
    return c;
}

void plat_draw(const char *screen, const char *colors)
{
    COORD pos = { 0, 0 };
    int i, y, last = -1;

    /* курсор в левый верхний угол, затем выводим строки экрана */
    SetConsoleCursorPosition(out, pos);
    for (y = 0; y < SCREEN_H; y++) {
        for (i = y * SCREEN_W; i < (y + 1) * SCREEN_W; i++) {
            /* код цвета \033[9Xm выводим, только когда цвет меняется */
            if (colors[i] != last) {
                last = colors[i];
                printf("\033[9%dm", last);
            }
            putchar(screen[i]);
        }
        putchar('\n');
    }
    printf("\033[0m");
    fflush(stdout);
}

void plat_sleep(int ms)
{
    Sleep(ms);
}
