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
#include <memory>

namespace Logger {
    // 日志级别枚举
    enum class LogLevel : unsigned {
        Trace = 0,
        Debug,
        Info,
        Warn,
        Error,
        Fatal
    };

    // 外部声明全局变量
    extern std::string logFileName;
    extern std::ofstream ofs;
    extern std::mutex logMutex;
    extern const char* logLevelStr[];
    extern LogLevel logLevel;

    inline void SetLogLevel(LogLevel level) {
        logLevel = level;
    }

    inline void SetLogFile(const std::string& fileName, bool append) {
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
        // 字符串格式化函数
        template<typename... Args>
        inline std::string FormatString(const char* fmt, Args... args) {
            // 计算所需空间
            int size = snprintf(nullptr, 0, fmt, args...) + 1;
            if (size <= 0) return "";

            // 动态分配缓冲区
            std::unique_ptr<char[]> buf(new char[size]);
            snprintf(buf.get(), size, fmt, args...);
            return std::string(buf.get(), buf.get() + size - 1);
        }

        // 截取文件名（去除路径）
        inline const char* GetBaseName(const char* path) {
            const char* p = strrchr(path, '/');
#ifdef _WIN32
            const char* winP = strrchr(path, '\\');
            if (winP && (!p || winP > p)) p = winP;
#endif
            return p ? p + 1 : path;
        }

    } // namespace



    namespace Internal {
        template <typename... Args>
        inline void LogSystem(const std::string& file, int line, bool toAbort,
            const char* fmt, Args... args) {
            std::string msg = FormatString(fmt, args...);
            std::ostringstream oss;
            oss << (toAbort ? "[ SYSFA] " : "[SYSERR] ")
                << msg << " - "
                << GetBaseName(file.c_str()) << ":" << line << "\n";

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

            if (toAbort) {
                std::abort();
            }
        }

        template <typename... Args>
        inline void LogBase(const std::string& file, int line, LogLevel level,
            bool toAbort, const char* fmt, Args... args) {
            std::ostringstream oss;
            oss << logLevelStr[static_cast<unsigned>(level)] << " "
                << FormatString(fmt, args...)
                << " - " << GetBaseName(file.c_str()) << ":" << line << "\n";

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

            if (toAbort) {
                std::abort();
            }
        }
    } // namespace Internal

} // namespace Logger

// 日志宏定义
#define LOG_TRACE(fmt, ...)  \
    if (static_cast<unsigned>(Logger::logLevel) <= static_cast<unsigned>(Logger::LogLevel::Trace)) \
    Logger::Internal::LogBase(__FILE__, __LINE__, \
        Logger::LogLevel::Trace, false, fmt, ##__VA_ARGS__)

#define LOG_DEBUG(fmt, ...)  \
    if (static_cast<unsigned>(Logger::logLevel) <= static_cast<unsigned>(Logger::LogLevel::Debug)) \
    Logger::Internal::LogBase(__FILE__, __LINE__, \
        Logger::LogLevel::Debug, false, fmt, ##__VA_ARGS__)

#define LOG_INFO(fmt, ...)   \
    if (static_cast<unsigned>(Logger::logLevel) <= static_cast<unsigned>(Logger::LogLevel::Info)) \
    Logger::Internal::LogBase(__FILE__, __LINE__, \
        Logger::LogLevel::Info, false, fmt, ##__VA_ARGS__)

#define LOG_WARN(fmt, ...)   \
    if (static_cast<unsigned>(Logger::logLevel) <= static_cast<unsigned>(Logger::LogLevel::Warn)) \
    Logger::Internal::LogBase(__FILE__, __LINE__, \
        Logger::LogLevel::Warn, false, fmt, ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...)  \
    if (static_cast<unsigned>(Logger::logLevel) <= static_cast<unsigned>(Logger::LogLevel::Error)) \
    Logger::Internal::LogBase(__FILE__, __LINE__, \
        Logger::LogLevel::Error, false, fmt, ##__VA_ARGS__)

#define LOG_FATAL(fmt, ...)  \
    Logger::Internal::LogBase(__FILE__, __LINE__, \
        Logger::LogLevel::Fatal, true, fmt, ##__VA_ARGS__)

#define LOG_SYS_ERROR(fmt, ...) \
    Logger::Internal::LogSystem(__FILE__, __LINE__, false, fmt, ##__VA_ARGS__)

#define LOG_SYS_FATAL(fmt, ...) \
    Logger::Internal::LogSystem(__FILE__, __LINE__, true, fmt, ##__VA_ARGS__)

#endif // LOGGER_H