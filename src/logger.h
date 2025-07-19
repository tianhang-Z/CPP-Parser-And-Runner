#ifndef LOGGER_H
#define LOGGER_H

#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>
#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

namespace logger {
    // 外部声明全局变量
    extern std::string logFileName;
    extern std::ofstream ofs;
    extern std::mutex logMutex;

    // 日志级别字符串数组
    extern const char* logLevelStr[];

    // 日志级别枚举
    enum class LOG_LEVEL : unsigned {
        LOG_LEVEL_TRACE = 0,
        LOG_LEVEL_DEBUG,
        LOG_LEVEL_INFO,
        LOG_LEVEL_WARN,
        LOG_LEVEL_ERROR,
        LOG_LEVEL_FATAL
    };

    // 日志级别全局变量声明
    extern LOG_LEVEL logLevel;

    namespace internal {
        // 内部日志功能函数声明
        template <typename... Args>
        void logSys(const std::string& file, int line, bool to_abort,
            const char* fmt, Args... args);

        template <typename... Args>
        void logBase(const std::string& file, int line, LOG_LEVEL level,
            bool to_abort, const char* fmt, Args... args);
    } // namespace internal

    // 设置日志级别
    inline void setLogLevel(LOG_LEVEL rhs);

    // 设置日志文件
    inline void setLogFile(const std::string& fileName);
} // namespace logger

