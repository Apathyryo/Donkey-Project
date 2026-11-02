#include <stdio.h>
#include "vm.h"
#include "forth.h"

static VM vm;
static Forth forth;

/* Пока что проверяем транслятор на небольшой программе */
static const char *demo =
    "\\ квадрат числа\n"
    ": square ( n -- n*n ) dup * ;\n"
    ": main 5 square 1 + ;\n";

int main(void)
{
    int addr, err;

    vm_init(&vm);
    if (forth_compile(&forth, &vm, demo) != 0) {
        printf("Compile error: %s\n", forth.error);
        return 1;
    }
    addr = forth_find(&forth, "main");
    if (addr < 0) {
        printf("Word 'main' not found\n");
        return 1;
    }

    err = vm_run(&vm, addr);
    if (err != VM_OK) {
        printf("VM error: %s\n", vm_error_str(err));
        return 1;
    }
    printf("Result: %d\n", (int)vm_pop(&vm));
    return 0;
}
