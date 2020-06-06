# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.15)

project ("auto-ets2" CXX)

find_package(OpenCV CONFIG REQUIRED)
link_libraries(opencv_core opencv_highgui)
find_package(Tesseract CONFIG REQUIRED)
link_libraries(libtesseract)

# Include sub-projects.
add_subdirectory ("src")

include(CTest)
add_subdirectory("test")
