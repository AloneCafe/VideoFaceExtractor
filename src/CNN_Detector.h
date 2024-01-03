#pragma once

#include <facedetectcnn.h>
#include <opencv2/opencv.hpp>

#define DETECT_BUFFER_SIZE 0x20000

#define SCALE 8

class CNN_Detector {
private:
    cv::VideoCapture _cap;

public:
    CNN_Detector(const cv::VideoCapture& cap) : _cap(cap) { }
    virtual ~CNN_Detector() = default;

    cv::VideoCapture * videoCapture() {
        return &_cap;
    }

    bool detect(cv::Mat & image, cv::Mat & marked_image) {
        int * pResults = NULL;

        std::shared_ptr<unsigned char[]> pBuffer(new unsigned char[DETECT_BUFFER_SIZE]);

        cv::Mat resizedImage;
        _cap >> image;
        if (image.empty())
            return false;
        //cv::resize(image, resizedImage, cv::Size(180, 120));
        cv::resize(image, resizedImage, cv::Size(320, 240));
        // 此处是按倍数缩放
        //const int scaled_width = _cap.get(cv::CAP_PROP_FRAME_WIDTH) / SCALE, scaled_height = _cap.get(cv::CAP_PROP_FRAME_HEIGHT) / SCALE;
        //cv::resize(image, resizedImage, cv::Size(scaled_width, scaled_height));

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

        marked_image = image.clone();
        for (int i = 0; i < (pResults ? *pResults : 0); i++)
        {
            short * p = ((short*)(pResults + 1)) + 16 * i;
            int confidence = p[0];
            int x = p[1] * SCALE;
            int y = p[2] * SCALE;
            int w = p[3] * SCALE;
            int h = p[4] * SCALE;

            //show the score of the face. Its range is [0-100]
            char sScore[256];
            snprintf(sScore, 256, "%d", confidence);
            cv::putText(marked_image, sScore, cv::Point(x, (y-3)),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 200), 1);

            //draw face rectangle
            rectangle(marked_image, cv::Rect(x, y, w, h), cv::Scalar(0, 0, 200), 1);
            //draw five face landmarks in different colors
            cv::circle(marked_image, cv::Point(p[5] * SCALE, p[5 + 1] * SCALE), 1, cv::Scalar(255, 0, 0), 2);
            cv::circle(marked_image, cv::Point(p[5 + 2] * SCALE, p[5 + 3] * SCALE), 1, cv::Scalar(0, 0, 255), 2);
            cv::circle(marked_image, cv::Point(p[5 + 4] * SCALE, p[5 + 5] * SCALE), 1, cv::Scalar(0, 255, 0), 2);
            cv::circle(marked_image, cv::Point(p[5 + 6] * SCALE, p[5 + 7] * SCALE), 1, cv::Scalar(255, 0, 255), 2);
            cv::circle(marked_image, cv::Point(p[5 + 8] * SCALE, p[5 + 9] * SCALE), 1, cv::Scalar(0, 255, 255), 2);

            //print the result
//            printf("face %d: confidence=%d, [%d, %d, %d, %d] (%d,%d) (%d,%d) (%d,%d) (%d,%d) (%d,%d)\n",
//                   i, confidence, x, y, w, h,
//                   p[5] * SCALE, p[6] * SCALE, p[7] * SCALE, p[8] * SCALE, p[9] * SCALE,
//                   p[10] * SCALE, p[11] * SCALE, p[12] * SCALE, p[13] * SCALE ,p[14] * SCALE);

        }

        cvtm.stop();

        if (pResults) {
            //printf("time = %gms, %d faces detected.\n", cvtm.getTimeMilli(), (pResults ? *pResults : 0));
        }
        return pResults != nullptr && *pResults != 0;
    }

    bool detect() {
        cv::Mat image, marked_image;
        return detect(image, marked_image);
    }
};
