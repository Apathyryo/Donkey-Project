#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm.h"
#include "forth.h"
#include "core.h"

static VM vm;
static Forth forth;
static char src[65536];                   /* текст скрипта */
static char screen[SCREEN_H * SCREEN_W];  /* экран: строки подряд */
static char error[200];                   /* текст последней ошибки */
static int key;                           /* клавиша, нажатая в этом кадре */
static int init_addr, tick_addr;          /* адреса слов init и tick */

/* Поставить символ на экран, если координаты внутри экрана */
static void put(int x, int y, int ch)
{
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H)
        screen[y * SCREEN_W + x] = (char)ch;
}

/* Выполнить системный вызов номер n */
static void sys(VM *v, int n)
{
    int32_t x, y, a, len, i;
    char buf[16];

    switch (n) {
    case SYS_CLS:
        memset(screen, ' ', sizeof(screen));
        break;
    case SYS_PUT:
        a = vm_pop(v);
        y = vm_pop(v);
        x = vm_pop(v);
        put(x, y, a);
        break;
    case SYS_TEXT:
        len = vm_pop(v);
        a = vm_pop(v);
        y = vm_pop(v);
        x = vm_pop(v);
        for (i = 0; i < len; i++)
            if (a + i >= 0 && a + i < DATA_SIZE)
                put(x + i, y, v->data[a + i]);
        break;
    case SYS_NUM:
        a = vm_pop(v);
        y = vm_pop(v);
        x = vm_pop(v);
        sprintf(buf, "%d", (int)a);
        for (i = 0; buf[i] != '\0'; i++)
            put(x + i, y, buf[i]);
        break;
    case SYS_KEY:
        vm_push(v, key);
        break;
    case SYS_RND:
        a = vm_pop(v);
        vm_push(v, a > 0 ? rand() % a : 0);
        break;
    default:
        v->err = VM_ERR_BAD_OP;
        break;
    }
}

/* Прочитать и оттранслировать скрипт.
   Скрипт должен определить слова init (вызывается один раз)
   и tick (вызывается каждый кадр). */
int core_load(const char *path)
{
    FILE *f = fopen(path, "rb");
    size_t n;

    if (f == NULL) {
        sprintf(error, "can't open %s", path);
        return -1;
    }
    n = fread(src, 1, sizeof(src) - 1, f);
    src[n] = '\0';
    fclose(f);

    vm_init(&vm);
    vm.sys = sys;
    if (forth_compile(&forth, &vm, src) != 0) {
        sprintf(error, "%s: %s", path, forth.error);
        return -1;
    }
    init_addr = forth_find(&forth, "init");
    tick_addr = forth_find(&forth, "tick");
    if (init_addr < 0 || tick_addr < 0) {
        sprintf(error, "%s: words init and tick are required", path);
        return -1;
    }
    memset(screen, ' ', sizeof(screen));
    return 0;
}

/* Выполнить слово скрипта. Если на стеке что-то осталось - отбрасываем */
static int run(int addr)
{
    if (vm_run(&vm, addr) != VM_OK) {
        sprintf(error, "VM error: %s", vm_error_str(vm.err));
        return -1;
    }
    vm.sp = 0;
    return 0;
}

int core_init(void)
{
    return run(init_addr);
}

/* Один кадр игры: передать клавишу скрипту и выполнить tick */
int core_tick(int k)
{
    key = k;
    return run(tick_addr);
}

const char *core_screen(void)
{
    return screen;
}

const char *core_error(void)
{
    return error;
}
