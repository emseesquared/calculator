# Тестовое задание &laquo;Калькулятор&raquo;

Программа вычисляет входящее арифметическое выражение. Понимает основные числовые операции (+-*/), скобочную структуру и числа с плавающей точкой.
Результат вычисления округляется до второго знака после запятой. Округление происходит по обычным правилам (0.004 -> 0, 0.005 -> 0.01).

# Сборка и запуск

Сборка программы и запуск тестов:
```
make
```

Сборка программы:
```
make calc
```

Использование:
```
$ ./calc
2*2
4
1 + 2.5(3 + 5) - 3*4
9
```

# Тестирование

Тесты запускаются командой
```
make test
```
# Версии

Сборка тестировалась с GCC 6.2, GCC 4.8 и GNU Make 3.8
