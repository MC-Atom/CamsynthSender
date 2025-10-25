//
// Created by Atom on 7/20/25.
//

#pragma once
#include <opencv2/opencv.hpp>
#include "frameGrabber.h"


class FrameProcessor
{
public:
    FrameProcessor(FrameGrabber& grabber, Options& options);
    ~FrameProcessor() {stop();};

    void start();
    void stop();

    // Frame to sum with for smear
    cv::Mat accumulatedFrame;
    // array to hold the hueGray and grayscale frames
    cv::Mat hsv, hue, hueGray, grayscale;
    // array to hold the last frame
    cv::Mat frame;
    // Edge Detection
    cv::Mat blur, edges, cleaned, bordered;
    // other stuff
    cv::Mat largestCont, fourierImage, temp;

private:
    void run();

    cv::Ptr<cv::SimpleBlobDetector> blobDetect;
    FrameGrabber& grabber;
    std::thread processingThread;
    bool running = false;
    Options& options;
};

