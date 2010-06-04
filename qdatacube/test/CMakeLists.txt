
QT4_AUTOMOC(testplaincube.cpp)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions("-DDATADIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data\"")

ADD_EXECUTABLE(testplaincube testplaincube.cpp)
TARGET_LINK_LIBRARIES(testplaincube qdatacube ${QT_QTTEST_LIBRARY})

ADD_TEST(testplaincube testplaincube)
