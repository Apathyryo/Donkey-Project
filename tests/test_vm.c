/* Модульные тесты виртуальной машины */
#include "check.h"
#include "../src/vm.h"

static VM vm;

/* Загрузить программу в ВМ и выполнить её с адреса 0 */
static int run(const int32_t *prog, int n)
{
    int i;

    vm_init(&vm);
    for (i = 0; i < n; i++)
        vm_emit(&vm, prog[i]);
    return vm_run(&vm, 0);
}

#define RUN(prog) run(prog, sizeof(prog) / sizeof(prog[0]))

/* Вершина стека */
static int32_t top(void)
{
    return vm.ds[vm.sp - 1];
}

static void test_arith(void)
{
    int32_t add[] = { OP_PUSH, 2, OP_PUSH, 3, OP_ADD, OP_HALT };
    int32_t sub[] = { OP_PUSH, 2, OP_PUSH, 3, OP_SUB, OP_HALT };
    int32_t mul[] = { OP_PUSH, -4, OP_PUSH, 3, OP_MUL, OP_HALT };
    int32_t div[] = { OP_PUSH, 17, OP_PUSH, 5, OP_DIV, OP_HALT };
    int32_t mod[] = { OP_PUSH, 17, OP_PUSH, 5, OP_MOD, OP_HALT };

    CHECK(RUN(add) == VM_OK && top() == 5);
    CHECK(RUN(sub) == VM_OK && top() == -1);
    CHECK(RUN(mul) == VM_OK && top() == -12);
    CHECK(RUN(div) == VM_OK && top() == 3);
    CHECK(RUN(mod) == VM_OK && top() == 2);
}

static void test_stack(void)
{
    int32_t dup[] = { OP_PUSH, 7, OP_DUP, OP_HALT };
    int32_t drop[] = { OP_PUSH, 1, OP_PUSH, 2, OP_DROP, OP_HALT };
    int32_t swap[] = { OP_PUSH, 1, OP_PUSH, 2, OP_SWAP, OP_HALT };
    int32_t over[] = { OP_PUSH, 1, OP_PUSH, 2, OP_OVER, OP_HALT };

    CHECK(RUN(dup) == VM_OK && vm.sp == 2 && vm.ds[0] == 7 && vm.ds[1] == 7);
    CHECK(RUN(drop) == VM_OK && vm.sp == 1 && top() == 1);
    CHECK(RUN(swap) == VM_OK && vm.ds[0] == 2 && vm.ds[1] == 1);
    CHECK(RUN(over) == VM_OK && vm.sp == 3 && top() == 1);
}

static void test_logic(void)
{
    int32_t eq[] = { OP_PUSH, 4, OP_PUSH, 4, OP_EQ, OP_HALT };
    int32_t lt[] = { OP_PUSH, 3, OP_PUSH, 4, OP_LT, OP_HALT };
    int32_t gt[] = { OP_PUSH, 3, OP_PUSH, 4, OP_GT, OP_HALT };
    int32_t and[] = { OP_PUSH, 6, OP_PUSH, 3, OP_AND, OP_HALT };
    int32_t or[] = { OP_PUSH, 6, OP_PUSH, 3, OP_OR, OP_HALT };
    int32_t not[] = { OP_PUSH, 0, OP_NOT, OP_HALT };

    CHECK(RUN(eq) == VM_OK && top() == 1);
    CHECK(RUN(lt) == VM_OK && top() == 1);
    CHECK(RUN(gt) == VM_OK && top() == 0);
    CHECK(RUN(and) == VM_OK && top() == 2);
    CHECK(RUN(or) == VM_OK && top() == 7);
    CHECK(RUN(not) == VM_OK && top() == 1);
}

static void test_memory(void)
{
    /* data[10] = 42, затем прочитать data[10] */
    int32_t prog[] = { OP_PUSH, 42, OP_PUSH, 10, OP_STORE,
                       OP_PUSH, 10, OP_LOAD, OP_HALT };

    CHECK(RUN(prog) == VM_OK && vm.sp == 1 && top() == 42);
    CHECK(vm.data[10] == 42);
}

static void test_jumps(void)
{
    /* сумма чисел от 1 до 10: на стеке сумма и счётчик */
    int32_t loop[] = {
        OP_PUSH, 0, OP_PUSH, 10,       /* 0: sum=0 i=10 */
        OP_DUP, OP_JZ, 18,             /* 4: если i == 0, выход */
        OP_SWAP, OP_OVER, OP_ADD,      /* 7: sum += i */
        OP_SWAP, OP_PUSH, 1, OP_SUB,   /* 10: i -= 1 */
        OP_JMP, 4,                     /* 14 */
        OP_HALT, OP_HALT,              /* 16 */
        OP_DROP, OP_HALT               /* 18 */
    };

    CHECK(RUN(loop) == VM_OK && vm.sp == 1 && top() == 55);
}

static void test_call(void)
{
    /* square(5) */
    int32_t call[] = { OP_PUSH, 5, OP_CALL, 5, OP_HALT,
                       OP_DUP, OP_MUL, OP_RET };
    /* OP_RET в начальном слове завершает выполнение */
    int32_t ret[] = { OP_PUSH, 1, OP_RET, OP_PUSH, 2, OP_HALT };

    CHECK(RUN(call) == VM_OK && top() == 25 && vm.rp == 0);
    CHECK(RUN(ret) == VM_OK && vm.sp == 1 && top() == 1);
}

static void test_errors(void)
{
    int32_t underflow[] = { OP_ADD, OP_HALT };
    int32_t div_zero[] = { OP_PUSH, 1, OP_PUSH, 0, OP_DIV, OP_HALT };
    int32_t bad_op[] = { 1000 };
    int32_t bad_load[] = { OP_PUSH, DATA_SIZE, OP_LOAD, OP_HALT };
    int32_t bad_store[] = { OP_PUSH, 1, OP_PUSH, -1, OP_STORE, OP_HALT };
    int32_t bad_jump[] = { OP_JMP, 100 };
    int32_t no_halt[] = { OP_PUSH, 1 };
    /* бесконечно кладём числа на стек */
    int32_t overflow[] = { OP_PUSH, 1, OP_JMP, 0 };
    /* бесконечная рекурсия */
    int32_t recursion[] = { OP_CALL, 0 };

    CHECK(RUN(underflow) == VM_ERR_UNDERFLOW);
    CHECK(RUN(div_zero) == VM_ERR_DIV_ZERO);
    CHECK(RUN(bad_op) == VM_ERR_BAD_OP);
    CHECK(RUN(bad_load) == VM_ERR_BAD_ADDR);
    CHECK(RUN(bad_store) == VM_ERR_BAD_ADDR);
    CHECK(RUN(bad_jump) == VM_ERR_BAD_ADDR);
    CHECK(RUN(no_halt) == VM_ERR_BAD_ADDR);
    CHECK(RUN(overflow) == VM_ERR_OVERFLOW);
    CHECK(RUN(recursion) == VM_ERR_OVERFLOW);
}

int main(void)
{
    test_arith();
    test_stack();
    test_logic();
    test_memory();
    test_jumps();
    test_call();
    test_errors();

    printf("vm: %d tests, %d failed\n", total, failed);
    return failed != 0;
}
