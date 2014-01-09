#
# Copyright Ange Optimization 2010
#
PROJECT(qdatacube)
cmake_minimum_required(VERSION 2.8)
include(CMakePackageConfigHelpers)
include(GenerateExportHeader)
add_compiler_export_flags()

ENABLE_TESTING()

SET(QDATACUBE_VERSION "3.0")
SET(QDATACUBE_SO_VERSION 3)

FIND_PACKAGE(Qt5Core 5.2.0 REQUIRED NO_MODULE)
FIND_PACKAGE(Qt5Widgets 5.2.0 REQUIRED NO_MODULE)

SET(CMAKE_AUTOMOC on)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)


IF(NOT WIN32)
   add_compile_options(-ggdb3 -Wall -Werror -Wno-error=deprecated-declarations -Wno-error=unused-local-typedefs -Wno-strict-overflow -Wno-trigraphs -Wno-deprecated -Wclobbered -Wempty-body -Wignored-qualifiers -Wmissing-field-initializers -Wsign-compare -Wtype-limits -Winit-self -Wunused-parameter)
ELSE(NOT WIN32)
  add_compile_options(-Wno-trigraphs -Wno-deprecated)
ENDIF(NOT WIN32)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_subdirectory(qdatacube)
