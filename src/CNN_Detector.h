#pragma once

#include <facedetectcnn.h>
#include <opencv2/opencv.hpp>

#define DETECT_BUFFER_SIZE 0x20000

class CNN_Detector {
private:
    cv::VideoCapture & _cap;

public:
    CNN_Detector(cv::VideoCapture & cap) : _cap(cap) { }
    virtual ~CNN_Detector() = default;

    cv::VideoCapture * videoCapture() {
        return &_cap;
    }

    bool detect() {
        int * pResults = NULL;

        std::shared_ptr<unsigned char[]> pBuffer(new unsigned char[DETECT_BUFFER_SIZE]);

        cv::Mat image, resizedImage;
        _cap >> image;
        if (image.empty())
            return false;
        //cv::resize(image, resizedImage, cv::Size(320, 240));
        cv::resize(image, resizedImage, cv::Size(160, 120));
        //cv::resize(image, resizedImage, cv::Size(640, 480));

        cv::TickMeter cvtm;
        cvtm.start();
        /** The function that loads the face detection model.
         @param result_buffer Buffer memory for storing face detection results, whose size must be 0x20000 * bytes.
         @param rgb_image_data Input image, which must be BGR (three channels) instead of RGB image.
         @param width The width of the input image.
         @param height The height.
         @param step The step.
         @return An int pointer reflecting the face detection result, see Example for detailed usage. */
        pResults = facedetect_cnn(pBuffer.get(),
                                  (unsigned char*)(resizedImage.ptr(0)),
                                  resizedImage.cols, resizedImage.rows,
                                  (int)resizedImage.step);
        cvtm.stop();
        if (pResults) {
            //printf("time = %gms, %d faces detected.\n", cvtm.getTimeMilli(), (pResults ? *pResults : 0));
        }
        return pResults != nullptr && *pResults != 0;
    }
};
