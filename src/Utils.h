#pragma once
#include <string>

struct Utils {
	static std::string msToHMS(double ms);

    static constexpr const char * CASCADE_FILE_PATH
        = "haarcascade_frontalface_default.xml";
};