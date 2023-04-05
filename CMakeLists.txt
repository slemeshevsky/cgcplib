cmake_minimum_required(VERSION 3.1...3.25)
project(
  SBREP
  VERSION 0.1
  LANGUAGES CXX)

include(cmake/utils.cmake)
include(GNUInstallDirs)

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" is_top_level)

set(BINARY_DIR "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${BINARY_DIR}/lib")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SBREP_BUILD_TESTS "Build SBREP tests" OFF)
option(SBREP_BUILD_EXAMPLES "Build SBREP examples" ON)
option(SBREP_BUILD_DOCS "Build SBREP documentation" OFF)
option(SBREP_INSTALL "Generate target for installing SBREP" ${is_top_level})
set_if_undefined(
  SBREP_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/SBREP" CACHE STRING
  "Install path for SBREP package-related CMake files")

if(DEFINED SBREP_SHARED_LIBS)
  set(BUILD_SHARED_LIBS ${SBREP_SHARED_LIBS})
endif()

set(IS_DEBUG_BUILD CMAKE_BUILD_TYPE STREQUAL "Debug")

# Indication to the code that this is a debug build
if(${IS_DEBUG_BUILD})
  add_compile_definitions(__DEBUG__)
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE
      Release
      CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                                               "MinSizeRel" "RelWithDebInfo")
endif()

set_if_undefined(CMAKE_CXX_VISIBILITY_PRESET hidden)
set_if_undefined(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

add_library(SBREP)
add_library(SBREP::SBREP ALIAS SBREP)

include(GenerateExportHeader)
set(export_file_name "export_shared.h")

if(NOT BUILD_SHARED_LIBS)
  set(export_file_name "export_static.h")
endif()
generate_export_header(SBREP EXPORT_FILE_NAME include/SBREP/${export_file_name})

set(headers
  include/SBREP/config.hpp
  include/SBREP/Core.hpp
  include/SBREP/SBREP_builder.hpp
  include/SBREP/SBREP_object.hpp
  include/SBREP/IO/OBJ.hpp
  include/SBREP/IO/SBrep.hpp)
set(sources ${headers}
  src/SBREP_builder.cpp
  src/SBREP_object.cpp)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

include(CMakePackageConfigHelpers)
target_sources(SBREP PRIVATE ${sources})
target_include_directories(
  SBREP
  PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
         "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
         "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

set_target_properties(
  SBREP
  PROPERTIES PUBLIC_HEADER "${headers}"
             SOVERSION ${PROJECT_VERSION_MAJOR}
             VERSION ${PROJECT_VERSION}
             CMAKE_CXX_STANDARD 17
             CMAKE_CXX_STANDARD_REQUIRED On
             CMAKE_CXX_EXTENSIONS On)

find_package(CGAL REQUIRED COMPONENTS Core Qt5)
set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE TRUE)
if(CGAL_FOUND)
  target_link_libraries(SBREP PUBLIC CGAL::CGAL CGAL::CGAL_Core)
endif()

install(DIRECTORY include/SBREP DESTINATION include)
install(TARGETS SBREP EXPORT SBREPConfig)
install(
  EXPORT SBREPConfig
  NAMESPACE SBREP::
  DESTINATION share/SBREP/cmake)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/SBREPConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion)

if(SBREP_INSTALL AND NOT CMAKE_SKIP_INSTALL_RULES)
  configure_package_config_file(cmake/SBREP-config.cmake.in SBREP-config.cmake
                                INSTALL_DESTINATION "${SBREP_INSTALL_CMAKEDIR}")

  write_basic_package_version_file(SBREP-config-version.cmake
                                   COMPATIBILITY SameMajorVersion)
  install(
    TARGETS SBREP
    EXPORT SBREP_export
    RUNTIME COMPONENT SBREP
    LIBRARY COMPONENT SBREP NAMELINK_COMPONENT SBREP-dev
    ARCHIVE COMPONENT SBREP-dev
    INCLUDES
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
  install(
    DIRECTORY include/
    TYPE INCLUDE
    COMPONENT SBREP-dev)
  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/include/SBREP/${export_file_name}"
    COMPONENT SBREP-dev
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/SBREP")

  set(targets_file "SBREP-shared-targets.cmake")

  if(NOT BUILD_SHARED_LIBS)
    set(targets_file "SBREP-static-targets.cmake")
  endif()

  install(
    EXPORT SBREP_export
    COMPONENT SBREP-dev
    FILE "${targets_file}"
    DESTINATION "${SBREP_INSTALL_CMAKEDIR}"
    NAMESPACE SBREP::)

  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/SBREP-config.cmake"
          "${CMAKE_CURRENT_BINARY_DIR}/SBREP-config-version.cmake"
    COMPONENT SBREP-dev
    DESTINATION "${SBREP_INSTALL_CMAKEDIR}")

  if(MSVC)
    set(pdb_file "")
    set(pdb_file_destination "")

    if(BUILD_SHARED_LIBS)
      set(pdb_file "$<TARGET_PDB_FILE:SBREP>")
      set(pdb_file_destination "${CMAKE_INSTALL_BINDIR}")
    else()
      # TARGET_PDB_FILE does not work for pdb file generated for static library
      # build, determining it manually
      set(pdb_file
          "$<TARGET_FILE_DIR:SBREP>/$<TARGET_FILE_PREFIX:SBREP>$<TARGET_FILE_BASE_NAME:SBREP>.pdb"
      )
      set(pdb_file_destination "${CMAKE_INSTALL_LIBDIR}")
    endif()

    install(
      FILES "${pdb_file}"
      COMPONENT SBREP-dev
      CONFIGURATIONS Debug RelWithDebInfo
      DESTINATION "${pdb_file_destination}"
      OPTIONAL)
  endif()
endif()

if(SBREP_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(SBREP_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if(SBREP_BUILD_DOCS)
  find_package(Doxygen REQUIRED)
  doxygen_add_docs(docs include)
endif()

# first we can indicate the documentation build as an option and set it to OFF by default
# option(BUILD_DOC "Build documentation" OFF)

# # check if Doxygen is installed
# find_package(Doxygen)
# if (DOXYGEN_FOUND)
#     # set input and output files
#     set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
#     set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

#     # request to configure the file
#     configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
#     message("Doxygen build started")

#     # note the option ALL which allows to build the docs together with the application
#     add_custom_target( doc_doxygen ALL
#         COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
#         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#         COMMENT "Generating API documentation with Doxygen"
#         VERBATIM )
# else (DOXYGEN_FOUND)
#   message("Doxygen need to be installed to generate the doxygen documentation")
# endif (DOXYGEN_FOUND)
