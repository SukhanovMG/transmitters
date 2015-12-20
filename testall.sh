#!/usr/bin/env bash

NUMBER_OF_THREADS=$(expr `nproc` - 1)

./test.py ./tm .config_no_copy_no_mempool_no_jemalloc 0 1000
./test.py ./tm .config_no_copy_no_mempool_jemalloc 0 1000
./test.py ./tm .config_no_copy_mempool_no_jemalloc 0 1000
./test.py ./tm .config_no_copy_mempool_jemalloc 0 1000
./test.py ./tm .config_copy_no_mempool_no_jemalloc 0 1000
./test.py ./tm .config_copy_no_mempool_jemalloc 0 1000
./test.py ./tm .config_copy_mempool_no_jemalloc 0 1000
./test.py ./tm .config_copy_mempool_jemalloc 0 1000

./test.py ./tm .config_no_copy_no_mempool_no_jemalloc ${NUMBER_OF_THREADS} 1000
./test.py ./tm .config_no_copy_no_mempool_jemalloc ${NUMBER_OF_THREADS} 1000
./test.py ./tm .config_no_copy_mempool_no_jemalloc ${NUMBER_OF_THREADS} 1000
./test.py ./tm .config_no_copy_mempool_jemalloc ${NUMBER_OF_THREADS} 1000
./test.py ./tm .config_copy_no_mempool_no_jemalloc ${NUMBER_OF_THREADS} 1000
./test.py ./tm .config_copy_no_mempool_jemalloc ${NUMBER_OF_THREADS} 1000
./test.py ./tm .config_copy_mempool_no_jemalloc ${NUMBER_OF_THREADS} 1000
./test.py ./tm .config_copy_mempool_jemalloc ${NUMBER_OF_THREADS} 1000

./test.py ./tm .config_libev_no_return ${NUMBER_OF_THREADS} 100000
./test.py ./tm .config_libev_return ${NUMBER_OF_THREADS} 100000