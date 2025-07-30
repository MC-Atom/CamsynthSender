cmake_minimum_required(VERSION 3.10)
project(WebcamSynth)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify the location of the OpenCV cmake config file (Apple Silicon path)
set(OpenCV_DIR "/opt/homebrew/opt/opencv/share/opencv4")

find_package(OpenCV REQUIRED core imgproc highgui xphoto video)
find_package(CURL REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} main.cpp
        dft.cpp
        dft.h
        frameProcessor.cpp
        frameProcessor.h
        frameGrabber.cpp
        frameGrabber.h
        options.h
        OptionsSnapshot.h
)
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS} CURL)
