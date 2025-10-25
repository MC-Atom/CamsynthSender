//
// Created by Atom on 7/20/25.
//

#include <opencv2/xphoto.hpp>
#include <curl/curl.h>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>
#include <complex>

#include "frameProcessor.h"

#include <random>
#include <opencv2/features2d.hpp>

#include "dft.h"
#include "options.h"

FrameProcessor::FrameProcessor(FrameGrabber& grabber, Options& options): grabber(grabber), options(options) {

    blobDetect = cv::SimpleBlobDetector::create();
    auto params = cv::SimpleBlobDetector::Params();
    params.filterByArea = true;
    params.minArea = 100;
    params.filterByCircularity = false;
    params.filterByConvexity = false;
    params.filterByInertia = false;
    params.minThreshold = 0;
    params.maxThreshold = 255;

    blobDetect->setParams(params);
};

void FrameProcessor::start() {
    running = true;
    processingThread = std::thread(&FrameProcessor::run, this);
}

void FrameProcessor::stop() {
    running = false;
    if (processingThread.joinable())
        processingThread.join();
}

void FrameProcessor::run() {

    const int targetWidth = options.getTargetWidth();

    // Used code from https://curl.se/libcurl/c/http-post.html
    CURL *curl;
    CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_global_init() failed: %s\n", curl_easy_strerror(res));
    }

    curl = curl_easy_init();
    assert(curl);
    std::string urls [options.getPort().size()];
    for (int i = 0; i < options.getPort().size(); i++) {
        urls [i] = std::string("http://127.0.0.1:") + std::to_string(options.getPort()[i]);
    }

    // Used code from https://gist.github.com/priteshgohil/edce691cf557e7e3bb708ff100a18da3 for camera capture

    // create a window to display the images from the webcam


    // array to hold the original and resized frame
    cv::Mat inputFrame, resized;





    while (!grabber.getLatestFrame(inputFrame)){}
    // find the target width and height
    //inputFrame = cv::imread("../assets/star.jpg", cv::IMREAD_COLOR);
    accumulatedFrame = inputFrame.clone();
    int originalWidth = inputFrame.cols;
    int originalHeight = inputFrame.rows;
    // Calculate the scale ratio and new height
    double scale = static_cast<double>(targetWidth) / originalWidth;
    int newHeight = static_cast<int>(originalHeight * scale);

    const double ALPHA = 1;//0.4; // intensity of frame smearing

    double time = 0.0;
    const double STEP = 0.1;
    const int DEPTH = 100;

    while (running) {
        auto optionsSnapshot = options.getSnapshot();
        if (grabber.getLatestFrame(inputFrame)) {

            // display the frame until you press a key
            // capture the next frame from the webcam
            //WhiteBalance
            cv::Ptr<cv::xphoto::SimpleWB> wb = cv::xphoto::createSimpleWB();
            wb ->balanceWhite(inputFrame, inputFrame);

            //inputFrame = cv::imread("../assets/star.jpg", cv::IMREAD_COLOR);
            //cv::addWeighted(accumulatedFrame, 1.0 - ALPHA, inputFrame, ALPHA, 0, accumulatedFrame);
            accumulatedFrame = inputFrame;
            // Resize the frame
            cv::resize(inputFrame, resized, cv::Size(targetWidth, newHeight));


            // 1. Convert BGR to HSV
            cv::cvtColor(resized, hsv, cv::COLOR_BGR2HSV);
            // 2. Split into H, S, V
            std::vector<cv::Mat> hsvChannels;
            cv::split(hsv, hsvChannels);
            // 3. Extract Hue channel
            hue = hsvChannels[1];  // Hue is in [0,179]
            // 4. Optional: scale hue to 0–255 for display
            hue.convertTo(hueGray, CV_8UC1, 255.0 / 179.0);

            // 1. Convert to Greyscale
            cv::cvtColor(resized, grayscale, cv::COLOR_BGR2GRAY);


            // show the image on the window


            frame = grayscale;
            //cv::imshow("frame", frame);

            // 2. Up the contrast & brightness
            cv::Mat contrastFrame = cv::Mat::zeros( frame.size(), frame.type() );
            for( int y = 0; y < frame.rows; y++ ) {
                for( int x = 0; x < frame.cols; x++ ) {
                    for( int c = 0; c < frame.channels(); c++ ) {
                        contrastFrame.at<cv::Vec3b>(y,x)[c] =
                          cv::saturate_cast<uchar>( optionsSnapshot.contrast * frame.at<cv::Vec3b>(y,x)[c] + optionsSnapshot.brightness );
                    }
                }
            }
            frame = contrastFrame;

            // Apply Gaussian blur to smooth the input image
            //cv::blur(frame, temp, cv::Size(2, 2));
            //frame = temp;

            // Blob detection and isolation
            //if (optionsSnapshot.getBlobDetection()) {
                std::vector<cv::KeyPoint> keypoints;
                blobDetect->detect(frame,keypoints,blur);
                drawKeypoints( frame, keypoints, blur, cv::Scalar(0,0,255), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS );
                //frame = blur;
            //}
            //std::this_thread::sleep_for(std::chrono::milliseconds(10));

            // Apply Canny edge detection
            cv::Canny(frame, edges, 40, 180);
            // Morphological closing to close small holes in the edges
            //cv::morphologyEx(edges, cleaned, cv::MORPH_CLOSE, cv::Mat::ones(12, 12, CV_8U));


            int morphValue = 2;
            int imsize = frame.cols * frame.rows;
            int fillVal = imsize;

            while (morphValue < 32 && fillVal > (imsize * 0.9)) {
                cleaned = frame;
                cv::morphologyEx(edges, cleaned, cv::MORPH_CLOSE, cv::Mat::ones(morphValue, morphValue, CV_8U));
                fillVal = cv::floodFill(cleaned, cv::Point(0),255);
                morphValue++;
            }
            if (morphValue >= 32) {continue;}
            frame = cleaned;

            int borderSize = 20;
            borderSize = 20;
            cv::copyMakeBorder(
                frame,
                bordered,
                borderSize, borderSize,  // top, bottom
                borderSize, borderSize,  // left, right
                cv::BORDER_CONSTANT,
                cv::Scalar(255, 255, 255)
            );


            // Find contours
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(bordered.clone(), contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
            if (contours.empty()) {
                std::cerr << "No contours found\n";
                continue;
            }

            // Find the largest contour
            size_t largestContourIdx = 0;
            size_t SecondlargestContourIdx = 0;
            double maxArea = 0.0;
            double maxArea2 = 0.0;
            for (size_t i = 0; i < contours.size(); ++i) {
                double area = cv::contourArea(contours[i]);
                if (area > maxArea) {
                    maxArea2 = maxArea;
                    SecondlargestContourIdx = largestContourIdx;
                    maxArea = area;
                    largestContourIdx = i;
                } else if (area > maxArea2) {
                    maxArea2 = area;
                    SecondlargestContourIdx = i;
                }
            }
            std::vector<cv::Point> largestContour = contours[SecondlargestContourIdx];

            largestCont = cv::Mat::zeros(bordered.size(), CV_8UC3);
            for (cv::Point& pt : largestContour) {

                cv::circle(largestCont, pt, 1, cv::Scalar(0, 0, 255), -1);
            }



            // convert from cv Point to complex numbers
            std::vector<std::complex<double>> complexPoints;
            complexPoints.reserve(largestContour.size());
            for (const cv::Point& pt : largestContour) {
                complexPoints.emplace_back(static_cast<double>(pt.x), static_cast<double>(pt.y));
            }

            //auto rng = std::default_random_engine {2123};
            //std::shuffle(std::begin(complexPoints), std::end(complexPoints), rng);

             //cv::drawContours(blur, contours, -1, cv::Scalar(0, 255, 0), cv::FILLED);

            // THIS IS WHERE THE MAGIC HAPPENSS YIPEPEEEEEEEE!!!!
            std::vector<FourierComponent> fourierSeries = fourier(complexPoints,DEPTH);

            std::string jsonOut = "{\"harmonicSeries\":[";
            bool first = true;
            for (auto [freq, amp, phase] : fourierSeries) {

                if (true || freq > 0 && freq < 20){
                    if (!first) jsonOut.append(",");
                    first = false;

                    jsonOut.append("{\"Amp\":"+std::to_string(amp)+",\"Freq\":" +std::to_string(freq)+",\"Phase\":" +std::to_string(phase)+"}");
                }
            }
            jsonOut.append("]}");

            //jsonOut = R"({"harmonicSeries":[{"Amp":0.5,"Freq":2,"Phase":0}]})";

            for (std::string url : urls)  {
                curl = curl_easy_init();
                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonOut.c_str());
                res = curl_easy_perform(curl);
                if(res != CURLE_OK) fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                curl_easy_cleanup(curl);
            }

            fourierImage = cv::Mat::zeros(bordered.size(), CV_8UC3);
            int trailSize = 5000;
            for (int trail = -trailSize; trail <= 0; ++trail) {
                std::complex<double> sum(0, 0);
                for (auto [freq, amp, phase] : fourierSeries) {
                    double angle = std::fmod(phase + freq * (time + STEP * trail / 50), M_PI * 2);
                    std::complex<double> vec(std::cos(angle) * amp, std::sin(angle) * amp);
                    sum += vec;
                }
                cv::Point pt(sum.real(), sum.imag());
                cv::circle(fourierImage, pt, 3, cv::Scalar(0, 0, static_cast<int>(255.0 * (trailSize+trail) / trailSize)), -1);
                //std::cout << sum.real() << " " << sum.imag() << std::endl;
            }

            cv::Point lastPoint(0,0);
            for (auto [freq, amp, phase] : fourierSeries) {
                double angle = std::fmod(phase + freq * time, M_PI * 2);
                cv::Point nextPoint(lastPoint.x + std::cos(angle) * amp, lastPoint.y + std::sin(angle) * amp);
                cv::line(fourierImage, lastPoint, nextPoint, cv::Scalar(0, 0, 255), 2);
                lastPoint = nextPoint;
            }

            std::cout << time << std::endl;
            time += STEP;




            /*
            // Approximate contour to a polygon with roughly n edges
            double epsilon = 1.0;
            std::vector<cv::Point> approx;
            int attempts = 0;
            while (attempts++ < 100) {
                cv::approxPolyDP(largestContour, approx, epsilon, true);
                if ((int)approx.size() == desiredVertices)
                    break;
                if ((int)approx.size() > desiredVertices)
                    epsilon *= 1.2; // increase simplification
                else
                    epsilon *= 0.8; // decrease simplification
            }
            // Draw the contour result for visualization
            cv::Mat result(frame.size(), CV_8UC3, cv::Scalar(0));
            std::vector<std::vector<cv::Point>> toDraw{approx};
            cv::drawContours(result, toDraw, -1, cv::Scalar(0, 255, 0), 2);
            cv::imshow("Polygon Approximation", result);
            */


            /*
            // Draw smoothed curve using interpolation between points
            cv::Mat curveImg(frame.size(), CV_8UC3, cv::Scalar(0));
            for (size_t i = 0; i < largestContour.size(); ++i) {
                cv::Point p0 = largestContour[i % largestContour.size()];
                cv::Point p1 = largestContour[(i + 1) % largestContour.size()];

                // interpolate between p0 and p1
                for (float t = 0; t <= 1.0; t += 0.01f) {
                    float x = (1 - t) * p0.x + t * p1.x;
                    float y = (1 - t) * p0.y + t * p1.y;
                    cv::circle(curveImg, cv::Point(cvRound(x), cvRound(y)), 1, cv::Scalar(0, 255, 0), -1);
                }
            }
            cv::imshow("Curved Approximation", curveImg);
            */

            std::this_thread::sleep_for(std::chrono::milliseconds(10));


        }



    }
    curl_global_cleanup();
}