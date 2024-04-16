#pragma once

#include <ostream>
#include <string>
#include <chrono>

#include "tinyformat.h"

class Logger
{
public:


	static void SetStream(std::ostream* stream);

	static void Log(const std::string& message);

	template<typename... Args>
	static void Log(const char* fmt, const Args&... args)
	{
		Log(tfm::format(fmt, args...));
	}

private:
	static std::ostream *stream;
};
