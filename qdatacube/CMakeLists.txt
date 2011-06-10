#
# Copyright Ange Optimization 2010
#

SET(QDATACUBE_SRC
  abstract_aggregator.cpp
  cell.cpp
  column_aggregator.cpp
  datacube.cpp
  datacube_selection.cpp
  datacube_view.cpp
)

QT4_AUTOMOC(${QDATACUBE_SRC})

include_directories(. ${CMAKE_CURRENT_BINARY_DIR} ${QT_QTCORE_INCLUDE_DIR} {QT_QTGUI_INCLUDE_DIR})

ADD_LIBRARY(qdatacube SHARED ${QDATACUBE_SRC})

target_link_libraries(qdatacube ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
SET(QDATACUBE_SO_VERSION 0)
set_target_properties(qdatacube PROPERTIES VERSION "${QDATACUBE_SO_VERSION}.0.0" SOVERSION "${QDATACUBE_SO_VERSION}" DEFINE_SYMBOL BUILDING_QDATACUBE )

SET(CONTAINERS_PUBLIC_HEADERS
  column_aggregator.h
  datacube.h
  datacube_selection.h
  datacube_view.h
  abstract_aggregator.h
  qdatacube_export.h
)

add_subdirectory(test)

GET_TARGET_PROPERTY(LIB_OUT_NAME qdatacube LOCATION)
GET_FILENAME_COMPONENT(LIB_OUT_NAME  ${LIB_OUT_NAME} NAME)

# Create cmake files for install usage
STRING(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_UPPER)
SET(INCLUDEPATH "${CMAKE_INSTALL_PREFIX}/include")
SET(LIBPATH "${CMAKE_INSTALL_PREFIX}/lib")
CONFIGURE_FILE(qdatacube-config.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/qdatacube-config.cmake" @ONLY)
CONFIGURE_FILE(qdatacube-config-version.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/qdatacube-config-version.cmake" @ONLY)
SET(INCLUDEPATH "${CMAKE_SOURCE_DIR}")
SET(LIBPATH "${CMAKE_CURRENT_BINARY_DIR}")
CONFIGURE_FILE(qdatacube-config.cmake.in "${CMAKE_BINARY_DIR}/qdatacube-config.cmake" @ONLY)
CONFIGURE_FILE(qdatacube-config-version.cmake.in "${CMAKE_BINARY_DIR}/qdatacube-config-version.cmake" @ONLY)


#Installation
INSTALL(TARGETS qdatacube RUNTIME DESTINATION "bin" LIBRARY DESTINATION "lib")
INSTALL(FILES ${CONTAINERS_PUBLIC_HEADERS} DESTINATION "include/qdatacube")
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/qdatacube-config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/qdatacube-config-version.cmake
        DESTINATION "lib/cmake/qdatacube")
