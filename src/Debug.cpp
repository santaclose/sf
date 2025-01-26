#include "Debug.h"

#include <stdio.h>
#include <stdarg.h>

#define LOG_COUNT 1024
#define LOG_SIZE 256

namespace sf::Debug
{
    std::vector<LogInfo> logs(LOG_COUNT);
    uint32_t writeIndex = 0;
    uint32_t beginIndex = 0;
    uint32_t nextLogToGet = 0;
    uint32_t currentCount = 0;
    uint32_t nextLogCounter = 0;
}

void sf::Debug::LogSeekBegin()
{
    nextLogToGet = beginIndex;
    nextLogCounter = 0;
}

bool sf::Debug::LogGetNext(const LogInfo*& out)
{
    if (nextLogCounter == currentCount)
        return false;

    out = &logs[nextLogToGet];

    nextLogToGet = (nextLogToGet + 1) % LOG_COUNT;
    nextLogCounter++;
    return true;
}

void sf::Debug::Log(const char* fmt, ...)
{
    currentCount = currentCount + 1 > LOG_COUNT ? LOG_COUNT : currentCount + 1;
    if (logs[writeIndex].text == nullptr)
        logs[writeIndex].text = new char[LOG_SIZE];
    logs[writeIndex].color = ~0U;

    va_list args;
    va_start(args, fmt);
    vsnprintf(logs[writeIndex].text, LOG_SIZE - 1, fmt, args);
    va_end(args);
    if (currentCount == LOG_COUNT && writeIndex == beginIndex)
        beginIndex = (beginIndex + 1) % LOG_COUNT;
    writeIndex = (writeIndex + 1) % LOG_COUNT;
}

void sf::Debug::LogColored(uint32_t color, const char* fmt, ...)
{
    currentCount = currentCount + 1 > LOG_COUNT ? LOG_COUNT : currentCount + 1;
    if (logs[writeIndex].text == nullptr)
        logs[writeIndex].text = new char[LOG_SIZE];
    logs[writeIndex].color = color;

    va_list args;
    va_start(args, fmt);
    vsnprintf(logs[writeIndex].text, LOG_SIZE - 1, fmt, args);
    va_end(args);
    if (currentCount == LOG_COUNT && writeIndex == beginIndex)
        beginIndex = (beginIndex + 1) % LOG_COUNT;
    writeIndex = (writeIndex + 1) % LOG_COUNT;
}
