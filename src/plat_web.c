/* Платформа Web (Emscripten): экран - текст в элементе <pre> страницы,
   клавиши - события клавиатуры браузера */
#include <stdio.h>
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

/* Цвета COLOR_... для браузера */
static const char *css[COLOR_COUNT] = {
    "#888", "#f55", "#5d5", "#fd5", "#68f", "#f6f", "#5ee", "#ddd"
};

void plat_draw(const char *screen, const char *colors)
{
    /* с запасом: на каждую клетку может прийти свой <span> */
    static char html[SCREEN_H * SCREEN_W * 40];
    int i, y, n = 0, last = -1;

    /* строим HTML: кусок одного цвета - в своём <span> */
    for (y = 0; y < SCREEN_H; y++) {
        for (i = y * SCREEN_W; i < (y + 1) * SCREEN_W; i++) {
            if (colors[i] != last) {
                if (last >= 0)
                    n += sprintf(html + n, "</span>");
                last = colors[i];
                n += sprintf(html + n, "<span style=\"color:%s\">", css[last]);
            }
            /* < > & в HTML - служебные символы */
            if (screen[i] == '<')
                n += sprintf(html + n, "&lt;");
            else if (screen[i] == '>')
                n += sprintf(html + n, "&gt;");
            else if (screen[i] == '&')
                n += sprintf(html + n, "&amp;");
            else
                html[n++] = screen[i];
        }
        html[n++] = '\n';
    }
    n += sprintf(html + n, "</span>");
    html[n] = '\0';
    EM_ASM({
        document.getElementById('screen').innerHTML = UTF8ToString($0);
    }, html);
}

/* В браузере паузу между кадрами делает сам браузер */
void plat_sleep(int ms)
{
    (void)ms;
}
