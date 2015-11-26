cmake_minimum_required(VERSION 2.8.8)

project(mt)

# Путь к модулям cmake
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")

set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/build")
# Путь для исполняемых файлов
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: None Debug Release." FORCE)
endif()

# Поиск необходимых пакетов
find_package(Pthread REQUIRED)

# Флаги
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=gnu99")


message(STATUS "~~~~~~~~~~~~~~~~~~ BUILD TYPE '${CMAKE_BUILD_TYPE}' ~~~~~~~~~~~~~~~~~~")

set(LIBCONFIG_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/common/libs/libconfig")
set(LIBCONFIG_DST_DIR "${CMAKE_BINARY_DIR}/libs/libconfig")

set(LIBJEMALLOC_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/common/libs/jemalloc")
set(LIBJEMALLOC_DST_DIR "${CMAKE_BINARY_DIR}/libs/jemalloc")

include_directories("${LIBCONFIG_DST_DIR}/lib")
include_directories("${LIBJEMALLOC_DST_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/common")

add_subdirectory(common)

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

set(SRC main.c tm_thread.c)

add_executable("${CMAKE_PROJECT_NAME}" ${SRC} $<TARGET_OBJECTS:COMMON_MODULES>)
target_link_libraries("${CMAKE_PROJECT_NAME}"
                                            #libs
                                            rt # Исправить!!!!
                                            ${LIBJEMALLOC_DST_DIR}/lib/libjemalloc.a
                                            ${PTHREAD_LIBRARIES}
                                            ${LIBCONFIG_DST_DIR}/lib/.libs/libconfig.a
                                            )
