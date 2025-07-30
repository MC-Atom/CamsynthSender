#pragma once
#include <opencv2/opencv.hpp>
#include <mutex>
#include <thread>

#include "Options.h"

class FrameGrabber
{
public:
    FrameGrabber(Options& options): options(options) { };
    ~FrameGrabber(){ stop(); };

    void start();
    void stop();

    // Gets a copy of the latest frame
    bool getLatestFrame(cv::Mat& frame);

private:
    void run();

    std::thread frameGrabThread;
    std::mutex frameMutex;
    cv::Mat latestFrame;
    bool running = false;
    Options& options;
};
