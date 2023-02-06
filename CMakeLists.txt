cmake_minimum_required(VERSION 3.1...3.25)
project(cgcplib VERSION 0.1 LANGUAGES CXX)

include(cmake/utils.cmake)
include(GNUInstallDirs)

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" is_top_level)

option(CGCPLIB_BUILD_TESTS "Build cgcplib tests" OFF)
option(CGCPLIB_BUILD_EXAMPLES "Build cgcplib examples" OFF)
option(CGCPLIB_BUILD_DOCS "Build cgcplib documentation" OFF)
option(CGCPLIB_INSTALL "Generate target for installing cgcplib" ${is_top_level})
set_if_undefined(CGCPLIB_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/cgcplib" CACHE STRING
    "Install path for cgcplib package-related CMake files")

if(DEFINED CGCPLIB_SHARED_LIBS)
    set(BUILD_SHARED_LIBS ${CGCPLIB_SHARED_LIBS})
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set_if_undefined(CMAKE_CXX_VISIBILITY_PRESET hidden)
set_if_undefined(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

add_library(cgcplib)
add_library(cgcplib::cgcplib ALIAS cgcplib)


include(GenerateExportHeader)
set(export_file_name "export_shared.h")

if(NOT BUILD_SHARED_LIBS)
    set(export_file_name "export_static.h")
endif()
generate_export_header(cgcplib EXPORT_FILE_NAME include/cgcplib/${export_file_name})

set(headers
    include/cgcplib/SBREPBuilder.hpp
    include/cgcplib/SBREPObject.hpp
)
set(sources
    ${headers}
    src/SBREPBuilder.cpp
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})


include(CMakePackageConfigHelpers)
target_sources(cgcplib PRIVATE ${sources})
target_include_directories(cgcplib
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

set_target_properties(cgcplib PROPERTIES
    PUBLIC_HEADER "${headers}"
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION})

find_package(CGAL REQUIRED Core)
set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE TRUE)
if(CGAL_FOUND)
    target_link_libraries(cgcplib PUBLIC CGAL::CGAL CGAL::CGAL_Core)
endif()

install(DIRECTORY include/cgcplib DESTINATION include)
install(TARGETS cgcplib EXPORT CGCPlibConfig)
install(EXPORT CGCPlibConfig NAMESPACE CGCPlib:: DESTINATION share/CGCPlib/cmake)
include(CMakePackageConfigHelpers)
write_basic_package_version_file("${PROJECT_BINARY_DIR}/CGCPlibConfigVersion.cmake"
    VERSION
        ${PROJECT_VERSION}
    COMPATIBILITY
        AnyNewerVersion
)

if(CGCPLIB_INSTALL AND NOT CMAKE_SKIP_INSTALL_RULES)
    configure_package_config_file(cmake/cgcplib-config.cmake.in cgcplib-config.cmake
        INSTALL_DESTINATION "${CGCPLIB_INSTALL_CMAKEDIR}")

    write_basic_package_version_file(cgcplib-config-version.cmake
        COMPATIBILITY SameMajorVersion)
    install(TARGETS cgcplib EXPORT cgcplib_export
        RUNTIME COMPONENT cgcplib
        LIBRARY COMPONENT cgcplib NAMELINK_COMPONENT cgcplib-dev
        ARCHIVE COMPONENT cgcplib-dev
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
    install(DIRECTORY include/
        TYPE INCLUDE
        COMPONENT cgcplib-dev)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/cgcplib/${export_file_name}"
        COMPONENT cgcplib-dev
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/cgcplib")

    set(targets_file "cgcplib-shared-targets.cmake")

    if(NOT BUILD_SHARED_LIBS)
        set(targets_file "sgcplib-static-targets.cmake")
    endif() 

    install(EXPORT cgcplib_export
        COMPONENT cgcplib-dev
        FILE "${targets_file}"
        DESTINATION "${CGCPLIB_INSTALL_CMAKEDIR}"
        NAMESPACE cgcplib::)

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/cgcplib-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/cgcplib-config-version.cmake"
        COMPONENT cgcplib-dev
        DESTINATION "${CGCPLIB_INSTALL_CMAKEDIR}")

    if(MSVC)
        set(pdb_file "")
        set(pdb_file_destination "")

        if(BUILD_SHARED_LIBS)
            set(pdb_file "$<TARGET_PDB_FILE:cgcplib>")
            set(pdb_file_destination "${CMAKE_INSTALL_BINDIR}")
        else()
            # TARGET_PDB_FILE does not work for pdb file generated for static library build, determining it manually
            set(pdb_file "$<TARGET_FILE_DIR:cgcplib>/$<TARGET_FILE_PREFIX:cgcplib>$<TARGET_FILE_BASE_NAME:cgcplib>.pdb")
            set(pdb_file_destination "${CMAKE_INSTALL_LIBDIR}")
        endif()

        install(FILES "${pdb_file}"
            COMPONENT cgcplib-dev
            CONFIGURATIONS Debug RelWithDebInfo
            DESTINATION "${pdb_file_destination}"
            OPTIONAL)
    endif()
endif()

if(CGCPLIB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()