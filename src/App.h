#pragma once
#include <string>
#include <dlib/opencv.h>
#include <opencv2/opencv.hpp>
#include <thread>

#include "FaceDetector.h"
#include "ListWriter.h"
#include "Extractor.h"

struct App {

	template <typename CUTCFG>
    int full_run(
            const std::string& inFile,
            const CUTCFG& cutcfg, /*const std::string & outDir, */
            const std::string& outLst, const std::string& outFlt,
            const std::string& ffmpegPath, const std::string& outFile);

	static void testFeatures() {
#ifdef __AVX__
		std::cout << "AVX on" << std::endl;
#endif
#ifdef DLIB_HAVE_SSE2
		std::cout << "DLIB_HAVE_SSE2 on" << std::endl;
#endif
#ifdef DLIB_HAVE_SSE3
		std::cout << "DLIB_HAVE_SSE3 on" << std::endl;
#endif
#ifdef DLIB_HAVE_SSE41
		std::cout << "DLIB_HAVE_SSE41 on" << std::endl;
#endif
#ifdef DLIB_HAVE_AVX
		std::cout << "DLIB_HAVE_AVX on" << std::endl;
#endif
	}

    template <typename CUTCFG>
    int only_detect_run(const std::string& inFile,
                        const CUTCFG& cutcfg,
                        const std::string& outLst,
                        const std::string & outFlt);

    int only_split_run(const std::string& inFile,
                       const std::string& outLst,
                       const std::string & outFlt,
                       const std::string& ffmpegPath,
                       const std::string& outFile);
};

template <typename CUTCFG>
int App::only_detect_run(const std::string& inFile,
                    const CUTCFG& cutcfg,
                    const std::string& outLst,
                    const std::string & outFlt) {
    // 打开视频文件，以便获取帧率、总帧数
    cv::VideoCapture cap(inFile);
    if (!cap.isOpened()) {
        std::cout << "无法打开视频文件" << std::endl;
        return 1;
    }
    double frameTotalCount = cap.get(cv::CAP_PROP_FRAME_COUNT);
    double fps = cap.get(cv::CAP_PROP_FPS);
    std::cout << "total frames: " << frameTotalCount << std::endl;
    cap.release();

    //auto cpus = 1;
    auto cpus = std::thread::hardware_concurrency();
    auto framesPerThread = frameTotalCount / cpus;

    // 初始化输出清单
    bool ret = ListWriter::globalOpen(outLst, outFlt);
    if (!ret) {
        std::cerr << "错误，无法打开输出文件" << std::endl;
        return 1;
    }

    std::vector<std::thread> tr_grp;

    // 按硬件并发数分割任务
    for (size_t i = 0; i < cpus; ++i) {
        auto left = i * framesPerThread;
        auto right = left + framesPerThread;
        if (i == cpus - 1) {
            right = frameTotalCount - 1;
        }
        std::cout << "run_range: " << left << " -> " << right << std::endl;
        std::thread tr([&inFile, &cutcfg, left, right, fps] {
            FaceDetector fd(inFile, cutcfg);
            //fd.run_range(left, right, fps);
            fd.run_range(left, right, 1);
        });
        tr_grp.push_back(std::move(tr));
    }

    // 等待所有异步操作结束
    for (auto& tr : tr_grp) {
        tr.join();
    }

    ListWriter::globalClose();
    return 0;
}

template <typename CUTCFG>
int App::full_run(
        const std::string& inFile,
        const CUTCFG& cutcfg,
        const std::string& outLst,
        const std::string & outFlt,
        const std::string& ffmpegPath,
        const std::string& outFile)
{

    only_detect_run(inFile, cutcfg, outLst, outFlt);
    only_split_run(inFile, outLst, outFlt, ffmpegPath, outFile);
	return 0;
}