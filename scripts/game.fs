\ Объезд - игровая логика
\ Ядро вызывает init один раз и tick каждый кадр.
\ Экран 40x22. Трасса: обочины в столбцах 13 и 25,
\ три полосы по 3 символа, между ними разметка в столбцах 17 и 21.

variable dist      \ пройденный путь (растёт на каждом шаге)
variable lane      \ полоса игрока: 0, 1 или 2
variable frame     \ номер кадра
variable i         \ счётчик для циклов рисования
variable j         \ счётчик для циклов по объектам
variable lx        \ столбец для vline и dashes
variable lch       \ символ для vline

\ встречные объекты: у каждого полоса, строка и вид
\ вид: 0 - объекта нет, 1 - машина, 2 - камень, 3 - канистра
array ol 6
array oy 6
array ok 6
variable gap       \ сколько шагов до появления следующего объекта
variable lives     \ жизни
variable fuel      \ бензин, от 0 до 99
variable ended     \ 1 - игра окончена

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

\ ----- встречные объекты -----

: max-obj 6 ;

\ адреса полей объекта n
: olane ( n -- addr ) ol + ;
: row ( n -- addr ) oy + ;
: kind ( n -- addr ) ok + ;

\ перейти к следующему объекту; 1, если объекты закончились
: next-j ( -- f )
    j @ 1 + j !
    j @ max-obj < not ;

: clear-objects
    0 j !
    begin 0 j @ kind ! next-j until ;

\ занять свободный объект: машина, камень или канистра
\ в случайной полосе над экраном
: spawn
    0 j !
    begin
        j @ kind @ 0 = if
            3 rnd 1 + j @ kind !
            3 rnd j @ olane !
            -1 j @ row !
            max-obj j !      \ нашли свободный - выходим из цикла
        then
        next-j
    until ;

\ сдвинуть объекты на строку вниз, ушедшие за экран убрать
: move-objects
    0 j !
    begin
        j @ kind @ if
            j @ row @ 1 + j @ row !
            j @ row @ 21 > if 0 j @ kind ! then
        then
        next-j
    until ;

\ машина [V], камень (@), канистра [F]
: draw-object ( n -- )
    dup olane @ lane-x 1 - swap
    dup row @ swap
    kind @
    dup 1 = if drop s" [V]" else
    2 = if s" (@)" else s" [F]" then then
    text ;

: objects
    0 j !
    begin
        j @ kind @ if j @ draw-object then
        next-j
    until ;

\ объект n в полосе игрока и на уровне его машины (строки car-y..car-y+2)?
: hit? ( n -- f )
    dup olane @ lane @ =
    swap row @
    dup car-y 1 - >
    swap car-y 3 + <
    and and ;

\ канистра: +20 бензина, но не больше 99
: refuel
    fuel @ 20 + fuel !
    fuel @ 99 > if 99 fuel ! then ;

\ наехали на объект: канистра заправляет, машина или камень
\ отнимают жизнь; объект пропадает
: collide
    0 j !
    begin
        j @ kind @ if
            j @ hit? if
                j @ kind @ 3 = if refuel else lives @ 1 - lives ! then
                0 j @ kind !
            then
        then
        next-j
    until ;

\ сколько кадров длится один шаг мира: в начале 4,
\ каждые 150 шагов на 1 меньше, но не меньше 1
: delay ( -- n )
    4 dist @ 150 / -
    dup 1 < if drop 1 then ;

\ скорость для панели: от 1 до 4
: speed ( -- n ) 5 delay - ;

\ один шаг мира: путь растёт, объекты едут вниз,
\ через 4-6 шагов появляется новый объект
: step
    dist @ 1 + dist !
    dist @ 2 mod 0 = fuel @ 0 > and if fuel @ 1 - fuel ! then
    move-objects
    gap @ 1 - gap !
    gap @ 0 > not if
        spawn
        3 rnd 4 + gap !
    then ;

\ справа от трассы - название и пройденный путь,
\ слева - подсказка по клавишам
: panel
    28 1 s" DETOUR" text
    28 3 s" DIST" text
    28 4 dist @ num
    28 6 s" LIVES" text
    28 7 lives @ num
    28 9 s" FUEL" text
    28 10 fuel @ num
    28 12 s" SPEED" text
    28 13 speed num
    0 19 s" <- -> lane" text
    0 20 s" Esc   exit" text ;

\ игра кончается, когда нет жизней или бензина
: check-over
    lives @ 1 < fuel @ 1 < or if 1 ended ! then ;

: game-over-screen
    14 9 s"  GAME OVER " text
    14 10 s" R - restart" text ;

: init
    0 ended !
    0 dist !
    1 lane !
    0 frame !
    3 gap !
    3 lives !
    60 fuel !
    clear-objects ;

\ после конца игры ждём R, иначе - обычный кадр
: tick
    ended @ if
        key dup 'r' = swap 'R' = or if init then
    else
        control
        frame @ 1 + frame !
        frame @ delay mod 0 = if step then
        collide
        check-over
    then
    cls
    road
    objects
    player
    panel
    ended @ if game-over-screen then ;
