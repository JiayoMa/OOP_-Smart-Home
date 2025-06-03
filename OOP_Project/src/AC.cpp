// AC.cpp
#include "AC.h"
#include <iostream>
#include <sstream>
#include <stdexcept> // For std::stoi, std::stod exceptions

// ACMode 转换函数实现
std::string acModeUserString(ACMode mode) {
    switch (mode) {
    case ACMode::COOL: return "制冷";
    case ACMode::HEAT: return "制热";
    case ACMode::FAN:  return "送风";
    case ACMode::OFF:  return "关闭";
    default: return "未知模式";
    }
}
std::string acModeToString(ACMode mode) { // 用于文件存储
    return std::to_string(static_cast<int>(mode));
}
ACMode stringToACMode(const std::string& s) {
    try {
        int val = std::stoi(s);
        // 根据实际枚举值范围进行检查，假设 0-3 对应 COOL,HEAT,FAN,OFF
        if (val >= static_cast<int>(ACMode::COOL) && val <= static_cast<int>(ACMode::OFF)) {
            return static_cast<ACMode>(val);
        }
        // 日志应由调用者处理或通过全局日志记录
        std::cerr << "警告: 未知的空调模式值 '" << s << "'。默认为关闭。" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "警告: 解析空调模式值 '" << s << "' 失败: " << e.what() << "。默认为关闭。" << std::endl;
    }
    return ACMode::OFF;
}

// FanSpeed 转换函数实现
std::string fanSpeedUserString(FanSpeed speed) {
    switch (speed) {
    case FanSpeed::LOW:    return "低风";
    case FanSpeed::MEDIUM: return "中风";
    case FanSpeed::HIGH:   return "高风";
    case FanSpeed::AUTO:   return "自动";
    default: return "未知风速";
    }
}
std::string fanSpeedToString(FanSpeed speed) { // 用于文件存储
    return std::to_string(static_cast<int>(speed));
}
FanSpeed stringToFanSpeed(const std::string& s) {
    try {
        int val = std::stoi(s);
        // 假设 0-3 对应 LOW,MEDIUM,HIGH,AUTO
        if (val >= static_cast<int>(FanSpeed::LOW) && val <= static_cast<int>(FanSpeed::AUTO)) {
            return static_cast<FanSpeed>(val);
        }
        std::cerr << "警告: 未知的风扇速度值 '" << s << "'。默认为自动。" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "警告: 解析风扇速度值 '" << s << "' 失败: " << e.what() << "。默认为自动。" << std::endl;
    }
    return FanSpeed::AUTO;
}


AC::AC(int id, const std::string& name, DeviceImportance importance, double powerConsumption,
    const std::string& location, ACMode mode, double temp, FanSpeed speed)
    : Device(id, name, importance, powerConsumption, location),
    mode(mode), targetTemperature(temp), fanSpeed(speed) {
}

AC::~AC() {
    // std::cout << "空调 " << name << " (ID: " << id << ") 已销毁。" << std::endl;
}

std::string AC::toFileString() const {
    std::stringstream ss;
    ss << "AC," // 类型标识
        << Device::toFileString() // 调用基类获取通用部分
        << "," << acModeToString(getMode()) // 使用整数转换函数
        << "," << getTargetTemperature()
        << "," << fanSpeedToString(getFanSpeed()); // 使用整数转换函数
    return ss.str();
}

void AC::updateStatus() {
    std::cout << "正在更新空调 " << name << " (ID: " << id << ") 的状态:" << std::endl; //
    std::cout << "  当前模式: " << acModeUserString(mode) << std::endl; //
    if (mode != ACMode::OFF) {
        std::cout << "  目标温度: " << targetTemperature << " °C" << std::endl; //
        std::cout << "  风速: " << fanSpeedUserString(fanSpeed) << std::endl; //
    }
}

void AC::displayInfo() const {
    Device::displayInfo(); // 调用基类的 displayInfo
    std::cout << ", 类型: 空调"
        << ", 模式: " << acModeUserString(mode); //
    if (mode != ACMode::OFF) {
        std::cout << ", 目标温度: " << targetTemperature << " °C"
            << ", 风速: " << fanSpeedUserString(fanSpeed); //
    }
}

void AC::setMode(ACMode m) {
    mode = m;
    std::cout << "空调 " << name << " (ID: " << id << ") 模式已设置为 " << acModeUserString(mode) << "。" << std::endl; //
    if (mode == ACMode::OFF) {
        // 可以选择在关闭时重置目标温度或保持
        // targetTemperature = 25.0; // 例如，默认关闭时的待机温度
    }
}

void AC::setTargetTemperature(double temp) {
    if (mode == ACMode::OFF) {
        std::cout << "空调 " << name << " (ID: " << id << ") 当前为关闭状态，无法设置目标温度。" << std::endl; //
        return;
    }
    // 可以添加温度范围检查
    if (temp < 16.0 || temp > 30.0) { // 示例范围
        std::cout << "空调 " << name << " (ID: " << id << ") 无效的目标温度：" << temp << "°C。温度应在16-30°C之间。" << std::endl; //
        return;
    }
    targetTemperature = temp;
    std::cout << "空调 " << name << " (ID: " << id << ") 目标温度已设置为 " << targetTemperature << " °C。" << std::endl; //
}

void AC::setFanSpeed(FanSpeed speed) {
    if (mode == ACMode::OFF) {
        std::cout << "空调 " << name << " (ID: " << id << ") 当前为关闭状态，无法设置风速。" << std::endl; //
        return;
    }
    fanSpeed = speed;
    std::cout << "空调 " << name << " (ID: " << id << ") 风速已设置为 " << fanSpeedUserString(fanSpeed) << "。" << std::endl; //
}

ACMode AC::getMode() const { return mode; } //
double AC::getTargetTemperature() const { return targetTemperature; } //
FanSpeed AC::getFanSpeed() const { return fanSpeed; } //

std::istream& operator>>(std::istream& is, AC& ac) {
    is >> static_cast<Device&>(ac); // 读取通用设备信息
    int modeChoice, fanChoice;
    std::cout << "请选择空调模式 (0:制冷, 1:制热, 2:送风, 3:关闭): "; //
    is >> modeChoice;
    // 确保 modeChoice 在 ACMode 枚举的有效范围内
    if (modeChoice >= static_cast<int>(ACMode::COOL) && modeChoice <= static_cast<int>(ACMode::OFF)) {
        ac.mode = static_cast<ACMode>(modeChoice);
    }
    else {
        std::cout << "无效的模式选择，默认为关闭。" << std::endl;
        ac.mode = ACMode::OFF;
    }


    if (ac.mode != ACMode::OFF) {
        std::cout << "请输入目标温度 (摄氏度): "; //
        is >> ac.targetTemperature;
        std::cout << "请选择风速 (0:低风, 1:中风, 2:高风, 3:自动): "; //
        is >> fanChoice;
        if (fanChoice >= static_cast<int>(FanSpeed::LOW) && fanChoice <= static_cast<int>(FanSpeed::AUTO)) {
            ac.fanSpeed = static_cast<FanSpeed>(fanChoice);
        }
        else {
            std::cout << "无效的风速选择，默认为自动。" << std::endl;
            ac.fanSpeed = FanSpeed::AUTO;
        }
    }
    is.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 清除换行符
    return is;
}