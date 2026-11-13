#ifndef PLATFORM_H
#define PLATFORM_H

/* Всё, что зависит от ОС. Для каждой платформы свой файл plat_*.c */

/* Коды клавиш-стрелок (остальные клавиши - обычные коды символов) */
#define KEY_LEFT  1
#define KEY_RIGHT 2
#define KEY_UP    3
#define KEY_DOWN  4
#define KEY_ESC   27

void plat_init(void);
void plat_done(void);
int plat_key(void);                   /* нажатая клавиша или 0 */
void plat_draw(const char *screen);   /* вывести экран SCREEN_W x SCREEN_H */
void plat_sleep(int ms);

#endif
