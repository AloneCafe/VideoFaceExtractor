#pragma once

#include <string>


class Extractor {
private:

public:
    Extractor() = default;
    virtual ~Extractor() = default;

    int execute(const std::string & ffmpegPath, const std::string & inFile, const std::string & filt_path, const std::string & outFile);
};


