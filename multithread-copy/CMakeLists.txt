cmake_minimum_required(VERSION 2.8.8)

project(mt)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")


# Build type.
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release." FORCE)
endif()

find_package(Pthread REQUIRED)
#find_package(JeMalloc REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=gnu99")

include_directories("${CMAKE_BINARY_DIR}/libconfig/lib")
include_directories("${CMAKE_BINARY_DIR}/jemalloc/include")
#include_directories("${JEMALLOC_INCLUDE_DIR}")

message(STATUS "~~~~~~~~~~~~~~~~~~ BUILD TYPE '${CMAKE_BUILD_TYPE}' ~~~~~~~~~~~~~~~~~~")


# Сборка библиотеки libconfig
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/libconfig DESTINATION ${CMAKE_BINARY_DIR})
add_custom_target(
    libconfig
    DEPENDS ${CMAKE_BINARY_DIR}/libconfig/lib/.libs/libconfig.a
)
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/libconfig/Makefile
    COMMAND cd ${CMAKE_BINARY_DIR}/libconfig && ${CMAKE_BINARY_DIR}/libconfig/configure -q
)
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/libconfig/lib/.libs/libconfig.a
    COMMAND cd ${CMAKE_BINARY_DIR}/libconfig && make clean 2>1 > /dev/null && make 2>1 > ${CMAKE_BINARY_DIR}/libconfig.log
    DEPENDS ${CMAKE_BINARY_DIR}/libconfig/Makefile
)

# Сборка jemalloc
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/jemalloc DESTINATION ${CMAKE_BINARY_DIR})
add_custom_target(
    libjemalloc
    DEPENDS ${CMAKE_BINARY_DIR}/jemalloc/lib/libjemalloc.a
)
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/jemalloc/Makefile
    COMMAND cd ${CMAKE_BINARY_DIR}/jemalloc && ${CMAKE_BINARY_DIR}/jemalloc/configure -q --with-jemalloc-prefix="je_"
)
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/jemalloc/lib/libjemalloc.a
    COMMAND cd ${CMAKE_BINARY_DIR}/jemalloc && make clean 2>1 > /dev/null && make 2>1 > ${CMAKE_BINARY_DIR}/jemalloc.log
    DEPENDS ${CMAKE_BINARY_DIR}/jemalloc/Makefile
)


set(FOR_CONFIN_COPY_BLOCK_ON_TRANSFER "true")
set(FOR_CONFIG_USE_MEMPOOL "true")
configure_file("config.in" "${EXECUTABLE_OUTPUT_PATH}/.config_copy_mempool" @ONLY)
set(FOR_CONFIG_USE_MEMPOOL "false")
configure_file("config.in" "${EXECUTABLE_OUTPUT_PATH}/.config_copy_no_mempool" @ONLY)
set(FOR_CONFIN_COPY_BLOCK_ON_TRANSFER "false")
set(FOR_CONFIG_USE_MEMPOOL "true")
configure_file("config.in" "${EXECUTABLE_OUTPUT_PATH}/.config_no_copy_mempool" @ONLY)
set(FOR_CONFIG_USE_MEMPOOL "false")
configure_file("config.in" "${EXECUTABLE_OUTPUT_PATH}/.config_no_copy_no_mempool" @ONLY)

configure_file("test.py" "${EXECUTABLE_OUTPUT_PATH}/test.py" COPYONLY)

set(SRC main.c)
set(LIB tm_alloc.c tm_compat.c tm_read_config.c tm_queue.c tm_refcount.c tm_block.c tm_logging.c tm_time.c tm_mempool.c)

add_library(libs STATIC ${LIB})
add_dependencies(libs libconfig libjemalloc)

add_executable("${CMAKE_PROJECT_NAME}" ${SRC})
target_link_libraries("${CMAKE_PROJECT_NAME}" libs
                                            rt # Исправить!!!!
                                            ${CMAKE_BINARY_DIR}/jemalloc/lib/libjemalloc.a
                                            ${PTHREAD_LIBRARIES}
                                            ${CMAKE_BINARY_DIR}/libconfig/lib/.libs/libconfig.a
                                            )
