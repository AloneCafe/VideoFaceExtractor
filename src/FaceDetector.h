#pragma once
#include <memory>
#include <opencv2/opencv.hpp>
#include "CNN_Detector.h"

struct CutConfig {
    bool useFrame;   // 是否使用帧为单位时间，而不是秒
    double ignThs;   // 忽略阈值：当人脸持续时间少于 N 个单位时间，则忽略它
    double cutThs;   // 切断阈值：当后续 N 个单位时间不再存在人脸，则进行切分
    double preComp;  // 前置补偿：该人脸出现之前，额外在切分的视频中补偿前面的 N 个单位时间的影像
    double postComp; // 后置补偿：该人脸消失之后，额外在切分的视频中补偿后面的 N 个单位时间的影像

};

struct CutConfigByFrame {

    double ignThs;   // 忽略阈值：当人脸持续时间少于 N 个帧，则忽略它
    double cutThs;   // 切断阈值：当后续 N 个帧不再存在人脸，则进行切分

    double preComp;  // 前置补偿：该人脸出现之前，额外在切分的视频中补偿前面的 N 个帧的影像
    double postComp; // 后置补偿：该人脸消失之后，额外在切分的视频中补偿后面的 N 个帧的影像

    CutConfigByFrame() = default;
    CutConfigByFrame(const CutConfig& rhs, double fps) {
        if (!rhs.useFrame) {
            ignThs = rhs.ignThs * fps;
            cutThs = rhs.cutThs * fps;
            preComp = rhs.preComp * fps;
            postComp = rhs.postComp * fps;
        } else {
            ignThs = rhs.ignThs;
            cutThs = rhs.cutThs;
            preComp = rhs.preComp;
            postComp = rhs.postComp;
        }
    }
};

class FaceDetector {
public:
    FaceDetector(const std::string & inFile, const CutConfigByFrame &cutcfg);
    FaceDetector(const std::string & inFile, const CutConfig &cutcfg);
private:
    CutConfigByFrame _cutcfg{};
    cv::VideoCapture _vc;
    CNN_Detector _detector;

public:
	//using FrameGrabber::FrameGrabber;
	FaceDetector() = delete;
	virtual ~FaceDetector() = default;

	//size_t relocateFrame_R2L(bool has_faces, size_t left, size_t right);

	void run_range(size_t left, size_t right, size_t step);

    bool detect();

    void setFramePos(size_t count);
    size_t getFramePos();

    double getFrameMSec();
    double getFPS();
};