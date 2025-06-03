
#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip> 
#include <sstream> 
#include <thread>  
#include <mutex>   

enum class LogLevel {
    DEBUG, // 调试信息
    INFO,  // 普通信息
    ALERT  // 警报信息
};

// 日志策略接口 (抽象类)
class LoggerStrategy {
public:
    virtual ~LoggerStrategy() = default;
    virtual void log(const std::string& formatted_message, LogLevel level, int deviceId, std::thread::id threadId) = 0;
};

// 具体日志策略：输出到控制台
class ConsoleLogger : public LoggerStrategy {
public:
    void log(const std::string& formatted_message, LogLevel level, int deviceId, std::thread::id threadId) override;
};

// 具体日志策略：输出到文件
class FileLogger : public LoggerStrategy {
private:
    std::ofstream logFile;
    std::mutex fileMutex; // 用于线程安全的文件写入
public:
    FileLogger(const std::string& filename);
    ~FileLogger() override;
    void log(const std::string& formatted_message, LogLevel level, int deviceId, std::thread::id threadId) override;
};

// SmartLogger 类 (上下文)
class SmartLogger {
private:
    LoggerStrategy* strategy;
    LogLevel minimumLevel;
    std::mutex loggerMutex;   // 用于线程安全访问策略和共享资源

    std::string levelToString(LogLevel level);
    std::string getCurrentTimestamp();

public:
    SmartLogger(LoggerStrategy* strategy, LogLevel minLevel = LogLevel::INFO);
    ~SmartLogger();

    void setStrategy(LoggerStrategy* newStrategy);
    void setMinimumLevel(LogLevel level);

    void log(const std::string& message, LogLevel level, int deviceId = -1, std::thread::id threadId = std::this_thread::get_id());

    // 简化调用的类宏函数
    void DEBUG(const std::string& message, int deviceId = -1, std::thread::id threadId = std::this_thread::get_id());
    void INFO(const std::string& message, int deviceId = -1, std::thread::id threadId = std::this_thread::get_id());
    void ALERT(const std::string& message, int deviceId = -1, std::thread::id threadId = std::this_thread::get_id());
};