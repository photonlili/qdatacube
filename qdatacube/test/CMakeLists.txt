
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Qt5Test 5.2.0 REQUIRED NO_MODULE)


add_definitions("-DDATADIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data\"")

ADD_EXECUTABLE(testplaincube danishnamecube.cpp testplaincube.cpp modeltest.cpp)
TARGET_LINK_LIBRARIES(testplaincube qdatacube Qt5::Test)
target_compile_options(testplaincube PRIVATE -include QDebug)

ADD_EXECUTABLE(testheaders testheaders.cpp danishnamecube.cpp modeltest.cpp)
TARGET_LINK_LIBRARIES(testheaders qdatacube )
target_compile_options(testheaders PRIVATE -include QDebug)

ADD_TEST(testplaincube testplaincube)
