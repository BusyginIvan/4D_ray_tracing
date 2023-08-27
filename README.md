# 4D ray tracing

## Краткое описание

Сделал рейтрейсинг в четырёхмерном пространстве, используя C++ (библиотеку SFML) и GLSL (фрагментный шейдер).
Идею взять эти инструменты для реализации я позаимствовал у блогера Onigiri. Он написал 3D рейтрейсинг и снял про это
достаточно подробное [видео на YouTube](https://www.youtube.com/watch?v=jKjbeWHujV0).

Самое трудное было – запрогать получение случайно направленного вектора в четырёхмерном пространстве.
Это нужно для матовых поверхностей. Они отражают лучи в случайную сторону. Мой способ основан на генерации случайной
доли объёма гиперсферы и вычислении по нему четвёртой координаты. При этом получается распределение с нужной
плотностью вероятности. Но вычислять приходится обратную функцию (методом Ньютона), так как выразить в элементарных
функциях получается лишь объём через координату.

Пока я не видел аналогов моей программы. Только тут я смог увидеть в двумерном сечении трёхмерного зеркала трёхмерное
сечение четырёхмерного отражения гиперпространства. )

На данный момент можно относительно легко добавлять на сцену гиперсферы, пространства, 4D цилиндры, гиперкубы и tiger'ы,
а также менять цвет и яркость фона, характеристики солнца.
Все объекты обладают такими свойствами, как степень светимости и зеркальности.
То есть объекты могут быть светящимися, матовыми или зеркальными в той или иной степени.
Настраивается это всё непосредственно в коде шейдера (файл shader.frag).
Шейдер можно редактировать, не пересобирая exe'шник.

Размеры окон и пикселей, число переотражений луча и число запускаемых из каждого пикселя лучей непосредственно влияют на FPS.
Все эти параметры, а также скорость перемещения, скороть вращения камеры и прочие константы можно настроить в файле properties.txt.

Положение камеры харатеризуется четырьмя векторами: перед (y), право (x), верх (z) и w. Соответственно, на главном экране
мы видим сечение xyz, на левом дополнительном – xyw, на правом – wyz. То есть на каждом мы смотрим вперёд. Но расширено
поле зрения разными парами векторов.

Перемещение в текущем сечении при помощи клавиш W, S, D, A, Shift и Space, а также в четвёртое измерение клавишами E и Q.
Камера в текущем сечении (относительно плоскостей верх-w и право-w) вращается при помощи мышки.
Колёсиком её можно повернуть относительно горизонтальной плоскости (xy): это можно понимать, как поворот текущего основного сечения в четырёхмерном пространстве. 
Для освобождения курсора используется клавиша Esc.

Постарался минимально говнокодить. Но возможно получилось не очень.
Вся 4D-магия происходит в относительно жирном файле шейдера.
А всё остальное – это лишь обёртка, позволяющая его настраивать и управлять виртуальной камерой.

Кодил я на Windows в среде CLion с использованием компилятора MinGW. Соответственно, сборкой занимался CMake.

## Немного скриншотов

### Несколько шариков

![](/screenshots/screen1.png)

### Два зеркала друг напротив друга

![](/screenshots/screen2.png)

### Отражение того, чего нет в нашем сечении

![](/screenshots/screen3.png)

### С виду обычная комната, но если повернуть камеру в четвёртое измерение...

![](/screenshots/screen4.png)

### Вспомогательные окна с перпендикулярными трёхмерными сечениями

![](/screenshots/screen5.png)

### Фигура tiger выделяется на фоне других особой непонятностью

![](/screenshots/screen6.png)

### Гиперкуб (четырёхмерный куб)

![](/screenshots/screen7.png)