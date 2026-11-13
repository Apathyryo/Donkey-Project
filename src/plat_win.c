/* Платформа Windows: вывод в консоль, клавиатура через conio.h */
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include "platform.h"
#include "core.h"

static HANDLE out;

void plat_init(void)
{
    CONSOLE_CURSOR_INFO cursor;

    out = GetStdHandle(STD_OUTPUT_HANDLE);
    system("cls");
    /* спрятать курсор */
    GetConsoleCursorInfo(out, &cursor);
    cursor.bVisible = FALSE;
    SetConsoleCursorInfo(out, &cursor);
}

void plat_done(void)
{
    CONSOLE_CURSOR_INFO cursor;

    GetConsoleCursorInfo(out, &cursor);
    cursor.bVisible = TRUE;
    SetConsoleCursorInfo(out, &cursor);
    printf("\n");
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

void plat_draw(const char *screen)
{
    COORD pos = { 0, 0 };
    int y;

    /* курсор в левый верхний угол, затем выводим строки экрана */
    SetConsoleCursorPosition(out, pos);
    for (y = 0; y < SCREEN_H; y++) {
        fwrite(screen + y * SCREEN_W, 1, SCREEN_W, stdout);
        putchar('\n');
    }
    fflush(stdout);
}

void plat_sleep(int ms)
{
    Sleep(ms);
}
