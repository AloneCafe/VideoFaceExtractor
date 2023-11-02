#include <dlib/opencv.h>
#include <opencv2/opencv.hpp>

#include "App.h"
#include "ListWriter.h"

int App::only_split_run(const std::string &inFile, const std::string &outLst, const std::string &outFlt,
                        const std::string &ffmpegPath, const std::string &outFile) {
    return Extractor{}.execute(ffmpegPath, inFile, outFlt, outFile);
}
