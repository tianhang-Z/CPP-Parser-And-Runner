#include "logger.h"

namespace logger {
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
    LOG_LEVEL logLevel =
#ifndef NDEBUG
        LOG_LEVEL::LOG_LEVEL_DEBUG;
#else
        LOG_LEVEL::LOG_LEVEL_INFO;
#endif
}