#! /usr/bin/env python
# -*- coding: utf-8 -*-
import glob
import subprocess
import sys

class Test:
	def __init__(self, exec_name, config_name, work_threads):
		self.__exec_name = exec_name
		self.__config_name = config_name
		self.__work_threads = work_threads
		self.__result = 0

	def __get_test_result_string(self, clients_count, test_result):
		return "Test with " + str(clients_count) + (test_result == 0 and " passed" or " failed")

	def __get_exec_string(self, params_array):
		exec_string = ""
		for par in params_array:
			exec_string += par + " "
		return exec_string

	def __get_precision(self, num):
		result = num * 0.005
		if result < 1:
			result = 1
		return result

	# Функция для запуска очередного теста
	# clients_count - количество клиентов в тесте
	# Возвращает True, если тест пройден, False, если не пройден
	def run_step(self, clients_count):
		exec_params = [self.__exec_name, "-c", self.__config_name, "--work-threads=" + str(self.__work_threads), "--clients=" + str(clients_count)]
		print self.__get_exec_string(exec_params)

		test_res = subprocess.call(exec_params)
		print self.__get_test_result_string(clients_count, test_res)
		return (test_res == 0)

	def run(self):
		start_clients_count = 100000

		stop_test = False
		maximum_clients = 0
		a = start_clients_count
		b = start_clients_count

		while True:
			if not self.run_step(b):
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
			if self.run_step(middle):
				a = middle
			else:
				b = middle

			stop_test = (b - a <= self.__get_precision(a))
			maximum_clients = a

		self.__result = maximum_clients
		#return maximum_clients

	def get_exec_name(self):
		return self.__exec_name
	def get_config_name(self):
		return self.__config_name
	def get_work_threads(self):
		return self.__work_threads
	def get_result(self):
		return self.__result


nproc = int(subprocess.check_output(["nproc"]))
exec_names = ['./tm', './tm_tcmalloc']
#config_names = glob.glob('./cfg*')
config_names = [".cfg_pool_thread_libevpipe"]

test_arr = []
for exec_name in exec_names:
	for config_name in config_names:
		for i in xrange(1, 2):
			test_arr.append(Test(exec_name, config_name, i))

for test in test_arr:
	test.run()

winner = test_arr[0]
for test in test_arr:
	if test.get_result() > winner.get_result():
		winner = test

out = "Best choise is " + winner.get_exec_name() + " with config " + winner.get_config_name() + ' with ' + str(winner.get_work_threads())
out += ' work threads. Result is ' + str(winner.get_result())
print(out)