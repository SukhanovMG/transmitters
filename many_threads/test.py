#! /usr/bin/env python
# -*- coding: utf-8 -*-
import subprocess
import sys

test_exec_name = ""
test_config_file_name = ""

if len(sys.argv) < 3:
	print "Error: Specify executable and configuration file"
	sys.exit(1);

test_exec_name = sys.argv[1]
test_config_file_name = sys.argv[2]
fixed_number_of_threads = 0
start_clients_count = 1

if len(sys.argv) > 3:
	fixed_number_of_threads = int(sys.argv[3])
if len(sys.argv) > 4:
	start_clients_count = int(sys.argv[4])

# Функция для запуска очередного теста
# clients_count - количество клиентов в тесте
# Возвращает True, если тест пройден, False, если не пройден
def run_test(clients_count):
	exec_params = [test_exec_name, "-c", test_config_file_name, "--clients=" + str(clients_count)]
	if fixed_number_of_threads > 0:
		exec_params.append("--work-threads=" + str(fixed_number_of_threads))

	exec_string = ""
	for par in exec_params:
		if exec_string == "":
			exec_string = par
		else:
			exec_string = exec_string + " " + par
	print exec_string

	test_res = subprocess.call(exec_params)
	if test_res == 0:
		print "===clients_count = " + str(clients_count) + " passed==="
	else:
		print "===clients_count = " + str(clients_count) + " failed==="
	return (test_res == 0)

#Начинаем с одного клиента
clients_count = start_clients_count
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
# серидина отрезка
middle = (a + b) / 2
while not stop_condition:
	# Определяем с какой стороны сокращаем отрезок
	if run_test(middle):
		a = middle
	else:
		b = middle
	
	middle = (a + b) / 2
	if b - a <= 1:
		stop_condition = True

	# Если очередная серидина будет нецелой, значит конец
	#if ( (a+b) % 2  != 0):
	#	stop_condition = True
# Максимальное количество клиентов - это левая граница
clients_count = a
print "Maximum clents = " + str(clients_count)
