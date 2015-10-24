cmake_minimum_required(VERSION 2.8.8)

project(mt)

# Путь к модулям cmake
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")
# Путь для исполняемых файлов
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")


# Build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: None Debug Release." FORCE)
endif()

# Поиск необходимых пакетов
find_package(Pthread REQUIRED)

# Флаги
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=gnu99")


message(STATUS "~~~~~~~~~~~~~~~~~~ BUILD TYPE '${CMAKE_BUILD_TYPE}' ~~~~~~~~~~~~~~~~~~")

# Сборка библиотеки libconfig
set(LIBCONFIG_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/libconfig")
set(LIBCONFIG_DST_DIR "${CMAKE_BINARY_DIR}/libs/libconfig")

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
set(LIBJEMALLOC_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/jemalloc")
set(LIBJEMALLOC_DST_DIR "${CMAKE_BINARY_DIR}/libs/jemalloc")

#file(COPY ${LIBJEMALLOC_SRC_DIR} DESTINATION ${CMAKE_BINARY_DIR}/libs)
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

# Папки включений
include_directories("${LIBCONFIG_DST_DIR}/lib")
include_directories("${LIBJEMALLOC_DST_DIR}/include")

set(FOR_CONFIG_COPY_BLOCK_ON_TRANSFER "false")
set(FOR_CONFIG_USE_MEMPOOL "false")
set(FOR_CONFIG_USE_JEMALLOC "false")
configure_file("config.in" "${EXECUTABLE_OUTPUT_PATH}/.config_no_copy_no_mempool_no_jemalloc" @ONLY)
set(FOR_CONFIG_COPY_BLOCK_ON_TRANSFER "false")
set(FOR_CONFIG_USE_MEMPOOL "false")
set(FOR_CONFIG_USE_JEMALLOC "true")
configure_file("config.in" "${EXECUTABLE_OUTPUT_PATH}/.config_no_copy_no_mempool_jemalloc" @ONLY)
set(FOR_CONFIG_COPY_BLOCK_ON_TRANSFER "false")
set(FOR_CONFIG_USE_MEMPOOL "true")
set(FOR_CONFIG_USE_JEMALLOC "false")
configure_file("config.in" "${EXECUTABLE_OUTPUT_PATH}/.config_no_copy_mempool_no_jemalloc" @ONLY)
set(FOR_CONFIG_COPY_BLOCK_ON_TRANSFER "false")
set(FOR_CONFIG_USE_MEMPOOL "true")
set(FOR_CONFIG_USE_JEMALLOC "true")
configure_file("config.in" "${EXECUTABLE_OUTPUT_PATH}/.config_no_copy_mempool_jemalloc" @ONLY)
set(FOR_CONFIG_COPY_BLOCK_ON_TRANSFER "true")
set(FOR_CONFIG_USE_MEMPOOL "false")
set(FOR_CONFIG_USE_JEMALLOC "false")
configure_file("config.in" "${EXECUTABLE_OUTPUT_PATH}/.config_copy_no_mempool_no_jemalloc" @ONLY)
set(FOR_CONFIG_COPY_BLOCK_ON_TRANSFER "true")
set(FOR_CONFIG_USE_MEMPOOL "false")
set(FOR_CONFIG_USE_JEMALLOC "true")
configure_file("config.in" "${EXECUTABLE_OUTPUT_PATH}/.config_copy_no_mempool_jemalloc" @ONLY)
set(FOR_CONFIG_COPY_BLOCK_ON_TRANSFER "true")
set(FOR_CONFIG_USE_MEMPOOL "true")
set(FOR_CONFIG_USE_JEMALLOC "false")
configure_file("config.in" "${EXECUTABLE_OUTPUT_PATH}/.config_copy_mempool_no_jemalloc" @ONLY)
set(FOR_CONFIG_COPY_BLOCK_ON_TRANSFER "true")
set(FOR_CONFIG_USE_MEMPOOL "true")
set(FOR_CONFIG_USE_JEMALLOC "true")
configure_file("config.in" "${EXECUTABLE_OUTPUT_PATH}/.config_copy_mempool_jemalloc" @ONLY)

configure_file("test.py" "${EXECUTABLE_OUTPUT_PATH}/test.py" COPYONLY)
configure_file("testall.sh" "${EXECUTABLE_OUTPUT_PATH}/testall.sh" COPYONLY)

set(SRC main.c)
set(MODULES tm_alloc.c tm_compat.c tm_configuration.c tm_queue.c tm_refcount.c tm_block.c tm_logging.c tm_time.c tm_mempool.c)

add_library(libs STATIC ${MODULES})
add_dependencies(libs libconfig libjemalloc)

add_executable("${CMAKE_PROJECT_NAME}" ${SRC})
target_link_libraries("${CMAKE_PROJECT_NAME}" libs
                                            rt # Исправить!!!!
                                            ${LIBJEMALLOC_DST_DIR}/lib/libjemalloc.a
                                            ${PTHREAD_LIBRARIES}
                                            ${LIBCONFIG_DST_DIR}/lib/.libs/libconfig.a
                                            )
