//
// Created by Devyn Oh on 7/29/25.
//

#ifndef OPTIONS_H
#define OPTIONS_H
#include <mutex>
#include <thread>

#include "OptionsSnapshot.h"


class Options {

public:
    Options() = default;

    OptionsSnapshot getSnapshot() {
        std::lock_guard<std::mutex> lock(mu);

        return {
            port, cropLeft, cropRight, cropTop, cropBottom,
            targetWidth, contrast, brightness, blobDetection, whiteBorder
        };
    }

    bool getLock() {
        return mu.try_lock();
    }
    void unlock() {
        mu.unlock();
    }

    std::vector<int> getPort() { std::lock_guard<std::mutex> lock(mu); return port;}
    int getTargetWidth() { std::lock_guard<std::mutex> lock(mu); return targetWidth;}

    void setOptions(OptionsSnapshot snapshot) {
        std::lock_guard<std::mutex> lock(mu);
        port = snapshot.port;
        cropLeft = snapshot.cropLeft;
        cropRight = snapshot.cropRight;
        cropTop = snapshot.cropTop;
        cropBottom = snapshot.cropBottom;
        targetWidth = snapshot.targetWidth;
        contrast = snapshot.contrast;
        brightness = snapshot.brightness;
        blobDetection = snapshot.blobDetection;
        whiteBorder = snapshot.whiteBorder;
    }

private:
    std::mutex mu;

    std::vector<int> port = {};
    int cropLeft = 0;
    int cropRight = 0;
    int cropTop = 0;
    int cropBottom = 0;
    int targetWidth = 500;
    float contrast = 2;
    float brightness = 0.0;
    bool blobDetection = false;
    bool whiteBorder = false;
};



#endif //OPTIONS_H
