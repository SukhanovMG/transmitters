# Сборка библиотеки libconfig
file(COPY ${LIBCONFIG_SRC_DIR} DESTINATION ${CMAKE_BINARY_DIR}/libs)
add_custom_target(
    libconfig
    DEPENDS ${LIBCONFIG_DST_DIR}/lib/.libs/libconfig.a
)
add_custom_command(
    OUTPUT ${LIBCONFIG_DST_DIR}/Makefile
    COMMAND cd ${LIBCONFIG_DST_DIR} && ${LIBCONFIG_DST_DIR}/configure -q > /dev/null 2>&1
)
add_custom_command(
    OUTPUT ${LIBCONFIG_DST_DIR}/lib/.libs/libconfig.a
    COMMAND cd ${LIBCONFIG_DST_DIR} && make clean > /dev/null 2>&1 && make > ${CMAKE_BINARY_DIR}/libconfig.log 2>&1
    DEPENDS ${LIBCONFIG_DST_DIR}/Makefile
)

# Сборка jemalloc
add_custom_target(
    libjemalloc
    DEPENDS ${LIBJEMALLOC_DST_DIR}/lib/libjemalloc.a
)
add_custom_command(
    OUTPUT ${LIBJEMALLOC_DST_DIR}/configure
    COMMAND tar -xzf "${CMAKE_CURRENT_SOURCE_DIR}/libs/jemalloc.tar.gz" --directory "${CMAKE_BINARY_DIR}/libs/"
)
add_custom_command(
    OUTPUT ${LIBJEMALLOC_DST_DIR}/Makefile
    COMMAND cd ${LIBJEMALLOC_DST_DIR} && ${LIBJEMALLOC_DST_DIR}/configure -q --with-jemalloc-prefix="je_"  > /dev/null 2>&1
    DEPENDS ${LIBJEMALLOC_DST_DIR}/configure
)
add_custom_command(
    OUTPUT ${LIBJEMALLOC_DST_DIR}/lib/libjemalloc.a
    COMMAND cd ${LIBJEMALLOC_DST_DIR} && make clean > /dev/null 2>&1 && make > ${CMAKE_BINARY_DIR}/jemalloc.log 2>&1
    DEPENDS ${LIBJEMALLOC_DST_DIR}/Makefile
)

# Сборка libhl
add_custom_target(
        libhl
        DEPENDS ${LIBHL_DST_DIR}/build/libhl.a
)
add_custom_command(
        OUTPUT ${LIBHL_DST_DIR}/build/configure
        COMMAND tar -xzf "${CMAKE_CURRENT_SOURCE_DIR}/libs/libhl.tar.gz" --directory "${CMAKE_BINARY_DIR}/libs/"
)
add_custom_command(
        OUTPUT ${LIBHL_DST_DIR}/build/Makefile
        COMMAND cd ${LIBHL_DST_DIR}/build && ${LIBHL_DST_DIR}/build/configure -q > ${CMAKE_BINARY_DIR}/libhl.log 2>&1
        DEPENDS ${LIBHL_DST_DIR}/build/configure
)
add_custom_command(
        OUTPUT ${LIBHL_DST_DIR}/build/libhl.a
        COMMAND cd ${LIBHL_DST_DIR}/build && make clean > ${CMAKE_BINARY_DIR}/libhl.log 2>&1 && make > ${CMAKE_BINARY_DIR}/libhl.log 2>&1
        DEPENDS ${LIBHL_DST_DIR}/build/Makefile
)

add_library(COMMON_MODULES OBJECT
    tm_alloc.c
    tm_configuration.c
    tm_queue.c
    tm_queue_simple.c
    tm_queue_rbuf.c
    tm_refcount.c
    tm_block.c
    tm_logging.c
    tm_time.c
    tm_mempool.c
)

add_dependencies(COMMON_MODULES libconfig libjemalloc libhl)