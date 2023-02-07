cmake_minimum_required(VERSION 3.1...3.25)
project(cgcp_converter_test)

include(../cmake/utils.cmake)
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" is_top_level)

add_subdirectory(converter)
