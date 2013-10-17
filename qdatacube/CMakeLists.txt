#
# Copyright Ange Optimization 2010
#

SET(QDATACUBE_SRC
  abstractfilter.cpp
  abstractaggregator.cpp
  abstractformatter.cpp
  cell.cpp
  columnaggregator.cpp
  columnsumformatter.cpp
  countformatter.cpp
  datacube.cpp
  datacubeselection.cpp
  datacubeview.cpp
  filterbyaggregate.cpp
)

QT4_AUTOMOC(${QDATACUBE_SRC})

QT4_WRAP_CPP(MANUALLY_MOCED datacube_p.h datacubeselection_p.h datacubeview_p.h)

include_directories(. ${CMAKE_CURRENT_BINARY_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR})

ADD_LIBRARY(qdatacube SHARED ${QDATACUBE_SRC} ${MANUALLY_MOCED})

target_link_libraries(qdatacube ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
SET(QDATACUBE_SO_VERSION 0)
set_target_properties(qdatacube PROPERTIES VERSION "${QDATACUBE_SO_VERSION}.0.0" SOVERSION "${QDATACUBE_SO_VERSION}" DEFINE_SYMBOL BUILDING_QDATACUBE )

SET(CONTAINERS_PUBLIC_HEADERS
  columnaggregator.h
  datacube.h
  datacubeselection.h
  datacubeview.h
  abstractaggregator.h
  abstractformatter.h
  abstractfilter.h
  columnsumformatter.h
  countformatter.h
  qdatacube_export.h
  filterbyaggregate.h
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
