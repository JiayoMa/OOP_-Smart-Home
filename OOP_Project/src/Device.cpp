// Device.cpp
#include "Device.h"
#include "DeviceContainer.h"
// DeviceImportance 枚举转换函数实现
std::string importanceToString(DeviceImportance imp) { //
    return std::to_string(static_cast<int>(imp)); //
}

DeviceImportance stringToImportance(const std::string& s) { //
    try {
        int val = std::stoi(s); //
        if (val >= static_cast<int>(DeviceImportance::LOW) && val <= static_cast<int>(DeviceImportance::CRITICAL)) { //
            return static_cast<DeviceImportance>(val); //
        }
        // 注意：这里直接输出到cerr可能不是最佳实践，应该通过日志系统或抛出异常
        std::cerr << "警告: 重要性值 '" << s << "' 超出预期范围。默认为 中。" << std::endl; //
    }
    catch (const std::exception& e) {
        std::cerr << "警告: 解析重要性值 '" << s << "' 失败: " << e.what() << "。默认为 中。" << std::endl; //
    }
    return DeviceImportance::MEDIUM; // 默认返回中等重要性
}


Device::Device(int id, const std::string& name, DeviceImportance importance,
    double powerConsumption, const std::string& location)
    : id(id), name(name), importance(importance),
    powerConsumption(powerConsumption), emergencyPowerOff(false), location(location) {
}

Device::~Device() {
    // std::cout << "设备 " << name << " (ID: " << id << ") 已销毁。" << std::endl;
}

std::string Device::toFileString() const {
    std::stringstream ss;
    // 类型字符串 (Sensor, Light, AC) 由 DeviceContainer::saveDevicesToFile 添加
    // 这里只处理通用部分，不包含类型字符串和前导逗号
    ss << getId() << ","
        << getName() << ","
        << importanceToString(getImportance()) << ","
        << getPowerConsumption() << ","
        << (isEmergencyPowerOff() ? "1" : "0") << ","
        << getLocation();
    return ss.str(); //
}

int Device::getId() const { return id; } //
std::string Device::getName() const { return name; } //
DeviceImportance Device::getImportance() const { return importance; } //
double Device::getPowerConsumption() const { return powerConsumption; } //
bool Device::isEmergencyPowerOff() const { return emergencyPowerOff; } //
std::string Device::getLocation() const { return location; }


void Device::setId(int id) { this->id = id; } //
void Device::setName(const std::string& name) { this->name = name; } //
void Device::setImportance(DeviceImportance importance) { this->importance = importance; } //
void Device::setPowerConsumption(double consumption) { this->powerConsumption = consumption; } //
void Device::setEmergencyPowerOff(bool status) { this->emergencyPowerOff = status; } //
void Device::setLocation(const std::string& newLocation) { this->location = newLocation; }


void Device::displayInfo() const {
    std::cout << "设备ID: " << id << ", 名称: " << name
        << ", 位置: " << location
        << ", 重要程度: "; //
    switch (importance) { //
    case DeviceImportance::LOW: std::cout << "低"; break;
    case DeviceImportance::MEDIUM: std::cout << "中"; break;
    case DeviceImportance::HIGH: std::cout << "高"; break;
    case DeviceImportance::CRITICAL: std::cout << "危急"; break;
    default: std::cout << "未知"; break;
    }
    std::cout << ", 功耗: " << powerConsumption << " 瓦" // 假设单位是瓦
        << ", 紧急断电状态: " << (emergencyPowerOff ? "已启用" : "未启用"); //
    // 子类应该调用这个方法然后添加自己的特定信息
}

std::ostream& operator<<(std::ostream& os, const Device& device) {
    device.displayInfo(); // 调用 displayInfo 方法来输出
    return os;
}

std::istream& operator>>(std::istream& is, Device& device) {
    // 这个操作符主要用于示例或非常简单的输入，实际项目中通常会有更复杂的UI或参数解析
    std::cout << "请输入设备ID: "; //
    is >> device.id;
    is.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 清除换行符
    std::cout << "请输入设备名称: "; //
    std::getline(is, device.name);
    std::cout << "请输入设备位置: "; //
    std::getline(is, device.location);
    // 重要程度和功耗等通常通过更结构化的方式设置
    // 例如，通过 DeviceParams 和工厂，或者特定的 setter UI
    std::cout << "请输入功耗 (瓦): "; //
    is >> device.powerConsumption;
    is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return is;
}