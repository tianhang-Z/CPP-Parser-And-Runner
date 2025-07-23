#include "logger.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace Logger {
    // ����ȫ�ֱ���
    std::string logFileName;
    std::ofstream ofs;
    std::mutex logMutex;

    // ������־�����ַ���
    const char* logLevelStr[] = {
        "[ TRACE]", "[ DEBUG]", "[  INFO]",
        "[  WARN]", "[ ERROR]", "[ FATAL]"
    };

    // ������־����������ʼ����
    LogLevel logLevel =
#ifndef NDEBUG
        LogLevel::Debug;
#else
        LogLevel::Info;
#endif


} // namespace Logger