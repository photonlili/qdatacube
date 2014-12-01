#
# Copyright Ange Optimization 2014
#

project(qdatacube)

cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)

include(CMakePackageConfigHelpers)

include(GenerateExportHeader)
add_compiler_export_flags()

enable_testing()

set(QDATACUBE_VERSION "3.1")
set(QDATACUBE_SO_VERSION 4)

find_package(Qt5Core 5.2.0 REQUIRED NO_MODULE)
find_package(Qt5Widgets 5.2.0 REQUIRED NO_MODULE)

set(CMAKE_AUTOMOC on)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)

if(NOT WIN32)
    add_compile_options(-ggdb3)
    add_compile_options(-Wall)
    add_compile_options(-Wextra)
    add_compile_options(-Werror -Wno-error=deprecated-declarations)
endif(NOT WIN32)

add_subdirectory(qdatacube)
