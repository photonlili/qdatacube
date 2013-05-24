
QT4_AUTOMOC(testplaincube.cpp testheaders.cpp modeltest.cpp testdatacube_model.cpp danishnamecube.cpp modeltest.cpp)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions("-DDATADIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data\"")

ADD_EXECUTABLE(testplaincube danishnamecube.cpp testplaincube.cpp modeltest.cpp)
TARGET_LINK_LIBRARIES(testplaincube qdatacube ${QT_QTTEST_LIBRARY})

ADD_EXECUTABLE(testheaders testheaders.cpp danishnamecube.cpp modeltest.cpp)
TARGET_LINK_LIBRARIES(testheaders qdatacube ${QT_QTTEST_LIBRARY} )

ADD_TEST(testplaincube testplaincube)
