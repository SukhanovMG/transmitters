
for num_of_threads in {1..9}
do
	for config in .config*
	do
		./test.py ./tm ${config} ${num_of_threads} 100000
	done
done
