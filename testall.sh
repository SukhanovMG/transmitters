./test.py ./tm .config_no_copy_no_mempool_no_jemalloc `nproc` 100
./test.py ./tm .config_no_copy_no_mempool_jemalloc `nproc` 100
./test.py ./tm .config_no_copy_mempool_no_jemalloc `nproc` 100
./test.py ./tm .config_no_copy_mempool_jemalloc `nproc` 100
./test.py ./tm .config_copy_no_mempool_no_jemalloc `nproc` 100
./test.py ./tm .config_copy_no_mempool_jemalloc `nproc` 100
./test.py ./tm .config_copy_mempool_no_jemalloc `nproc` 100
./test.py ./tm .config_copy_mempool_jemalloc `nproc` 100

./test.py ./tm .config_no_copy_no_mempool_no_jemalloc 0 100
./test.py ./tm .config_no_copy_no_mempool_jemalloc 0 100
./test.py ./tm .config_no_copy_mempool_no_jemalloc 0 100
./test.py ./tm .config_no_copy_mempool_jemalloc 0 100
./test.py ./tm .config_copy_no_mempool_no_jemalloc 0 100
./test.py ./tm .config_copy_no_mempool_jemalloc 0 100
./test.py ./tm .config_copy_mempool_no_jemalloc 0 100
./test.py ./tm .config_copy_mempool_jemalloc 0 100