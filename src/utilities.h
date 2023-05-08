#pragma once

#include <random>
#include <sstream>

namespace uuid {
	static std::random_device              rd;
	static std::mt19937                    gen(rd());
	static std::uniform_int_distribution<> dis(0, 15);
	static std::uniform_int_distribution<> dis2(8, 11);

	static std::string generate_uuid_v4() {
		std::stringstream ss;
		int i;
		ss << std::hex;
		for (i = 0; i < 8; i++) {
			ss << dis(gen);
		}
		ss << "-";
		for (i = 0; i < 4; i++) {
			ss << dis(gen);
		}
		ss << "-4";
		for (i = 0; i < 3; i++) {
			ss << dis(gen);
		}
		ss << "-";
		ss << dis2(gen);
		for (i = 0; i < 3; i++) {
			ss << dis(gen);
		}
		ss << "-";
		for (i = 0; i < 12; i++) {
			ss << dis(gen);
		};
		return ss.str();
	}
}

namespace StringHelper
{
	template <typename... Types>
	static std::string Printf(const char* format, Types... Args)
	{
		int nSize = std::snprintf(nullptr, 0, format, Args...);
		std::unique_ptr<char[]> pBuffer = std::make_unique<char[]>(nSize);
		std::snprintf(pBuffer.get(), nSize, format, Args...);
		return std::string(pBuffer.get(), pBuffer.get() + nSize - 1);
	}
}