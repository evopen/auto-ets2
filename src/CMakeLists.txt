add_executable(autodriver 
	"ScreenCapturer.cpp"
 "LaneDetector.h" "logger.cpp" "ets2-sampler/Sampler.cpp")

add_executable(ets2-sampler "ets2-sampler/Sampler.cpp" "ScreenCapturer.cpp")

set(CMAKE_INCLUDE_CURRENT_DIR true)