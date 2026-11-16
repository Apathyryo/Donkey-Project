#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include "core.h"
#include "platform.h"

#define FRAME_MS 50   /* 20 кадров в секунду */

static int running = 1;

/* Один кадр игры: клавиша -> логика -> экран */
static void frame(void)
{
    int key;

    if (!running)
        return;
    key = plat_key();
    if (key == KEY_ESC || core_tick(key) != 0) {
        running = 0;
        return;
    }
    plat_draw(core_screen(), core_colors());
}

int main(int argc, char **argv)
{
    const char *path = "scripts/game.fs";

    if (argc > 1)
        path = argv[1];
    srand((unsigned)time(NULL));
    if (core_load(path) != 0) {
        printf("%s\n", core_error());
        return 1;
    }

    plat_init();
    if (core_init() == 0) {
#ifdef __EMSCRIPTEN__
        /* в браузере нельзя крутить свой цикл:
           браузер сам вызывает frame 20 раз в секунду */
        emscripten_set_main_loop(frame, 1000 / FRAME_MS, 1);
#else
        while (running) {
            frame();
            plat_sleep(FRAME_MS);
        }
#endif
    }
    plat_done();

    if (core_error()[0] != '\0') {
        printf("%s\n", core_error());
        return 1;
    }
    return 0;
}
