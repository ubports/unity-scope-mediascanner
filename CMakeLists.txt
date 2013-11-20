project(unity-scope-hollywood C CXX)
cmake_minimum_required(VERSION 2.8.9)

find_package(PkgConfig REQUIRED)

pkg_check_modules(UNITY REQUIRED
  "unity >= 7.0.9"
  mediascanner-2.0
)
include_directories(${UNITY_INCLUDE_DIRS})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -std=c99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -std=c++11")

set(DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SOURCE_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/src")

include(GNUInstallDirs)

set(GETTEXT_PACKAGE unity-scope-mediascanner)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory("src")
add_subdirectory("data")

enable_testing()
add_subdirectory("tests")
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -V --output-on-failure)
