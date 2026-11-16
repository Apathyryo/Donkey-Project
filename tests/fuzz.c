/* Фаззинг: подаём ВМ и транслятору случайные программы
   и проверяем, что они не падают, не зависают и не портят память.
   Запуск: run_fuzz [seed] - при одинаковом seed входы одинаковые. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../src/vm.h"
#include "../src/forth.h"

#define RUNS 100000

static VM vm;
static Forth forth;
static int bad = 0;

/* Случайное число: обычно небольшое, иногда граничное */
static int32_t rnd_value(void)
{
    switch (rand() % 8) {
    case 0: return 0;
    case 1: return -1;
    case 2: return INT32_MAX;
    case 3: return INT32_MIN;
    default: return rand() % 64 - 16;
    }
}

/* Системный вызов для фаззинга: просто кладёт случайное число */
static void fake_sys(VM *v, int n)
{
    (void)n;
    vm_push(v, rnd_value());
}

/* После работы ВМ её состояние должно быть корректным */
static void check_vm(int err, const char *what, int run)
{
    if (err < VM_OK || err > VM_ERR_STEPS
        || vm.sp < 0 || vm.sp > STACK_SIZE
        || vm.rp < 0 || vm.rp > STACK_SIZE) {
        printf("BAD %s run %d: err=%d sp=%d rp=%d\n",
               what, run, err, vm.sp, vm.rp);
        bad++;
    }
}

/* Случайная программа из команд ВМ, иногда с неверными кодами команд */
static void fuzz_vm(int run)
{
    int i, n = 1 + rand() % 64;

    vm_init(&vm);
    vm.sys = fake_sys;
    vm.max_steps = 10000;  /* чтобы бесконечные циклы быстро обрывались */
    for (i = 0; i < n; i++) {
        if (rand() % 4 == 0)
            vm_emit(&vm, rnd_value());
        else
            vm_emit(&vm, rand() % (OP_COUNT + 2));
    }
    check_vm(vm_run(&vm, 0), "vm", run);
}

/* Слова, из которых собираются случайные тексты для транслятора */
static const char *tokens[] = {
    ":", ";", "main", "foo", "variable", "array", "x",
    "if", "else", "then", "begin", "until", "s\" ab", "\"",
    "(", ")", "\\", "\n", "dup", "drop", "swap", "over",
    "+", "-", "*", "/", "mod", "=", "<", ">", "and", "or", "not",
    "@", "!", "0", "1", "-1", "5", "100", "2147483647", "-2147483648",
    "99999999999", "'A'", "'", "key", "rnd", "num", "put", "cls", "text"
};

#define NTOKENS (int)(sizeof(tokens) / sizeof(tokens[0]))

/* Случайный текст программы; если удалось оттранслировать -
   выполняем все слова из словаря */
static void fuzz_forth(int run)
{
    static char src[2048];
    int i, n = 1 + rand() % 40;

    src[0] = '\0';
    if (rand() % 2)
        strcat(src, ": main ");
    for (i = 0; i < n; i++) {
        strcat(src, tokens[rand() % NTOKENS]);
        strcat(src, " ");
    }
    if (rand() % 2)
        strcat(src, ";");

    vm_init(&vm);
    vm.sys = fake_sys;
    vm.max_steps = 10000;
    if (forth_compile(&forth, &vm, src) != 0)
        return;
    for (i = 0; i < forth.nwords; i++) {
        if (forth.words[i].is_var)
            continue;
        vm.sp = 0;
        check_vm(vm_run(&vm, forth.words[i].addr), "forth", run);
    }
}

int main(int argc, char **argv)
{
    unsigned seed = (unsigned)time(NULL);
    int i;

    if (argc > 1)
        seed = (unsigned)strtoul(argv[1], NULL, 10);
    printf("fuzz: seed %u\n", seed);
    srand(seed);

    for (i = 0; i < RUNS; i++)
        fuzz_vm(i);
    for (i = 0; i < RUNS; i++)
        fuzz_forth(i);

    printf("fuzz: %d vm + %d forth programs, %d bad\n", RUNS, RUNS, bad);
    return bad != 0;
}
