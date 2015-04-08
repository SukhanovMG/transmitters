cmake_minimum_required(VERSION 2.8.8)

project(mt_copy)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")
#set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
#set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")


# Build type.
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release." FORCE)
endif()

find_package(Pthread REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c99")
if(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -s")
    set(CMAKE_LINK_FLAGS "${CMAKE_LINK_FLAGS} -O3 -s")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g -pg")
    set(CMAKE_LINK_FLAGS "${CMAKE_LINK_FLAGS} -O0 -g -pg")
endif()

include_directories("${CMAKE_BINARY_DIR}/libconfig/lib")

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



set(SRC main.c)
set(LIB tm_alloc.c tm_compat.c tm_read_config.c tm_queue.c)
add_library(libs STATIC ${LIB})

add_dependencies(libs libconfig)

add_executable(${CMAKE_PROJECT_NAME} ${SRC})
target_link_libraries(${CMAKE_PROJECT_NAME} libs
                                            ${PTHREAD_LIBRARIES}
                                            ${CMAKE_BINARY_DIR}/libconfig/lib/.libs/libconfig.a
                                            )