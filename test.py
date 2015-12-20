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

def get_precision(num):
	result = num * 0.005
	if result < 1:
		result = 1
	return result

stop_test = False
maximum_clients = 0
a = start_clients_count
b = start_clients_count

while True:
	if not run_test(b):
		break
	a = b
	b *= 2

if a == b:
	if a == 1:
		stop_test = True
	else:
		a = 1

while not stop_test:
	middle = (a + b) / 2
	if run_test(middle):
		a = middle
	else:
		b = middle

	stop_test = (b - a <= get_precision(a))
	maximum_clients = a

print "Maximum clients = " + str(maximum_clients)