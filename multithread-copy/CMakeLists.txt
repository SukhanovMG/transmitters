cmake_minimum_required(VERSION 2.8.8)

project(mt_copy)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")
#set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
#set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

find_package(Pthread REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

include_directories("${CMAKE_BINARY_DIR}/libconfig/lib")

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
set(LIB read_config.c)
add_library(read_config STATIC ${LIB})

add_dependencies(read_config libconfig)

add_executable(${CMAKE_PROJECT_NAME} ${SRC})
target_link_libraries(${CMAKE_PROJECT_NAME} read_config
                                            ${PTHREAD_LIBRARIES}
                                            ${CMAKE_BINARY_DIR}/libconfig/lib/.libs/libconfig.a
                                            )