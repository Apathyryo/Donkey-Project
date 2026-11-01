#ifndef VM_H
#define VM_H

#include <stdint.h>

/* Размеры памяти ВМ (в ячейках). Ячейка всегда 32 бита,
   поэтому программа работает одинаково на 32- и 64-битных процессорах. */
#define CODE_SIZE  8192
#define STACK_SIZE 256

/* Команды ВМ */
enum {
    OP_HALT,   /* остановить ВМ */
    OP_PUSH,   /* положить на стек число из следующей ячейки кода */
    OP_DROP,   /* a -- */
    OP_DUP,    /* a -- a a */
    OP_SWAP,   /* a b -- b a */
    OP_OVER,   /* a b -- a b a */
    OP_ADD,    /* a b -- a+b */
    OP_SUB,    /* a b -- a-b */
    OP_MUL,    /* a b -- a*b */
    OP_DIV,    /* a b -- a/b */
    OP_MOD,    /* a b -- a%b */
    OP_COUNT   /* число команд */
};

/* Коды ошибок */
enum {
    VM_OK,
    VM_ERR_OVERFLOW,   /* стек переполнен */
    VM_ERR_UNDERFLOW,  /* на стеке не хватает чисел */
    VM_ERR_DIV_ZERO,   /* деление на ноль */
    VM_ERR_BAD_OP,     /* неизвестная команда */
    VM_ERR_BAD_ADDR    /* адрес за пределами кода */
};

typedef struct {
    int32_t code[CODE_SIZE];  /* байт-код */
    int code_len;             /* сколько ячеек кода занято */
    int32_t ds[STACK_SIZE];   /* стек данных */
    int sp;                   /* число элементов на стеке */
    int pc;                   /* адрес текущей команды */
    int err;                  /* код ошибки, VM_OK если всё хорошо */
} VM;

void vm_init(VM *vm);
void vm_emit(VM *vm, int32_t x);
void vm_push(VM *vm, int32_t x);
int32_t vm_pop(VM *vm);
int vm_run(VM *vm, int addr);
const char *vm_error_str(int err);

#endif
