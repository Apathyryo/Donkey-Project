/* Модульные тесты транслятора */
#include <string.h>
#include "check.h"
#include "../src/forth.h"

static VM vm;
static Forth forth;

/* Оттранслировать текст и выполнить слово main.
   Возвращает код ошибки ВМ или -1, если не удалось оттранслировать. */
static int run(const char *src)
{
    int addr;

    vm_init(&vm);
    if (forth_compile(&forth, &vm, src) != 0)
        return -1;
    addr = forth_find(&forth, "main");
    if (addr < 0)
        return -1;
    return vm_run(&vm, addr);
}

/* Вершина стека */
static int32_t top(void)
{
    return vm.ds[vm.sp - 1];
}

/* Есть ли подстрока в тексте ошибки */
static int error_has(const char *s)
{
    return strstr(forth.error, s) != NULL;
}

static void test_numbers(void)
{
    CHECK(run(": main 42 ;") == VM_OK && top() == 42);
    CHECK(run(": main -7 3 + ;") == VM_OK && top() == -4);
    CHECK(run(": main 1 2 3 ;") == VM_OK && vm.sp == 3);
    /* одиночный минус - это не число, а вычитание */
    CHECK(run(": main 5 2 - ;") == VM_OK && top() == 3);
}

static void test_builtins(void)
{
    CHECK(run(": main 6 7 * ;") == VM_OK && top() == 42);
    CHECK(run(": main 17 5 / ;") == VM_OK && top() == 3);
    CHECK(run(": main 17 5 mod ;") == VM_OK && top() == 2);
    CHECK(run(": main 3 3 = ;") == VM_OK && top() == 1);
    CHECK(run(": main 3 4 < ;") == VM_OK && top() == 1);
    CHECK(run(": main 3 4 > ;") == VM_OK && top() == 0);
    CHECK(run(": main 6 3 and ;") == VM_OK && top() == 2);
    CHECK(run(": main 6 3 or ;") == VM_OK && top() == 7);
    CHECK(run(": main 0 not ;") == VM_OK && top() == 1);
    CHECK(run(": main 1 2 swap ;") == VM_OK && top() == 1);
    CHECK(run(": main 1 2 over ;") == VM_OK && top() == 1 && vm.sp == 3);
    CHECK(run(": main 1 dup + ;") == VM_OK && top() == 2);
    CHECK(run(": main 1 2 drop ;") == VM_OK && top() == 1 && vm.sp == 1);
    CHECK(run(": main 99 5 ! 5 @ ;") == VM_OK && top() == 99);
}

static void test_words(void)
{
    CHECK(run(": square dup * ; : main 5 square ;") == VM_OK && top() == 25);
    CHECK(run(": a 1 ; : b a a + ; : main b b + ;") == VM_OK && top() == 4);
    /* новое определение заменяет старое */
    CHECK(run(": f 1 ; : f 2 ; : main f ;") == VM_OK && top() == 2);
    CHECK(run(": main 1 ; : other 2 ;") == VM_OK && vm.sp == 1);
    CHECK(forth_find(&forth, "other") >= 0);
    CHECK(forth_find(&forth, "nothing") == -1);
}

static void test_variables(void)
{
    CHECK(run("variable x : main 5 x ! x @ ;") == VM_OK && top() == 5);
    /* переменные лежат в соседних ячейках */
    CHECK(run("variable a variable b : main a b ;") == VM_OK
          && vm.ds[0] == 0 && vm.ds[1] == 1);
    CHECK(run("variable n : inc n @ 1 + n ! ; : main inc inc inc n @ ;")
          == VM_OK && top() == 3);
    CHECK(run(": main variable x ;") == -1 && error_has("inside definition"));
    CHECK(run("variable") == -1 && error_has("missing name"));
}

static void test_chars(void)
{
    CHECK(run(": main 'A' ;") == VM_OK && top() == 65);
    CHECK(run(": main '#' ;") == VM_OK && top() == '#');
    CHECK(run(": main 'AB' ;") == -1 && error_has("unknown word"));
}

static void test_comments(void)
{
    CHECK(run("\\ comment\n: main ( n -- ) 1 \\ one\n 2 + ;") == VM_OK
          && top() == 3);
    CHECK(run("( multi\nline ) : main 5 ;") == VM_OK && top() == 5);
}

static void test_errors(void)
{
    CHECK(run(": main foo ;") == -1 && error_has("unknown word 'foo'"));
    CHECK(run("1 2 +") == -1 && error_has("outside definition"));
    CHECK(run(": main 1") == -1 && error_has("missing ;"));
    CHECK(run(": main : x ;") == -1 && error_has("nested"));
    CHECK(run(";") == -1 && error_has("unexpected"));
    CHECK(run(":") == -1 && error_has("missing name"));
    /* номер строки в сообщении */
    CHECK(run(": main\n1\nbar ;") == -1 && error_has("line 3"));
}

int main(void)
{
    test_numbers();
    test_builtins();
    test_words();
    test_variables();
    test_chars();
    test_comments();
    test_errors();

    printf("forth: %d tests, %d failed\n", total, failed);
    return failed != 0;
}
