
#include "SmartLogger.h"
std::string SmartLogger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now(); //
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;

    std::tm timeinfo_tm;
#ifdef _WIN32
    gmtime_s(&timeinfo_tm, &in_time_t); // Windows 特定
#else
    gmtime_r(&in_time_t, &timeinfo_tm); // POSIX 特定
#endif
    ss << std::put_time(&timeinfo_tm, "%Y-%m-%d %H:%M:%S UTC");
    return ss.str(); //
}

std::string SmartLogger::levelToString(LogLevel level) {
    switch (level) {
    case LogLevel::DEBUG: return "调试"; //
    case LogLevel::INFO:  return "信息";  //
    case LogLevel::ALERT: return "警报"; //
    default: return "未知";    //
    }
}

// --- ConsoleLogger 实现 ---
void ConsoleLogger::log(const std::string& formatted_message, LogLevel level, int deviceId, std::thread::id threadId) {
    std::cout << formatted_message << std::endl; //
}

// --- FileLogger 实现 ---
FileLogger::FileLogger(const std::string& filename) {
    logFile.open(filename, std::ios::app); //
    if (!logFile.is_open()) {
        std::cerr << "严重错误: 无法打开日志文件 '" << filename << "' 进行写入。" << std::endl;
    }
}

FileLogger::~FileLogger() {
    if (logFile.is_open()) {
        logFile.close(); //
    }
}

void FileLogger::log(const std::string& formatted_message, LogLevel level, int deviceId, std::thread::id threadId) {
    std::lock_guard<std::mutex> lock(fileMutex); // 确保线程安全写入
    if (logFile.is_open()) {
        logFile << formatted_message << std::endl; //
    }
}

// --- SmartLogger 实现 ---
SmartLogger::SmartLogger(LoggerStrategy* initialStrategy, LogLevel minLevel)
    : strategy(initialStrategy), minimumLevel(minLevel) { //
}

SmartLogger::~SmartLogger() {
    delete strategy; //
}

void SmartLogger::setStrategy(LoggerStrategy* newStrategy) {
    std::lock_guard<std::mutex> lock(loggerMutex);
    delete strategy; //
    strategy = newStrategy;
}

void SmartLogger::setMinimumLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(loggerMutex);
    minimumLevel = level; //
}

void SmartLogger::log(const std::string& message, LogLevel level, int deviceId, std::thread::id threadId) {
    std::lock_guard<std::mutex> lock(loggerMutex);
    if (level < minimumLevel) { //
        return;
    }
    if (!strategy) {
        // 这个错误非常严重，直接输出到cerr
        std::cerr << "严重错误: SmartLogger没有设置日志策略！" << std::endl;
        return;
    }

    std::stringstream formattedMessage;
    formattedMessage << "[" << getCurrentTimestamp() << "] "; //
    formattedMessage << "[" << levelToString(level) << "] ";   //

    std::stringstream tid_ss;
    tid_ss << threadId;
    formattedMessage << "[线程ID:" << tid_ss.str() << "] ";

    if (deviceId != -1) {
        formattedMessage << "[设备ID:" << deviceId << "] ";
    }
    formattedMessage << message;

    strategy->log(formattedMessage.str(), level, deviceId, threadId);
}

void SmartLogger::DEBUG(const std::string& message, int deviceId, std::thread::id threadId) {
    log(message, LogLevel::DEBUG, deviceId, threadId); //
}
void SmartLogger::INFO(const std::string& message, int deviceId, std::thread::id threadId) {
    log(message, LogLevel::INFO, deviceId, threadId); //
}
void SmartLogger::ALERT(const std::string& message, int deviceId, std::thread::id threadId) {
    log(message, LogLevel::ALERT, deviceId, threadId); //
}