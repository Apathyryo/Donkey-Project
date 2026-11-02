#include <stdio.h>
#include "vm.h"

static VM vm;

int main(void)
{
    int err;

    /* Пока что проверяем ВМ на простой программе:
       подпрограмма square возводит число в квадрат, вызываем её для 5 */
    vm_init(&vm);
    vm_emit(&vm, OP_PUSH);  /* 0 */
    vm_emit(&vm, 5);
    vm_emit(&vm, OP_CALL);  /* 2 */
    vm_emit(&vm, 5);
    vm_emit(&vm, OP_HALT);  /* 4 */
    vm_emit(&vm, OP_DUP);   /* 5: square */
    vm_emit(&vm, OP_MUL);
    vm_emit(&vm, OP_RET);

    err = vm_run(&vm, 0);
    if (err != VM_OK) {
        printf("VM error: %s\n", vm_error_str(err));
        return 1;
    }
    printf("Result: %d\n", (int)vm_pop(&vm));
    return 0;
}
