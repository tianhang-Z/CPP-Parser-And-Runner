#pragma once
#include <chrono>
#include <iostream>
#include <string>

class TimeGuard {
public:
    // 构造函数：记录开始时间并设置名称
    explicit TimeGuard(const std::string& name)
        : m_name(name),
        m_start(std::chrono::high_resolution_clock::now()) {
    }

    // 析构函数：计算并打印耗时
    ~TimeGuard() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_start).count();
        print_result(duration);
    }

    // 禁用拷贝和移动（单例行为）
    TimeGuard(const TimeGuard&) = delete;
    TimeGuard& operator=(const TimeGuard&) = delete;

private:
    // 打印结果（自动选择时间单位）
    void print_result(long long duration_ns) const {
        std::cout << "[TimeGuard] " << m_name << " - ";

        if (duration_ns < 1000) {
            std::cout << duration_ns << " ns\n";
        }
        else if (duration_ns < 1'000'000) {
            std::cout << (duration_ns / 1000.0) << " μs\n";
        }
        else if (duration_ns < 1'000'000'000) {
            std::cout << (duration_ns / 1'000'000.0) << " ms\n";
        }
        else {
            std::cout << (duration_ns / 1'000'000'000.0) << " s\n";
        }
    }

    std::string m_name;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
};