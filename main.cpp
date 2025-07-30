#include "FrameGrabber.h"
#include "FrameProcessor.h"
#include "options.h"

Options options = Options();
int brightness_slider = 300;
int contrast_slider = 20;

static void on_brightbar( int, void* )
{
    auto snapshot = options.getSnapshot();
    snapshot.brightness = brightness_slider - 400;
    options.setOptions(snapshot);
}

static void on_contrastbar( int, void* )
{
    auto snapshot = options.getSnapshot();
    snapshot.contrast = contrast_slider / 10.0f;
    options.setOptions(snapshot);
}

int main() {

    OptionsSnapshot snapshot;
    snapshot.port = 8080;
    snapshot.blobDetection = true;
    snapshot.cropLeft = 0;
    snapshot.cropRight = 0;
    snapshot.cropTop = 0;
    snapshot.cropBottom = 0;

    snapshot.targetWidth = 500;
    snapshot.brightness = brightness_slider - 400;
    snapshot.contrast = contrast_slider / 10.0f;

    options.setOptions(snapshot);

    FrameGrabber grabber(options);
    FrameProcessor processor(grabber,options);

    grabber.start();
    processor.start();

    cv::namedWindow("Webcam", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("HSV", cv::WINDOW_AUTOSIZE);
    //cv::namedWindow("Greyscale", cv::WINDOW_AUTOSIZE);

    namedWindow("Options", cv::WINDOW_AUTOSIZE); // Create Window
    cv::createTrackbar( "Brightness: " + std::to_string(brightness_slider / 5), "Options", &brightness_slider, 500, on_brightbar );
    cv::createTrackbar( "Contrast: " + std::to_string(brightness_slider/10.0f), "Options", &contrast_slider, 100, on_contrastbar );

    bool quit = false;
    while (!quit) {
        if (!processor.accumulatedFrame.empty()) {
            cv::imshow("Webcam", processor.accumulatedFrame);
            //if (!processor.hueGray.empty()) cv::imshow("HSV", processor.hueGray);
            //if (!processor.grayscale.empty()) cv::imshow("Greyscale", processor.grayscale);
            if (!processor.blur.empty()) cv::imshow("Blur", processor.blur);
            if (!processor.edges.empty()) cv::imshow("Edges", processor.cleaned);
            if (!processor.largestCont.empty()) cv::imshow("largestCont", processor.largestCont);
            if (!processor.fourierImage.empty()) cv::imshow("fourierImage", processor.fourierImage);
        }

        // wait (10ms) for esc key to be pressed to stop
        if (cv::waitKey(10) == 27)
            quit = true;
    }

    processor.stop();
    grabber.stop();

    return 0;
}



