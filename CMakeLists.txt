cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(Cece C)

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CECE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CECE_OUTPUT_DIRECTORY}>)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $<1:${CECE_OUTPUT_DIRECTORY}>)

add_library(cece_lib STATIC source/arguments.c source/cece.c source/lex.c source/log.c source/memory.c source/pre.c source/tree.c)

if(MSVC)
	target_compile_options(cece_lib PUBLIC /W4 /utf-8)
else()
	target_compile_options(cece_lib PUBLIC -Wall -Wextra -pedantic)
endif()

if(WIN32)
	target_compile_definitions(cece_lib PUBLIC UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS)
endif()

target_include_directories(cece_lib PUBLIC include)

add_executable(cece main.c)
target_link_libraries(cece PRIVATE cece_lib)

enable_testing()
add_executable(cece_tests tests/tests.c)
target_link_libraries(cece_tests PRIVATE cece_lib)
add_test(NAME cece_tests COMMAND cece_tests WORKING_DIRECTORY ${CECE_OUTPUT_DIRECTORY})
