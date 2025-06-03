
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
    DEBUG, // ������Ϣ
    INFO,  // ��ͨ��Ϣ
    ALERT  // ������Ϣ
};

// ��־���Խӿ� (������)
class LoggerStrategy {
public:
    virtual ~LoggerStrategy() = default;
    virtual void log(const std::string& formatted_message, LogLevel level, int deviceId, std::thread::id threadId) = 0;
};

// ������־���ԣ����������̨
class ConsoleLogger : public LoggerStrategy {
public:
    void log(const std::string& formatted_message, LogLevel level, int deviceId, std::thread::id threadId) override;
};

// ������־���ԣ�������ļ�
class FileLogger : public LoggerStrategy {
private:
    std::ofstream logFile;
    std::mutex fileMutex; // �����̰߳�ȫ���ļ�д��
public:
    FileLogger(const std::string& filename);
    ~FileLogger() override;
    void log(const std::string& formatted_message, LogLevel level, int deviceId, std::thread::id threadId) override;
};

// SmartLogger �� (������)
class SmartLogger {
private:
    LoggerStrategy* strategy;
    LogLevel minimumLevel;
    std::mutex loggerMutex;   // �����̰߳�ȫ���ʲ��Ժ͹�����Դ

    std::string levelToString(LogLevel level);
    std::string getCurrentTimestamp();

public:
    SmartLogger(LoggerStrategy* strategy, LogLevel minLevel = LogLevel::INFO);
    ~SmartLogger();

    void setStrategy(LoggerStrategy* newStrategy);
    void setMinimumLevel(LogLevel level);

    void log(const std::string& message, LogLevel level, int deviceId = -1, std::thread::id threadId = std::this_thread::get_id());

    // �򻯵��õ���꺯��
    void DEBUG(const std::string& message, int deviceId = -1, std::thread::id threadId = std::this_thread::get_id());
    void INFO(const std::string& message, int deviceId = -1, std::thread::id threadId = std::this_thread::get_id());
    void ALERT(const std::string& message, int deviceId = -1, std::thread::id threadId = std::this_thread::get_id());
};