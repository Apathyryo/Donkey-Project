\ Объезд - игровая логика
\ Ядро вызывает init один раз и tick каждый кадр.
\ Экран 40x22. Трасса: обочины в столбцах 13 и 25,
\ три полосы по 3 символа, между ними разметка в столбцах 17 и 21.

variable dist      \ пройденный путь (растёт каждый кадр)
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

: init
    0 dist ! ;

: tick
    dist @ 1 + dist !
    cls
    road ;
