set(TEST_TARGET ${PROJECT_NAME}_test)

set(SOURCE_FILES
    "../src/ScreenCapturer.cpp"
    "../src/logger.cpp"
)

add_executable(${TEST_TARGET} 
    "test.cpp"
    "InfoCollector_test.cpp"
    "ScreenCapturer_test.cpp"
    ${SOURCE_FILES} "LaneDetector_test.cpp")

add_executable(ObjectDetectorTest "ObjectDetector_test.cpp")

find_package(GTest CONFIG REQUIRED)
target_link_libraries(${TEST_TARGET} PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

find_package(OpenCV CONFIG REQUIRED)
target_link_libraries(${TEST_TARGET}  PRIVATE opencv_ml opencv_dnn opencv_core opencv_flann)

target_include_directories(${TEST_TARGET} PRIVATE "${CMAKE_SOURCE_DIR}/src")

find_package(Darknet CONFIG REQUIRED)
target_link_libraries(ObjectDetectorTest PRIVATE Darknet::dark)

include(GoogleTest)
gtest_discover_tests(${TEST_TARGET})


