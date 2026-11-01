#include <stdio.h>
#include "vm.h"

static VM vm;

int main(void)
{
    int err;

    /* Пока что проверяем ВМ на простой программе: (2 + 3) * 4 */
    vm_init(&vm);
    vm_emit(&vm, OP_PUSH);
    vm_emit(&vm, 2);
    vm_emit(&vm, OP_PUSH);
    vm_emit(&vm, 3);
    vm_emit(&vm, OP_ADD);
    vm_emit(&vm, OP_PUSH);
    vm_emit(&vm, 4);
    vm_emit(&vm, OP_MUL);
    vm_emit(&vm, OP_HALT);

    err = vm_run(&vm, 0);
    if (err != VM_OK) {
        printf("VM error: %s\n", vm_error_str(err));
        return 1;
    }
    printf("Result: %d\n", (int)vm_pop(&vm));
    return 0;
}
