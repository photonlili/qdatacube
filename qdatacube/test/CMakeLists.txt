find_package(Qt5Test 5.2.0 REQUIRED NO_MODULE)

add_executable(testplaincube danishnamecube.cpp testplaincube.cpp modeltest.cpp)
target_link_libraries(testplaincube qdatacube Qt5::Test)
add_test(testplaincube testplaincube)

# An interactive test application
add_executable(testheaders testheaders.cpp danishnamecube.cpp modeltest.cpp)
target_link_libraries(testheaders qdatacube Qt5::Test)
