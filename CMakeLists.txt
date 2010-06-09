#
# Copyright Ange Optimization 2010
#
cmake_minimum_required(VERSION 2.6)

ENABLE_TESTING()

PROJECT(qdatacube)

FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui REQUIRED)
include(${QT_USE_FILE})

SET(QDATACUBE_SO_VERSION 0)
SET(QDATACUBE_VERSION "0.3")

IF(NOT WIN32)
  SET(CMAKE_CXX_FLAGS "-fvisibility=hidden -fdiagnostics-show-option -ggdb3 -Wall -Werror=switch -Wno-strict-overflow -Werror=return-type -Wno-trigraphs -Wno-deprecated -Wclobbered -Wempty-body -Wignored-qualifiers -Wmissing-field-initializers -Wsign-compare -Wtype-limits -Werror=init-self -Wunused-parameter --include QDebug" CACHE STRING "Compile flags for c++" FORCE)
ELSE(NOT WIN32)
  SET(CMAKE_CXX_FLAGS "-Wno-trigraphs -Wno-deprecated" CACHE STRING "Compile flags for c++" FORCE)
ENDIF(NOT WIN32)

SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -Werror=uninitialized -Werror=init-self" CACHE STRING "Flags used by the compiler during Release with Debug Info builds" FORCE)
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_Release} -Wuninitialized" CACHE STRING "Flags used by the compiler during release builds" FORCE)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_subdirectory(qdatacube)
