project(unity-scope-hollywood C)
cmake_minimum_required(VERSION 2.8.9)

find_package(PkgConfig REQUIRED)

pkg_check_modules(UNITY REQUIRED "unity >= 7.0.9" grilo-0.2 REQUIRED)
include_directories(${UNITY_INCLUDE_DIRS})

add_definitions(-Wall)

set(DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SOURCE_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/src")

include(GNUInstallDirs)

set(GETTEXT_PACKAGE unity-scope-hollywood)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h")
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory("src")
