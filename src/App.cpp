#include <dlib/opencv.h>
#include <opencv2/opencv.hpp>

#include "App.h"
#include "ListWriter.h"

int App::only_split_run(const std::string &inFile, const std::string &outLst, const std::string &outFlt,
                        const std::string &ffmpegPath, const std::string &outFile) {

    int64_t begin, end;
    begin = cv::getTickCount();
    auto ret = Extractor{}.execute(ffmpegPath, inFile, outFlt, outFile);
    end = cv::getTickCount();
    std::cout << "[*] split procedure " << (ret == 0 ? "completed" : "failed") << ", spend " << double(end-begin) / cv::getTickFrequency() << "s" << std::endl;
    return ret;
}
