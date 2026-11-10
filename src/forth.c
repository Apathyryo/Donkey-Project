#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "forth.h"

/* Встроенные слова языка и соответствующие им команды ВМ */
static const struct {
    const char *name;
    int op;
} builtins[] = {
    { "+", OP_ADD },
    { "-", OP_SUB },
    { "*", OP_MUL },
    { "/", OP_DIV },
    { "mod", OP_MOD },
    { "=", OP_EQ },
    { "<", OP_LT },
    { ">", OP_GT },
    { "and", OP_AND },
    { "or", OP_OR },
    { "not", OP_NOT },
    { "dup", OP_DUP },
    { "drop", OP_DROP },
    { "swap", OP_SWAP },
    { "over", OP_OVER },
    { "@", OP_LOAD },
    { "!", OP_STORE },
    { NULL, 0 }
};

static int is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

/* Записать сообщение об ошибке с номером строки */
static int error(Forth *f, const char *msg)
{
    sprintf(f->error, "line %d: %s '%s'", f->line, msg, f->token);
    return -1;
}

/* Прочитать следующее слово в f->token.
   Возвращает 0, если текст закончился. */
static int next_token(Forth *f)
{
    int len = 0;

    while (is_space(*f->pos)) {
        if (*f->pos == '\n')
            f->line++;
        f->pos++;
    }
    if (*f->pos == '\0')
        return 0;
    while (*f->pos != '\0' && !is_space(*f->pos)) {
        if (len < NAME_LEN - 1)
            f->token[len++] = *f->pos;
        f->pos++;
    }
    f->token[len] = '\0';
    return 1;
}

/* Пропустить комментарий: \ до конца строки или ( до ) */
static void skip_comment(Forth *f, char end)
{
    while (*f->pos != '\0' && *f->pos != end) {
        if (*f->pos == '\n')
            f->line++;
        f->pos++;
    }
    if (*f->pos == ')')
        f->pos++;
}

/* Является ли слово числом: необязательный минус и цифры */
static int is_number(const char *s)
{
    if (*s == '-')
        s++;
    if (*s == '\0')
        return 0;
    while (*s != '\0') {
        if (*s < '0' || *s > '9')
            return 0;
        s++;
    }
    return 1;
}

static int find_builtin(const char *name)
{
    int i;

    for (i = 0; builtins[i].name != NULL; i++)
        if (strcmp(builtins[i].name, name) == 0)
            return builtins[i].op;
    return -1;
}

/* Является ли слово символом в кавычках, например 'A' */
static int is_char(const char *s)
{
    return s[0] == '\'' && s[1] != '\0' && s[2] == '\'' && s[3] == '\0';
}

/* Найти слово в словаре, вернуть его номер или -1.
   Поиск идёт с конца, чтобы новое определение заменяло старое. */
static int find_word(Forth *f, const char *name)
{
    int i;

    for (i = f->nwords - 1; i >= 0; i--)
        if (strcmp(f->words[i].name, name) == 0)
            return i;
    return -1;
}

/* Найти слово в словаре, вернуть его адрес или -1 */
int forth_find(Forth *f, const char *name)
{
    int i = find_word(f, name);

    if (i < 0)
        return -1;
    return f->words[i].addr;
}

/* Прочитать имя и добавить слово в словарь */
static int add_word(Forth *f, int addr, int is_var)
{
    if (!next_token(f))
        return error(f, "missing name after");
    if (f->nwords >= MAX_WORDS)
        return error(f, "too many words");
    strcpy(f->words[f->nwords].name, f->token);
    f->words[f->nwords].addr = addr;
    f->words[f->nwords].is_var = is_var;
    f->nwords++;
    return 0;
}

/* Перевести исходный текст в байт-код.
   Возвращает 0 при успехе, -1 при ошибке (текст ошибки в f->error). */
int forth_compile(Forth *f, VM *vm, const char *src)
{
    int in_def = 0;  /* 1, если мы внутри определения : ... ; */
    int op, i;

    f->vm = vm;
    f->nwords = 0;
    f->here = 0;
    f->pos = src;
    f->line = 1;
    f->error[0] = '\0';

    while (next_token(f)) {
        if (strcmp(f->token, "\\") == 0) {
            skip_comment(f, '\n');
        } else if (strcmp(f->token, "(") == 0) {
            skip_comment(f, ')');
        } else if (strcmp(f->token, ":") == 0) {
            if (in_def)
                return error(f, "nested definition");
            if (add_word(f, vm->code_len, 0) != 0)
                return -1;
            in_def = 1;
        } else if (strcmp(f->token, ";") == 0) {
            if (!in_def)
                return error(f, "unexpected");
            vm_emit(vm, OP_RET);
            in_def = 0;
        } else if (strcmp(f->token, "variable") == 0) {
            /* переменная - это одна ячейка в памяти данных */
            if (in_def)
                return error(f, "not allowed inside definition");
            if (f->here >= DATA_SIZE)
                return error(f, "out of data memory at");
            if (add_word(f, f->here, 1) != 0)
                return -1;
            f->here++;
        } else if (!in_def) {
            return error(f, "code outside definition");
        } else if (is_number(f->token)) {
            vm_emit(vm, OP_PUSH);
            vm_emit(vm, (int32_t)strtol(f->token, NULL, 10));
        } else if (is_char(f->token)) {
            vm_emit(vm, OP_PUSH);
            vm_emit(vm, (unsigned char)f->token[1]);
        } else if ((op = find_builtin(f->token)) >= 0) {
            vm_emit(vm, op);
        } else if ((i = find_word(f, f->token)) >= 0) {
            /* переменная кладёт на стек свой адрес, слово вызывается */
            vm_emit(vm, f->words[i].is_var ? OP_PUSH : OP_CALL);
            vm_emit(vm, f->words[i].addr);
        } else {
            return error(f, "unknown word");
        }
        if (vm->err != VM_OK)
            return error(f, "program too big at");
    }
    if (in_def)
        return error(f, "missing ; after");
    return 0;
}
