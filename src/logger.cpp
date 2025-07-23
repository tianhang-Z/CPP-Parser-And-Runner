#include "logger.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace Logger {
    // 定义全局变量
    std::string logFileName;
    std::ofstream ofs;
    std::mutex logMutex;

    // 定义日志级别字符串
    const char* logLevelStr[] = {
        "[ TRACE]", "[ DEBUG]", "[  INFO]",
        "[  WARN]", "[ ERROR]", "[ FATAL]"
    };

    // 定义日志级别（条件初始化）
    LogLevel logLevel =
#ifndef NDEBUG
        LogLevel::Debug;
#else
        LogLevel::Info;
#endif


} // namespace Logger