#pragma once
#include <string>
#include <dlib/opencv.h>
#include <opencv2/opencv.hpp>
#include <thread>
#include <csignal>

#include "FaceDetector.h"
#include "ListWriter.h"
#include "Extractor.h"

struct App {

    static std::map<int, std::shared_ptr<cv::VideoWriter>> outs;
    static std::map<int, std::shared_ptr<CNN_Detector>> caps;

    static void sig_handler(int sig) {

        for (auto & out : outs) {
            std::cout << "[!] 释放摄像头 CAM" << out.first << " 并同步输出视频文件 " << std::endl;
            out.second->release();
        }

        std::cout << "[!] 监控模拟程序将退出" << std::endl;

        exit(0);
    }

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

    template <typename CUTCFG>
    int full_run(const std::vector<int> &cameras, const CUTCFG &cutcfg, const std::string &outLst,
             const std::string &outFlt,
             const std::string &ffmpegPath, const std::string &outFile);
};

template <typename CUTCFG>
int App::only_detect_run(const std::string& inFile,
                    const CUTCFG& cutcfg,
                    const std::string& outLst,
                    const std::string & outFlt) {
    int64_t begin, end;
    begin = cv::getTickCount();

    std::vector<std::thread> tr_grp(0);
    auto cpus = 1;
    //auto cpus = std::thread::hardware_concurrency();
    size_t framesPerThread;
    double frameTotalCount, fps;
    int ret = 0;

    // 打开视频文件，以便获取帧率、总帧数
    cv::VideoCapture cap(inFile);
    if (!cap.isOpened()) {
        std::cout << "无法打开视频文件" << std::endl;
        ret = 1;
        goto END;
    }
    frameTotalCount = cap.get(cv::CAP_PROP_FRAME_COUNT);
    fps = cap.get(cv::CAP_PROP_FPS);
    std::cout << "total frames: " << frameTotalCount << std::endl;
    cap.release();

    framesPerThread = frameTotalCount / cpus;

    // 初始化输出清单
    ret = ListWriter::globalOpen(outLst, outFlt);
    if (!ret) {
        std::cerr << "错误，无法打开输出文件" << std::endl;
        ret = 1;
        goto END;
    }

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
    ret = 0;

END:
    end = cv::getTickCount();
    std::cout << "[*] detect procedure " << (ret == 0 ? "completed" : "failed") << ", spend " << double(end-begin) / cv::getTickFrequency() << "s" << std::endl;

    return ret;
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
    int64_t begin, end;
    begin = cv::getTickCount();
    auto ret1 = only_detect_run(inFile, cutcfg, outLst, outFlt);
    auto ret2 = only_split_run(inFile, outLst, outFlt, ffmpegPath, outFile);
    end = cv::getTickCount();
    std::cout << "[*] progress " << (ret1 == 0 && ret2 == 0 ? "completed" : "failed" ) << ", spend " << double(end-begin) / cv::getTickFrequency() << "s" << std::endl;
	return ret1 == 0 && ret2 == 0;
}

template <typename CUTCFG>
int App::full_run(
        const std::vector<int>& cameras,
        const CUTCFG& cutcfg,
        const std::string& outLst,
        const std::string & outFlt,
        const std::string& ffmpegPath,
        const std::string& outFile)
{
//    int64_t begin, end;
//    begin = cv::getTickCount();
//    auto ret1 = only_detect_run(cameras, cutcfg, outLst, outFlt);
//    auto ret2 = only_split_run(cameras, outLst, outFlt, ffmpegPath, outFile);
//    end = cv::getTickCount();
//    std::cout << "[*] progress " << (ret1 == 0 && ret2 == 0 ? "completed" : "failed" ) << ", spend " << double(end-begin) / cv::getTickFrequency() << "s" << std::endl;
//    return ret1 == 0 && ret2 == 0;

    for (auto cam_i : cameras) {

        cv::VideoCapture cap0 (cam_i);
        if (!cap0.isOpened()) {
            std::cerr << "[!] 系统摄像头 CAM-" << cam_i << " 无法被打开!" << std::endl;
            return 1;
        } else {
            std::shared_ptr<CNN_Detector> cap = std::make_shared<CNN_Detector>(cap0);
            auto fps = cap->videoCapture()->get(cv::CAP_PROP_FPS);
            auto width = cap->videoCapture()->get(cv::CAP_PROP_FRAME_WIDTH);
            auto height = cap->videoCapture()->get(cv::CAP_PROP_FRAME_HEIGHT);

            caps.insert(std::make_pair(cam_i, std::move(cap)));

            std::stringstream ss;
            ss << outFile << ".CAM" << cam_i << ".mp4";

            std::shared_ptr<cv::VideoWriter> out = std::make_shared<cv::VideoWriter>(ss.str(),
                                cv::VideoWriter::fourcc('H', '2', '6', '4'), fps,
                                cv::Size(width, height));
            if (!out->isOpened()) {
                std::cerr << "[X] 错误，无法打开视频文件进行写入：" << ss.str() << std::endl;
                return 1;
            } else {
                outs.insert(std::make_pair(cam_i, std::move(out)));
            }
        }
    }

    signal(SIGINT, App::sig_handler);
    signal(SIGTERM, App::sig_handler);
    signal(SIGABRT, App::sig_handler);

    std::cout << "[ ] 程序监视列表：";
    for (auto & e : caps) {
        std::cout << e.first << " ";
    }
    std::cout << "\n";


    std::vector<std::thread> trs;
    for (auto & e : caps) {
        std::thread tr([&]() {

            int cam_i = e.first;
            auto cap = e.second;
            auto out = outs[cam_i];

            std::string cam_name = std::format("CAM-{}", cam_i);

            cv::namedWindow(cam_name);

            size_t outFrameCount = 0;
            while (true) try {
                cv::Mat img, marked_img;
                bool hasFace = cap->detect(img, marked_img);

                {
                    // 获取当前时间点
                    auto now = std::chrono::system_clock::now();
                    // 转换为时间类型
                    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
                    // 获取毫秒部分
                    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
                    std::stringstream ss;
                    ss << cam_name << " " << std::put_time(std::localtime(&now_c), "%Y-%m-%dT%H:%M:%S") << "."
                       << std::setfill('0') << std::setw(3) << ms.count() << " FPS " << cap->videoCapture()->get(cv::CAP_PROP_FPS);
                    //ss << std::put_time(std::localtime(&t), "%Y年%m月%d日%H时%M分%S秒");

                    cv::putText(marked_img, ss.str(), cv::Point(0, 12),
                                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 2);
                }

                cv::imshow(cam_name, marked_img);

                if (hasFace) {
                    out->write(img);
                    outFrameCount++;
                }

                cv::waitKey(15);

            } catch (...) {
                std::cerr << "[X] " << cam_name << " 关闭，工作停止" << std::endl;
                out->release();
                return 1;
            }

        });
        trs.push_back(std::move(tr));
    }

    // 等待所有线程都执行完毕
    for (auto & tr : trs) {
        tr.join();
    }


    return 0;
}