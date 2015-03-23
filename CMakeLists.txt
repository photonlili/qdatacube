#
# Copyright Ange Optimization 2010-2015
#

cmake_minimum_required(VERSION 3.0)
cmake_policy(VERSION 3.0)

project(qdatacube)
set(QDATACUBE_VERSION "3.1")
set(QDATACUBE_SO_VERSION 4)

add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-Werror)
add_compile_options(-Wno-error=deprecated-declarations)
add_compile_options(-ggdb3)

set(CMAKE_CXX_FLAGS "-Wl,--build-id")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--build-id")

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)

include(CMakePackageConfigHelpers)

include(GenerateExportHeader)
add_compiler_export_flags()

find_package(Qt5Core 5.2.0 REQUIRED CONFIG)
find_package(Qt5Widgets 5.2.0 REQUIRED CONFIG)

enable_testing()

add_subdirectory(qdatacube)
