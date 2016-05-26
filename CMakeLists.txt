cmake_minimum_required(VERSION 2.8.8)

project(tm)

# Путь к модулям cmake
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")

set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/build")
# Путь для исполняемых файлов
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo." FORCE)
endif()

# Поиск необходимых пакетов
find_package(Pthread REQUIRED)
find_package(LibEV REQUIRED)

# Флаги
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fno-strict-aliasing -std=gnu99")

message(STATUS "~~~~~~~~~~~~~~~~~~ BUILD TYPE '${CMAKE_BUILD_TYPE}' ~~~~~~~~~~~~~~~~~~")

set(LIBCONFIG_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/common/libs/libconfig")
set(LIBCONFIG_DST_DIR "${CMAKE_BINARY_DIR}/libs/libconfig")

set(LIBJEMALLOC_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/common/libs")
set(LIBJEMALLOC_DST_DIR "${CMAKE_BINARY_DIR}/libs/jemalloc")

set(LIBGPERFTOOLS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/common/libs")
set(LIBGPERFTOOLS_DST_DIR "${CMAKE_BINARY_DIR}/libs/gperftools")

include_directories("${LIBCONFIG_DST_DIR}/lib")
include_directories("${LIBJEMALLOC_DST_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/common")

add_subdirectory(common)

set(CFG_MEMPOOL "false")
set(CFG_QUEUE_TYPE "1")
set(CFG_THREAD_MODULE_TYPE "1")
configure_file("config.in" "${EXECUTABLE_OUTPUT_PATH}/.cfg_no_pool_qpool_thread_libevq" @ONLY)
set(CFG_MEMPOOL "false")
set(CFG_QUEUE_TYPE "2")
set(CFG_THREAD_MODULE_TYPE "1")
configure_file("config.in" "${EXECUTABLE_OUTPUT_PATH}/.cfg_no_pool_qrbuf_thread_libevq" @ONLY)
set(CFG_MEMPOOL "true")
set(CFG_QUEUE_TYPE "1")
set(CFG_THREAD_MODULE_TYPE "1")
configure_file("config.in" "${EXECUTABLE_OUTPUT_PATH}/.cfg_pool_qpool_thread_libevq" @ONLY)
set(CFG_MEMPOOL "true")
set(CFG_QUEUE_TYPE "2")
set(CFG_THREAD_MODULE_TYPE "1")
configure_file("config.in" "${EXECUTABLE_OUTPUT_PATH}/.cfg_pool_qrbuf_thread_libevq" @ONLY)

set(CFG_MEMPOOL "false")
set(CFG_THREAD_MODULE_TYPE "2")
configure_file("config.in" "${EXECUTABLE_OUTPUT_PATH}/.cfg_no_pool_thread_libevpipe" @ONLY)
set(CFG_MEMPOOL "true")
set(CFG_THREAD_MODULE_TYPE "2")
configure_file("config.in" "${EXECUTABLE_OUTPUT_PATH}/.cfg_pool_thread_libevpipe" @ONLY)

configure_file("test.py" "${EXECUTABLE_OUTPUT_PATH}/test.py" COPYONLY)
configure_file("testall.sh" "${EXECUTABLE_OUTPUT_PATH}/testall.sh" COPYONLY)
configure_file("testall_new.sh" "${EXECUTABLE_OUTPUT_PATH}/testall_new.sh" COPYONLY)
configure_file("test_hardware.py" "${EXECUTABLE_OUTPUT_PATH}/test_hardware.py" COPYONLY)

set(SRC main.c tm_thread.c tm_thread_simple.c tm_thread_events.c tm_thread_events_queue.c)

add_executable("${CMAKE_PROJECT_NAME}" ${SRC} $<TARGET_OBJECTS:COMMON_MODULES>)
add_executable("${CMAKE_PROJECT_NAME}_jemalloc" ${SRC} $<TARGET_OBJECTS:COMMON_MODULES>)
add_executable("${CMAKE_PROJECT_NAME}_tcmalloc" ${SRC} $<TARGET_OBJECTS:COMMON_MODULES>)

target_link_libraries("${CMAKE_PROJECT_NAME}"
		rt
		${PTHREAD_LIBRARIES}
		${LIBCONFIG_DST_DIR}/lib/.libs/libconfig.a
		${LIBEV_LIBRARIES}
		)

target_link_libraries("${CMAKE_PROJECT_NAME}_tcmalloc"
		rt
		${PTHREAD_LIBRARIES}
		${LIBCONFIG_DST_DIR}/lib/.libs/libconfig.a
		${LIBEV_LIBRARIES}
		${LIBGPERFTOOLS_DST_DIR}/.libs/libtcmalloc_minimal.so
		)

target_link_libraries("${CMAKE_PROJECT_NAME}_jemalloc"
		rt
		${LIBJEMALLOC_DST_DIR}/lib/libjemalloc.a
		${PTHREAD_LIBRARIES}
		${LIBCONFIG_DST_DIR}/lib/.libs/libconfig.a
		${LIBEV_LIBRARIES}
		)

