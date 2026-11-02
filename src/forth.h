#ifndef FORTH_H
#define FORTH_H

#include "vm.h"

/* Транслятор Forth-подобного языка в байт-код ВМ */

#define MAX_WORDS 256
#define NAME_LEN  32

/* Слово, определённое в программе через : имя ... ; */
typedef struct {
    char name[NAME_LEN];
    int addr;              /* адрес начала слова в коде ВМ */
} Word;

typedef struct {
    VM *vm;
    Word words[MAX_WORDS]; /* словарь */
    int nwords;
    const char *pos;       /* текущая позиция в исходном тексте */
    int line;              /* номер текущей строки (для сообщений об ошибках) */
    char token[NAME_LEN];  /* последнее прочитанное слово */
    char error[128];       /* текст ошибки */
} Forth;

int forth_compile(Forth *f, VM *vm, const char *src);
int forth_find(Forth *f, const char *name);

#endif
