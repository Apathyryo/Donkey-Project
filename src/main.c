#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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
    plat_draw(core_screen());
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
        while (running) {
            frame();
            plat_sleep(FRAME_MS);
        }
    }
    plat_done();

    if (core_error()[0] != '\0') {
        printf("%s\n", core_error());
        return 1;
    }
    return 0;
}
