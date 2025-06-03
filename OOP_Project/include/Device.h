// Device.h
#pragma once
#include "Device.h"

#include <string>
#include <iostream>
#include <vector> 
#include <sstream> // 用于 toFileString

// 设备类型枚举，用于工厂和容器
enum class DeviceType {
    SENSOR,
    LIGHT,
    AC
};

// 设备重要程度枚举
enum class DeviceImportance {
    LOW,     // 低
    MEDIUM,  // 中
    HIGH,    // 高
    CRITICAL // 危急
};

// 将重要程度枚举转换为字符串（用于保存文件和显示）
std::string importanceToString(DeviceImportance imp);
// 将字符串转换为重要程度枚举（用于从文件加载）
DeviceImportance stringToImportance(const std::string& s);


class Device {
protected:
    int id;
    std::string name;
    DeviceImportance importance;
    double powerConsumption;    
    bool emergencyPowerOff;    
    std::string location;     
public:
    Device(int id, const std::string& name, DeviceImportance importance,
        double powerConsumption, const std::string& location = "默认位置");
    virtual ~Device();
    virtual std::string toFileString() const;
    virtual void updateStatus() = 0;
    // Getters
    int getId() const;
    std::string getName() const;
    DeviceImportance getImportance() const;
    double getPowerConsumption() const;
    bool isEmergencyPowerOff() const;
    std::string getLocation() const;
    void setId(int id); 
    void setName(const std::string& name);
    void setImportance(DeviceImportance importance);
    void setPowerConsumption(double consumption);
    void setEmergencyPowerOff(bool status);
    void setLocation(const std::string& newLocation);
    virtual void displayInfo() const;
    friend std::ostream& operator<<(std::ostream& os, const Device& device);
    friend std::istream& operator>>(std::istream& is, Device& device);
};