// 日志宏定义
#define TRACE(fmt, ...)  \
    if (static_cast<unsigned>(logger::logLevel) <= static_cast<unsigned>(logger::LOG_LEVEL::LOG_LEVEL_TRACE)) \
    logger::internal::logBase(__FILE__, __LINE__, \
        logger::LOG_LEVEL::LOG_LEVEL_TRACE, false, fmt, ##__VA_ARGS__)

#define DEBUG(fmt, ...)  \
    if (static_cast<unsigned>(logger::logLevel) <= static_cast<unsigned>(logger::LOG_LEVEL::LOG_LEVEL_DEBUG)) \
    logger::internal::logBase(__FILE__, __LINE__, \
        logger::LOG_LEVEL::LOG_LEVEL_DEBUG, false, fmt, ##__VA_ARGS__)

#define INFO(fmt, ...)   \
    if (static_cast<unsigned>(logger::logLevel) <= static_cast<unsigned>(logger::LOG_LEVEL::LOG_LEVEL_INFO)) \
    logger::internal::logBase(__FILE__, __LINE__, \
        logger::LOG_LEVEL::LOG_LEVEL_INFO, false, fmt, ##__VA_ARGS__)

#define WARN(fmt, ...)   \
    if (static_cast<unsigned>(logger::logLevel) <= static_cast<unsigned>(logger::LOG_LEVEL::LOG_LEVEL_WARN)) \
    logger::internal::logBase(__FILE__, __LINE__, \
        logger::LOG_LEVEL::LOG_LEVEL_WARN, false, fmt, ##__VA_ARGS__)

#define ERROR(fmt, ...)  \
    if (static_cast<unsigned>(logger::logLevel) <= static_cast<unsigned>(logger::LOG_LEVEL::LOG_LEVEL_ERROR)) \
    logger::internal::logBase(__FILE__, __LINE__, \
        logger::LOG_LEVEL::LOG_LEVEL_ERROR, false, fmt, ##__VA_ARGS__)

#define FATAL(fmt, ...)  \
    logger::internal::logBase(__FILE__, __LINE__, \
        logger::LOG_LEVEL::LOG_LEVEL_FATAL, true, fmt, ##__VA_ARGS__)

#define SYSERR(fmt, ...) \
    logger::internal::logSys(__FILE__, __LINE__, false, fmt, ##__VA_ARGS__)

#define SYSFATAL(fmt, ...) \
    logger::internal::logSys(__FILE__, __LINE__, true, fmt, ##__VA_ARGS__)

// 内联函数的实现
namespace logger {

    inline void setLogLevel(LOG_LEVEL rhs) {
        logLevel = rhs;
    }

    inline void setLogFile(const std::string& fileName, bool append) {
        std::lock_guard<std::mutex> lock(logMutex);

        // 关闭现有文件（如果不是stdout）
        if (!logFileName.empty() && logFileName != "stdout") {
            ofs.close();
        }

        // 打开新文件
        if (fileName != "stdout") {
            auto mode = std::ios::out;
            if (append) mode |= std::ios::app;

            ofs.open(fileName, mode);
            if (!ofs) {
                std::cerr << "Failed to open log file: " << fileName << std::endl;
            }
        }

        logFileName = fileName;
    }

    namespace {

        // C++14兼容的时间戳格式化函数
        inline std::string timestamp() {
            auto now = std::chrono::system_clock::now();
            std::time_t t = std::chrono::system_clock::to_time_t(now);

            std::tm tm_struct;

            // VS 安全解决方案：使用 localtime_s
#ifdef _WIN32
            errno_t err = localtime_s(&tm_struct, &t);
            if (err != 0) {
                return "19700101 00:00:00.000";
            }
#else
            if (localtime_r(&t, &tm_struct) == nullptr) {
                return "19700101 00:00:00.000";
            }
#endif

            // 提取毫秒部分
            auto since_epoch = now.time_since_epoch();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                since_epoch) % 1000;

            // 使用字符串流格式化
            std::ostringstream oss;
            oss << std::put_time(&tm_struct, "%Y%m%d %H:%M:%S") << "."
                << std::setfill('0') << std::setw(3) << ms.count();
            return oss.str();
        }

        // C++14兼容的字符串格式化函数
        template<typename... Args>
        inline std::string string_format(const char* fmt, Args... args) {
            // 计算所需空间
            int size = snprintf(nullptr, 0, fmt, args...) + 1;
            if (size <= 0) return "";

            // 动态分配缓冲区
            std::unique_ptr<char[]> buf(new char[size]);
            snprintf(buf.get(), size, fmt, args...);
            return std::string(buf.get(), buf.get() + size - 1);
        }

        // 截取文件名（去除路径）
        inline const char* base_name(const char* path) {
            const char* p = strrchr(path, '/');
#ifdef _WIN32
            const char* win_p = strrchr(path, '\\');
            if (win_p && (!p || win_p > p)) p = win_p;
#endif
            return p ? p + 1 : path;
        }

    } // namespace

    namespace internal {

        template <typename... Args>
        inline void logSys(const std::string& file, int line, bool to_abort,
            const char* fmt, Args... args) {
            std::string msg = string_format(fmt, args...);
            std::ostringstream oss;
            oss << timestamp()
                << " [" << std::setw(5) << std::setfill(' ') <<
#ifdef _WIN32
                _getpid()
#else
                getpid()
#endif 
                << "] "
                << (to_abort ? "[ SYSFA] " : "[SYSERR] ")
                << msg << ": " << strerror(errno) << " - "
                << base_name(file.c_str()) << ":" << line << "\n";

            // 线程安全的日志写入
            {
                std::lock_guard<std::mutex> lock(logMutex);
                if (logFileName.empty() || logFileName == "stdout") {
                    std::cout << oss.str();
                }
                else {
                    ofs << oss.str();
                    ofs.flush();
                }
            }

            if (to_abort) {
                abort();
            }
        }

        template <typename... Args>
        inline void logBase(const std::string& file, int line, LOG_LEVEL level,
            bool to_abort, const char* fmt, Args... args) {
            std::ostringstream oss;
            oss << timestamp()
                << " [" << std::setw(5) << std::setfill(' ') <<
#ifdef _WIN32
                _getpid()
#else
                getpid()
#endif 
                << "]"
                << " " << logLevelStr[static_cast<unsigned>(level)] << " "
                << string_format(fmt, args...)
                << " - " << base_name(file.c_str()) << ":" << line << "\n";

            // 线程安全的日志写入
            {
                std::lock_guard<std::mutex> lock(logMutex);
                if (logFileName.empty() || logFileName == "stdout") {
                    std::cout << oss.str();
                }
                else {
                    ofs << oss.str();
                    ofs.flush();
                }
            }

            if (to_abort) {
                abort();
            }
        }

    } // namespace internal
} // namespace logger

#endif // LOGGER_H