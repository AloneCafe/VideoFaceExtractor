
#include "Utils.h"

std::string Utils::msToHMS(double ms) {
	std::string s(32, 0);
	auto ms_part = static_cast<size_t>(ms) % 1000;
	auto seconds = static_cast<size_t>(ms) / 1000;
	auto h_part = seconds / 3600;
	auto m_part = seconds % 3600 / 60;
	auto s_part = seconds % 60;
	sprintf(s.data(), "%02d:%02d:%02d.%03d", h_part, m_part, s_part, ms_part);
	return s;
}

