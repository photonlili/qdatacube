
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Qt5Test 5.2.0 REQUIRED NO_MODULE)


ADD_EXECUTABLE(testplaincube danishnamecube.cpp testplaincube.cpp modeltest.cpp)
TARGET_LINK_LIBRARIES(testplaincube qdatacube Qt5::Test)

ADD_EXECUTABLE(testheaders testheaders.cpp danishnamecube.cpp modeltest.cpp)
TARGET_LINK_LIBRARIES(testheaders qdatacube Qt5::Test )

ADD_TEST(testplaincube testplaincube)
