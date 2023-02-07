cmake_minimum_required(VERSION 3.1...3.25)
project(converter_test)

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" is_top_level)

if(is_top_level)
  find_package(cdcplib REQUIRED)
endif()

set(sources
  converter.cpp)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

add_executable(converter_test)
target_sources(converter_test PRIVATE ${sources} )

target_link_libraries(converter_test PUBLIC
  cgcplib::cgcplib)

find_package(CGAL REQUIRED OPTIONAL_COMPONENTS Qt5)
if(CGAL_Qt5_FOUND)
  target_link_libraries(converter_test PUBLIC
      CGAL::CGAL_Basic_viewer)
else()
  message("CGAL Qt5 is not found.")
endif()

# if(NOT is_top_level)
#     win_copy_deps_to_target_dir(converter_test cgcplib::cgcplib)
# endif()
