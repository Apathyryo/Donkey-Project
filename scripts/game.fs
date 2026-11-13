\ Объезд - игровая логика
\ Ядро вызывает init один раз и tick каждый кадр.
\ Экран 40x22. Трасса: обочины в столбцах 13 и 25,
\ три полосы по 3 символа, между ними разметка в столбцах 17 и 21.

variable dist      \ пройденный путь (растёт каждый кадр)
variable lane      \ полоса игрока: 0, 1 или 2
variable i         \ счётчик для циклов
variable lx        \ столбец для vline и dashes
variable lch       \ символ для vline

: road-left 13 ;
: road-right 25 ;

\ сплошная линия символа ch в столбце x на всю высоту экрана
: vline ( x ch -- )
    lch ! lx !
    0 i !
    begin
        lx @ i @ lch @ put
        i @ 1 + i !
        i @ 21 >
    until ;

\ рисовать ли штрих разметки в строке y:
\ штрихи по 2 строки через 2, сдвигаются вниз вместе с dist
: dash? ( y -- f )
    dist @ 4 mod - 4 + 4 mod 2 < ;

\ прерывистая разметка в столбце x
: dashes ( x -- )
    lx !
    0 i !
    begin
        i @ dash? if lx @ i @ '|' put then
        i @ 1 + i !
        i @ 21 >
    until ;

: road
    road-left '#' vline
    road-right '#' vline
    17 dashes
    21 dashes ;

\ столбец середины полосы n: 15, 19 или 23
: lane-x ( n -- x ) 4 * 15 + ;

\ верхняя строка машины игрока
: car-y 18 ;

\ машина игрока 3x3:  /^\
\                     [#]
\                     o-o
: player
    lane @ lane-x 1 -
    dup car-y s" /^\" text
    dup car-y 1 + s" [#]" text
    car-y 2 + s" o-o" text ;

\ стрелки влево и вправо меняют полосу
: control
    key
    dup 1 = lane @ 0 > and if lane @ 1 - lane ! then
    2 = lane @ 2 < and if lane @ 1 + lane ! then ;

\ справа от трассы - название и пройденный путь,
\ слева - подсказка по клавишам
: panel
    28 1 s" DETOUR" text
    28 3 s" DIST" text
    28 4 dist @ num
    0 19 s" <- -> lane" text
    0 20 s" Esc   exit" text ;

: init
    0 dist !
    1 lane ! ;

: tick
    control
    dist @ 1 + dist !
    cls
    road
    player
    panel ;
