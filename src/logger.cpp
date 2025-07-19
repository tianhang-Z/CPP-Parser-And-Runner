#include "logger.h"

namespace logger {
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
    LOG_LEVEL logLevel =
#ifndef NDEBUG
        LOG_LEVEL::LOG_LEVEL_DEBUG;
#else
        LOG_LEVEL::LOG_LEVEL_INFO;
#endif
}