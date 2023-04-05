cmake_minimum_required(VERSION 3.1...3.25)
project(cgcp_test LANGUAGES CXX)

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}"
               is_top_level)

if(is_top_level)
  enable_testing()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS On)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF)

set(BUILD_SHARED_LIBS OFF)

FetchContent_MakeAvailable(googletest)

# ----------------------------------------------------------------
# tests dependencies
# ----------------------------------------------------------------

if(is_top_level)
  find_package(SBREP REQUIRED)
endif()

# ----------------------------------------------------------------
# tests sources
# ----------------------------------------------------------------

set(sources IO_OBJ_tests.cpp SBREP_builder_tests.cpp)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

# ----------------------------------------------------------------
# tests target
# ----------------------------------------------------------------

add_executable(SBREP_tests)
target_sources(SBREP_tests PRIVATE ${sources})
target_link_libraries(SBREP_tests PRIVATE SBREP::SBREP gtest_main)

include(GoogleTest)
gtest_discover_tests(SBREP_tests)

add_custom_target(check
        COMMAND env CTEST_OUTPUT_ON_FAILURE=1 GTEST_COLOR=1 ${CMAKE_CTEST_COMMAND}
        DEPENDS SBREP_tests)
