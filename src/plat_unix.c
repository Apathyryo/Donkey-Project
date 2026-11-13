/* Платформа Linux (и другие Unix): вывод в терминал escape-кодами,
   клавиатура через termios */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include "platform.h"
#include "core.h"

static struct termios old_mode;

void plat_init(void)
{
    struct termios t;

    /* выключаем построчный ввод и эхо, read не ждёт нажатия */
    tcgetattr(0, &old_mode);
    t = old_mode;
    t.c_lflag &= ~(ICANON | ECHO);
    t.c_cc[VMIN] = 0;
    t.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &t);
    printf("\033[2J\033[?25l");  /* очистить экран, спрятать курсор */
}

void plat_done(void)
{
    tcsetattr(0, TCSANOW, &old_mode);
    printf("\033[?25h\n");       /* вернуть курсор */
}

int plat_key(void)
{
    unsigned char c, seq[2];

    if (read(0, &c, 1) != 1)
        return 0;
    if (c != 27)
        return c;
    /* стрелки приходят как ESC [ A..D */
    if (read(0, seq, 2) == 2 && seq[0] == '[') {
        switch (seq[1]) {
        case 'A': return KEY_UP;
        case 'B': return KEY_DOWN;
        case 'C': return KEY_RIGHT;
        case 'D': return KEY_LEFT;
        }
        return 0;
    }
    return KEY_ESC;
}

void plat_draw(const char *screen)
{
    int y;

    printf("\033[H");  /* курсор в левый верхний угол */
    for (y = 0; y < SCREEN_H; y++) {
        fwrite(screen + y * SCREEN_W, 1, SCREEN_W, stdout);
        putchar('\n');
    }
    fflush(stdout);
}

void plat_sleep(int ms)
{
    struct timespec ts;

    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}
