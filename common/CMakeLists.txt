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

# Сборка tcmalloc
add_custom_target(
        libgperftools
        DEPENDS ${LIBGPERFTOOLS_DST_DIR}/.libs/libprofiler.so
)
add_custom_command(
        OUTPUT ${LIBGPERFTOOLS_DST_DIR}/configure
        COMMAND tar -xzf "${LIBGPERFTOOLS_SRC_DIR}/gperftools.tar.gz" --directory "${CMAKE_BINARY_DIR}/libs/" && cd "${CMAKE_BINARY_DIR}/libs/gperftools" && ./autogen.sh
)
add_custom_command(
        OUTPUT ${LIBGPERFTOOLS_DST_DIR}/Makefile
        COMMAND cd ${LIBGPERFTOOLS_DST_DIR} && ${LIBGPERFTOOLS_DST_DIR}/configure -q > ${CMAKE_BINARY_DIR}/gperftools.log 2>&1
        DEPENDS ${LIBGPERFTOOLS_DST_DIR}/configure
)
add_custom_command(
        OUTPUT ${LIBGPERFTOOLS_DST_DIR}/.libs/libprofiler.so
        COMMAND cd ${LIBGPERFTOOLS_DST_DIR} && make clean > /dev/null 2>&1 && make > ${CMAKE_BINARY_DIR}/gperftools.log 2>&1
        DEPENDS ${LIBGPERFTOOLS_DST_DIR}/Makefile
)

add_library(COMMON_MODULES OBJECT
    tm_alloc.c
    tm_configuration.c
    tm_queue.c
    tm_refcount.c
    tm_block.c
    tm_logging.c
    tm_time.c
    tm_mempool.c
)

add_dependencies(COMMON_MODULES libconfig libjemalloc libgperftools)