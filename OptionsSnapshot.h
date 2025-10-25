//
// Created by Devyn Oh on 7/30/25.
//

#ifndef OPTIONSSNAPSHOT_H
#define OPTIONSSNAPSHOT_H

struct OptionsSnapshot {
    std::vector<int> port;
    int cropLeft, cropRight, cropTop, cropBottom;
    int targetWidth;
    float contrast, brightness;
    int morphology;
    bool blobDetection;
    bool whiteBorder;
};

#endif //OPTIONSSNAPSHOT_H
