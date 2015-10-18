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
    COMMAND cd ${LIBCONFIG_DST_DIR} && ${LIBCONFIG_DST_DIR}/configure -q
)
add_custom_command(
    OUTPUT ${LIBCONFIG_DST_DIR}/lib/.libs/libconfig.a
    COMMAND cd ${LIBCONFIG_DST_DIR} && make clean 2>1 > /dev/null && make 2>1 > ${CMAKE_BINARY_DIR}/libconfig.log
    DEPENDS ${LIBCONFIG_DST_DIR}/Makefile
)

# Сборка jemalloc
set(LIBJEMALLOC_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/jemalloc")
set(LIBJEMALLOC_DST_DIR "${CMAKE_BINARY_DIR}/libs/jemalloc")

file(COPY ${LIBJEMALLOC_SRC_DIR} DESTINATION ${CMAKE_BINARY_DIR}/libs)
add_custom_target(
    libjemalloc
    DEPENDS ${LIBJEMALLOC_DST_DIR}/lib/libjemalloc.a
)
add_custom_command(
    OUTPUT ${LIBJEMALLOC_DST_DIR}/Makefile
    COMMAND cd ${LIBJEMALLOC_DST_DIR} && ${LIBJEMALLOC_DST_DIR}/configure -q --with-jemalloc-prefix="je_"
)
add_custom_command(
    OUTPUT ${LIBJEMALLOC_DST_DIR}/lib/libjemalloc.a
    COMMAND cd ${LIBJEMALLOC_DST_DIR} && make clean 2>1 > /dev/null && make 2>1 > ${CMAKE_BINARY_DIR}/jemalloc.log
    DEPENDS ${LIBJEMALLOC_DST_DIR}/Makefile
)

# Папки включений
include_directories("${LIBCONFIG_DST_DIR}/lib")
include_directories("${LIBJEMALLOC_DST_DIR}/include")


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
set(MODULES tm_alloc.c tm_compat.c tm_read_config.c tm_queue.c tm_refcount.c tm_block.c tm_logging.c tm_time.c)# tm_mempool.c)

set(MODULES1 ${MODULES} tm_mempool1.c)
set(MODULES2 ${MODULES} tm_mempool2.c)
set(MODULES3 ${MODULES} tm_mempool3.c)

#add_library(libs STATIC ${MODULES})
add_library(libs1 STATIC ${MODULES1})
add_library(libs2 STATIC ${MODULES2})
add_library(libs3 STATIC ${MODULES3})
#add_dependencies(libs libconfig libjemalloc)
add_dependencies(libs1 libconfig libjemalloc)
add_dependencies(libs2 libconfig libjemalloc)
add_dependencies(libs3 libconfig libjemalloc)

add_executable("${CMAKE_PROJECT_NAME}_1" ${SRC})
target_link_libraries("${CMAKE_PROJECT_NAME}_1" libs1
                                            rt # Исправить!!!!
                                            ${LIBJEMALLOC_DST_DIR}/lib/libjemalloc.a
                                            ${PTHREAD_LIBRARIES}
                                            ${LIBCONFIG_DST_DIR}/lib/.libs/libconfig.a
                                            )
                                            
add_executable("${CMAKE_PROJECT_NAME}_2" ${SRC})
target_link_libraries("${CMAKE_PROJECT_NAME}_2" libs2
                                            rt # Исправить!!!!
                                            ${LIBJEMALLOC_DST_DIR}/lib/libjemalloc.a
                                            ${PTHREAD_LIBRARIES}
                                            ${LIBCONFIG_DST_DIR}/lib/.libs/libconfig.a
                                            )
add_executable("${CMAKE_PROJECT_NAME}_3" ${SRC})
target_link_libraries("${CMAKE_PROJECT_NAME}_3" libs3
                                            rt # Исправить!!!!
                                            ${LIBJEMALLOC_DST_DIR}/lib/libjemalloc.a
                                            ${PTHREAD_LIBRARIES}
                                            ${LIBCONFIG_DST_DIR}/lib/.libs/libconfig.a
                                            )
