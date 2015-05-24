#! /usr/bin/env python
# -*- coding: utf-8 -*-
import subprocess

# Функция для запуска очередного теста
# clients_count - количество клиентов в тесте
# Возвращает True, если тест пройден, False, если не пройден
def run_test(clients_count):
	test_res = subprocess.call(["./mt_copy", "-c", ".config", "-C", str(clients_count)])
	if test_res == 0:
		print "===clients_count = " + str(clients_count) + " passed==="
	else:
		print "===clients_count = " + str(clients_count) + " failed==="
	return (test_res == 0)

#Начинаем с одного клиента
clients_count = 1
"""
	Удваивая количество клиентов после успешного теста,
	ищем количество клиентов, на котором тест проваливается.
"""
while True:
	if not run_test(clients_count):
		break
	clients_count *= 2

# Если это количество 1 или 2, то оно и является максимальным
stop_condition = False
if clients_count == 1 or clients_count == 2:
	stop_condition = True

# Если больше, то будем искать более точное значение методом биссекции
# a - последнее значение, на котором тест пройден (делим на 2, т.к. каждую успешную итерацию умножаем на 2)
a = clients_count / 2
# b - значение, на котором тест провалился, т.е. последнее значение количества клиентов
b = clients_count
while not stop_condition:
	# середина отрезка
	middle = (a + b) / 2

	# Определяем с какой стороны сокращаем отрезок
	if run_test(middle):
		a = middle
	else:
		b = middle

	# Если очередная серидина будет нецелой, значит конец
	if ( (a+b) % 2  != 0):
		stop_condition = True
# Максимальное количество клиентов - это левая граница
clients_count = a
print "Maximum clents = " + str(clients_count)