cmake_minimum_required(VERSION 3.1...3.25)
project(converter LANGUAGES CXX)

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" is_top_level)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS On)

if(is_top_level)
  find_package(SBREP REQUIRED)
endif()

set(IS_DEBUG_BUILD CMAKE_BUILD_TYPE STREQUAL "Debug")

# Indication to the code that this is a debug build
if (${IS_DEBUG_BUILD})
    add_compile_definitions(__DEBUG__)
endif ()

set(sources
  converter.cpp)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

add_executable(converter)
target_sources(converter PRIVATE ${sources} )

target_link_libraries(converter PUBLIC
  SBREP::SBREP)

find_package(CGAL REQUIRED OPTIONAL_COMPONENTS Qt5)
if(CGAL_Qt5_FOUND)
  target_link_libraries(converter PUBLIC
      CGAL::CGAL_Basic_viewer)
else()
  message("CGAL Qt5 is not found.")
endif()

# if(NOT is_top_level)
#     win_copy_deps_to_target_dir(converter_test SBREP::SBREP)
# endif()
