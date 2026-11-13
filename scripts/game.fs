\ Проверка ядра: символ @ двигается стрелками
\ Коды клавиш: 1 - влево, 2 - вправо, 3 - вверх, 4 - вниз

variable x
variable y

: init 20 x ! 10 y ! ;

: move ( key -- )
    dup 1 = if x @ 1 - x ! then
    dup 2 = if x @ 1 + x ! then
    dup 3 = if y @ 1 - y ! then
    4 = if y @ 1 + y ! then ;

: tick
    key move
    cls
    0 0 s" Arrows - move, Esc - exit" text
    0 1 s" x =" text 4 1 x @ num
    x @ y @ '@' put ;
