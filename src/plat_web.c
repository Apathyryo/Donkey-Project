/* Платформа Web (Emscripten): экран - текст в элементе <pre> страницы,
   клавиши - события клавиатуры браузера */
#include <string.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include "platform.h"
#include "core.h"

static int last_key;

/* Браузер вызывает эту функцию при нажатии клавиши */
static EM_BOOL on_key(int type, const EmscriptenKeyboardEvent *e, void *data)
{
    (void)type;
    (void)data;
    if (strcmp(e->key, "ArrowLeft") == 0)
        last_key = KEY_LEFT;
    else if (strcmp(e->key, "ArrowRight") == 0)
        last_key = KEY_RIGHT;
    else if (strcmp(e->key, "ArrowUp") == 0)
        last_key = KEY_UP;
    else if (strcmp(e->key, "ArrowDown") == 0)
        last_key = KEY_DOWN;
    else if (strlen(e->key) == 1)
        last_key = e->key[0];
    else
        return EM_FALSE;
    return EM_TRUE;  /* чтобы стрелки не прокручивали страницу */
}

void plat_init(void)
{
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT,
                                    NULL, EM_TRUE, on_key);
}

void plat_done(void)
{
}

int plat_key(void)
{
    int k = last_key;

    last_key = 0;
    return k;
}

void plat_draw(const char *screen)
{
    static char text[SCREEN_H * (SCREEN_W + 1) + 1];
    int y, n = 0;

    /* склеиваем строки экрана через перевод строки */
    for (y = 0; y < SCREEN_H; y++) {
        memcpy(text + n, screen + y * SCREEN_W, SCREEN_W);
        n += SCREEN_W;
        text[n++] = '\n';
    }
    text[n] = '\0';
    EM_ASM({
        document.getElementById('screen').textContent = UTF8ToString($0);
    }, text);
}

/* В браузере паузу между кадрами делает сам браузер */
void plat_sleep(int ms)
{
    (void)ms;
}
