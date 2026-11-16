#ifndef CORE_H
#define CORE_H

/* Ядро игры: загружает скрипт с логикой, выполняет его на ВМ
   и хранит экран. Ядро не зависит от платформы. */

#define SCREEN_W 40
#define SCREEN_H 22

/* Цвета клеток экрана, номера как у цветов терминала */
enum {
    COLOR_GREY,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE,
    COLOR_COUNT
};

int core_load(const char *path);
int core_init(void);
int core_tick(int key);
const char *core_screen(void);
const char *core_colors(void);
const char *core_error(void);

#endif
