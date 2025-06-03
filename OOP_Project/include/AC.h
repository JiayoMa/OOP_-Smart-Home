// AC.h
#pragma once
#include "Device.h"
#include "AC.h"

// 空调模式枚举
enum class ACMode {
    COOL,   // 制冷
    HEAT,   // 制热
    FAN,    // 送风
    OFF     // 关闭
};

// 风速枚举
enum class FanSpeed {
    LOW,    // 低风
    MEDIUM, // 中风
    HIGH,   // 高风
    AUTO    // 自动
};

// 将AC模式枚举转换为字符串（用于保存文件和显示）
std::string acModeUserString(ACMode mode); // 用于用户界面显示
std::string acModeToString(ACMode mode); // 用于文件存储 (通常是整数)
ACMode stringToACMode(const std::string& s); // 用于从文件加载

// 将风速枚举转换为字符串（用于保存文件和显示）
std::string fanSpeedUserString(FanSpeed speed); // 用于用户界面显示
std::string fanSpeedToString(FanSpeed speed); // 用于文件存储
FanSpeed stringToFanSpeed(const std::string& s); // 用于从文件加载


class AC : public Device {
private:
    ACMode mode;                // 当前模式
    double targetTemperature;   // 目标温度 (摄氏度)
    FanSpeed fanSpeed;          // 当前风速

public:
    AC(int id, const std::string& name, DeviceImportance importance, double powerConsumption,
        const std::string& location = "默认空调位置",
        ACMode mode = ACMode::OFF, double temp = 22.0, FanSpeed speed = FanSpeed::AUTO);
    ~AC() override;

    std::string toFileString() const override;
    void updateStatus() override;
    void displayInfo() const override;

    // 空调特定方法
    void setMode(ACMode m);
    void setTargetTemperature(double temp);
    void setFanSpeed(FanSpeed speed);

    // Getters
    ACMode getMode() const;
    double getTargetTemperature() const;
    FanSpeed getFanSpeed() const;

    friend std::istream& operator>>(std::istream& is, AC& ac);
};