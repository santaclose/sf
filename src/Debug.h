#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace sf::Debug
{
	struct LogInfo
	{
		uint32_t color;
		char* text = nullptr;
	};
	void LogSeekBegin();
	bool LogGetNext(const LogInfo*& out);
	void Log(const char* fmt, ...);
	void LogColored(uint32_t color, const char* fmt, ...);
}