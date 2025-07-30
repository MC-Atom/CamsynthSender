//
// Created by Devyn Oh on 7/20/25.
//

#include "frameGrabber.h"
#include <thread>

void FrameGrabber::start() {
    running = true;
    frameGrabThread = std::thread(&FrameGrabber::run, this);
}

void FrameGrabber::stop() {
    running = false;
    if (frameGrabThread.joinable())
        frameGrabThread.join();
}

void FrameGrabber::run() {

    // open the first webcam plugged in the computer
    cv::VideoCapture camera(0); // in linux check $ ls /dev/video0
    if (!camera.isOpened()) {
        std::cerr << "ERROR: Could not open camera" << std::endl;
    }
    // camera.set(cv::CAP_PROP_AUTO_EXPOSURE, 0.25); // manual mode
    // camera.set(cv::CAP_PROP_EXPOSURE, 1);


    while (running) {
        OptionsSnapshot snapshot = options.getSnapshot();
        // capture the next frame from the webcam
        cv::Mat frame;
        //camera.set(cv::CAP_PROP_AUTO_EXPOSURE, 0.25);
        //camera.set(cv::CAP_PROP_EXPOSURE , 1);
        camera >> frame;
        auto width = frame.size().width;
        auto height = frame.size().height;
        auto left = snapshot.cropLeft;
        auto top = snapshot.cropTop;

        cv::Rect crop(left, top, width-left-snapshot.cropRight, height-top-snapshot.cropRight);
        frame = frame(crop);

        std::lock_guard<std::mutex> lock(frameMutex);
        latestFrame = frame.clone();  // overwrite with newest frame
    }
}

bool FrameGrabber::getLatestFrame(cv::Mat& frame) {
    std::lock_guard<std::mutex> lock(frameMutex);
    if (latestFrame.empty())
        return false;
    frame = latestFrame.clone();
    return true;
}