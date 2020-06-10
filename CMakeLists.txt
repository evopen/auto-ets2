# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.15)

project ("auto-ets2" CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV CONFIG REQUIRED)
link_libraries(opencv_core opencv_highgui)
find_package(Tesseract CONFIG REQUIRED)
link_libraries(libtesseract)
find_package(CUDA 10.1 REQUIRED)

#include_directories("D:/whatever/TensorRT-7.0.0.11/include")
#include_directories("D:/whatever/TensorRT-7.0.0.11/samples/common")
include_directories("3rdparty/onnxruntime-win-x64-gpu-1.3.0/include")
include_directories(${CUDA_INCLUDE_DIRS})
message(${CUDA_INCLUDE_DIRS})

#link_directories("D:/whatever/TensorRT-7.0.0.11/lib")
link_directories("3rdparty/onnxruntime-win-x64-gpu-1.3.0/lib")
#link_libraries("nvinfer.lib")
link_libraries(${CUDA_LIBRARIES})
link_libraries("onnxruntime.lib")

# Include sub-projects.
add_subdirectory ("src")

include(CTest)
add_subdirectory("test")
