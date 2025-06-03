
#include "SmartLogger.h"
std::string SmartLogger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now(); //
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;

    std::tm timeinfo_tm;
#ifdef _WIN32
    gmtime_s(&timeinfo_tm, &in_time_t); // Windows �ض�
#else
    gmtime_r(&in_time_t, &timeinfo_tm); // POSIX �ض�
#endif
    ss << std::put_time(&timeinfo_tm, "%Y-%m-%d %H:%M:%S UTC");
    return ss.str(); //
}

std::string SmartLogger::levelToString(LogLevel level) {
    switch (level) {
    case LogLevel::DEBUG: return "����"; //
    case LogLevel::INFO:  return "��Ϣ";  //
    case LogLevel::ALERT: return "����"; //
    default: return "δ֪";    //
    }
}

// --- ConsoleLogger ʵ�� ---
void ConsoleLogger::log(const std::string& formatted_message, LogLevel level, int deviceId, std::thread::id threadId) {
    std::cout << formatted_message << std::endl; //
}

// --- FileLogger ʵ�� ---
FileLogger::FileLogger(const std::string& filename) {
    logFile.open(filename, std::ios::app); //
    if (!logFile.is_open()) {
        std::cerr << "���ش���: �޷�����־�ļ� '" << filename << "' ����д�롣" << std::endl;
    }
}

FileLogger::~FileLogger() {
    if (logFile.is_open()) {
        logFile.close(); //
    }
}

void FileLogger::log(const std::string& formatted_message, LogLevel level, int deviceId, std::thread::id threadId) {
    std::lock_guard<std::mutex> lock(fileMutex); // ȷ���̰߳�ȫд��
    if (logFile.is_open()) {
        logFile << formatted_message << std::endl; //
    }
}

// --- SmartLogger ʵ�� ---
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
        // �������ǳ����أ�ֱ�������cerr
        std::cerr << "���ش���: SmartLoggerû��������־���ԣ�" << std::endl;
        return;
    }

    std::stringstream formattedMessage;
    formattedMessage << "[" << getCurrentTimestamp() << "] "; //
    formattedMessage << "[" << levelToString(level) << "] ";   //

    std::stringstream tid_ss;
    tid_ss << threadId;
    formattedMessage << "[�߳�ID:" << tid_ss.str() << "] ";

    if (deviceId != -1) {
        formattedMessage << "[�豸ID:" << deviceId << "] ";
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