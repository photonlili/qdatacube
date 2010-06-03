#
# Copyright Ange Optimization 2010
#

SET(QDATACUBE_SRC
  qdatacube.cpp
  qdatacube_colrow.cpp
  qdatacube_header.cpp
  qdatacube_header_model.cpp
  qdatacube_model.cpp
)

QT4_AUTOMOC(${QDATACUBE_SRC})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY(qdatacube SHARED ${QDATACUBE_SRC})

target_link_libraries(qdatacube ${QT_CORE_LIBRARY} ${QT_GUI_LIBRARY})
SET(QDATACUBE_SO_VERSION 0)
SET(QDATACUBE_VERSION "0.1")
set_target_properties(qdatacube PROPERTIES VERSION "${QDATACUBE_SO_VERSION}.0.0" SOVERSION "${QDATACUBE_SO_VERSION}" DEFINE_SYMBOL BUILDING_QDATACUBE )

SET(CONTAINERS_PUBLIC_HEADERS
  qdatacube.h
  qdatacube_colrow.h
  qdatacube_model.h
  qdatacube_header.h
  qdatacube_header_model.h
  abstract_filter.h
)

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
