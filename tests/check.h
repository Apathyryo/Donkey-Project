#ifndef CHECK_H
#define CHECK_H

/* Простейший фреймворк для тестов: CHECK считает проверки
   и печатает те, что не прошли */

#include <stdio.h>

static int total = 0;
static int failed = 0;

#define CHECK(cond) do { \
    total++; \
    if (!(cond)) { \
        failed++; \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

#endif
