set(TEST_TARGET ${PROJECT_NAME}_test)

set(SOURCE_FILES
    "../src/InfoCollector.cpp"
    "../src/ScreenCapturer.cpp"
)

add_executable(${TEST_TARGET} 
    "test.cpp"
    "InfoCollector_test.cpp"
    "ScreenCapturer_test.cpp"
    ${SOURCE_FILES})

find_package(GTest CONFIG REQUIRED)
target_link_libraries(${TEST_TARGET} PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

find_package(OpenCV CONFIG REQUIRED)
target_link_libraries(${TEST_TARGET}  PRIVATE opencv_ml opencv_dnn opencv_core opencv_flann)

target_include_directories(${TEST_TARGET} PRIVATE "${CMAKE_SOURCE_DIR}/src")

include(GoogleTest)
gtest_discover_tests(${TEST_TARGET})
