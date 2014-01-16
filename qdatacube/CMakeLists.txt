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

SET(QDATACUBE_PUBLIC_HEADERS
  columnaggregator.h
  datacube.h
  datacubeselection.h
  datacubeview.h
  abstractaggregator.h
  abstractformatter.h
  abstractfilter.h
  columnsumformatter.h
  countformatter.h
  filterbyaggregate.h
)

add_library(qdatacube SHARED ${QDATACUBE_SRC})
target_link_libraries(qdatacube Qt5::Core Qt5::Widgets)
target_compile_options(qdatacube PRIVATE -include QDebug)
generate_export_header(qdatacube)

set_property(TARGET qdatacube PROPERTY VERSION "${QDATACUBE_SO_VERSION}.0.0")
set_property(TARGET qdatacube PROPERTY SOVERSION "${QDATACUBE_SO_VERSION}")

install(TARGETS qdatacube EXPORT QDatacubeTargets
    RUNTIME DESTINATION "bin" 
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    INCLUDES DESTINATION "include"
)

install(FILES ${QDATACUBE_PUBLIC_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/qdatacube_export.h DESTINATION "include/qdatacube" )



write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/QDatacubeConfigVersion.cmake VERSION ${QDATACUBE_VERSION} COMPATIBILITY SameMajorVersion )

export(TARGETS qdatacube FILE "${CMAKE_CURRENT_BINARY_DIR}/QDatacubeTargets.cmake")


set(ConfigPackageLocation lib/cmake/QDatacube)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/QDatacubeConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/QDatacubeConfig.cmake"
  @ONLY
)

install(EXPORT QDatacubeTargets 
    FILE QDatacubeTargets.cmake
    DESTINATION ${ConfigPackageLocation})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/QDatacubeConfigVersion.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/QDatacubeConfig.cmake DESTINATION  ${ConfigPackageLocation})



add_subdirectory(test)

