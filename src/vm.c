#include "vm.h"

void vm_init(VM *vm)
{
    int i;

    for (i = 0; i < DATA_SIZE; i++)
        vm->data[i] = 0;
    vm->code_len = 0;
    vm->sp = 0;
    vm->rp = 0;
    vm->pc = 0;
    vm->err = VM_OK;
}

/* Добавить ячейку в конец кода */
void vm_emit(VM *vm, int32_t x)
{
    if (vm->code_len >= CODE_SIZE) {
        vm->err = VM_ERR_BAD_ADDR;
        return;
    }
    vm->code[vm->code_len++] = x;
}

void vm_push(VM *vm, int32_t x)
{
    if (vm->sp >= STACK_SIZE) {
        vm->err = VM_ERR_OVERFLOW;
        return;
    }
    vm->ds[vm->sp++] = x;
}

int32_t vm_pop(VM *vm)
{
    if (vm->sp <= 0) {
        vm->err = VM_ERR_UNDERFLOW;
        return 0;
    }
    return vm->ds[--vm->sp];
}

/* Прочитать следующую ячейку кода */
static int32_t fetch(VM *vm)
{
    if (vm->pc < 0 || vm->pc >= vm->code_len) {
        vm->err = VM_ERR_BAD_ADDR;
        return OP_HALT;
    }
    return vm->code[vm->pc++];
}

/* Выполнять команды с адреса addr до OP_HALT, до выхода
   из начального слова по OP_RET или до ошибки */
int vm_run(VM *vm, int addr)
{
    int32_t a, b;

    vm->pc = addr;
    vm->rp = 0;
    vm->err = VM_OK;
    while (vm->err == VM_OK) {
        int32_t op = fetch(vm);
        switch (op) {
        case OP_HALT:
            return vm->err;
        case OP_PUSH:
            a = fetch(vm);
            vm_push(vm, a);
            break;
        case OP_DROP:
            vm_pop(vm);
            break;
        case OP_DUP:
            a = vm_pop(vm);
            vm_push(vm, a);
            vm_push(vm, a);
            break;
        case OP_SWAP:
            b = vm_pop(vm);
            a = vm_pop(vm);
            vm_push(vm, b);
            vm_push(vm, a);
            break;
        case OP_OVER:
            b = vm_pop(vm);
            a = vm_pop(vm);
            vm_push(vm, a);
            vm_push(vm, b);
            vm_push(vm, a);
            break;
        case OP_ADD:
            b = vm_pop(vm);
            a = vm_pop(vm);
            vm_push(vm, a + b);
            break;
        case OP_SUB:
            b = vm_pop(vm);
            a = vm_pop(vm);
            vm_push(vm, a - b);
            break;
        case OP_MUL:
            b = vm_pop(vm);
            a = vm_pop(vm);
            vm_push(vm, a * b);
            break;
        case OP_DIV:
        case OP_MOD:
            b = vm_pop(vm);
            a = vm_pop(vm);
            if (vm->err != VM_OK)
                break;
            if (b == 0) {
                vm->err = VM_ERR_DIV_ZERO;
                break;
            }
            vm_push(vm, op == OP_DIV ? a / b : a % b);
            break;
        case OP_EQ:
            b = vm_pop(vm);
            a = vm_pop(vm);
            vm_push(vm, a == b);
            break;
        case OP_LT:
            b = vm_pop(vm);
            a = vm_pop(vm);
            vm_push(vm, a < b);
            break;
        case OP_GT:
            b = vm_pop(vm);
            a = vm_pop(vm);
            vm_push(vm, a > b);
            break;
        case OP_AND:
            b = vm_pop(vm);
            a = vm_pop(vm);
            vm_push(vm, a & b);
            break;
        case OP_OR:
            b = vm_pop(vm);
            a = vm_pop(vm);
            vm_push(vm, a | b);
            break;
        case OP_NOT:
            a = vm_pop(vm);
            vm_push(vm, a == 0);
            break;
        case OP_LOAD:
            a = vm_pop(vm);
            if (vm->err != VM_OK)
                break;
            if (a < 0 || a >= DATA_SIZE) {
                vm->err = VM_ERR_BAD_ADDR;
                break;
            }
            vm_push(vm, vm->data[a]);
            break;
        case OP_STORE:
            a = vm_pop(vm);
            b = vm_pop(vm);
            if (vm->err != VM_OK)
                break;
            if (a < 0 || a >= DATA_SIZE) {
                vm->err = VM_ERR_BAD_ADDR;
                break;
            }
            vm->data[a] = b;
            break;
        case OP_JMP:
            vm->pc = fetch(vm);
            break;
        case OP_JZ:
            a = fetch(vm);
            if (vm_pop(vm) == 0)
                vm->pc = a;
            break;
        case OP_CALL:
            a = fetch(vm);
            if (vm->rp >= STACK_SIZE) {
                vm->err = VM_ERR_OVERFLOW;
                break;
            }
            vm->rs[vm->rp++] = vm->pc;
            vm->pc = a;
            break;
        case OP_RET:
            /* стек возвратов пуст - значит, закончилось слово,
               с которого начали выполнение */
            if (vm->rp == 0)
                return vm->err;
            vm->pc = vm->rs[--vm->rp];
            break;
        default:
            vm->err = VM_ERR_BAD_OP;
            break;
        }
    }
    return vm->err;
}

const char *vm_error_str(int err)
{
    switch (err) {
    case VM_OK:            return "ok";
    case VM_ERR_OVERFLOW:  return "stack overflow";
    case VM_ERR_UNDERFLOW: return "stack underflow";
    case VM_ERR_DIV_ZERO:  return "division by zero";
    case VM_ERR_BAD_OP:    return "bad opcode";
    case VM_ERR_BAD_ADDR:  return "bad address";
    }
    return "unknown error";
}
