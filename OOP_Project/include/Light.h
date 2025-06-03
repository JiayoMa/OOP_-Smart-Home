// Light.h
#pragma once
#include "Device.h"

class Light : public Device {
private:
    bool isOn;              // 开关状态
    int brightness;         // 亮度 (百分比 0-100)

public:
    Light(int id, const std::string& name, DeviceImportance importance, double powerConsumption,
        const std::string& location = "默认灯具位置",
        bool on = false, int bright = 50);
    ~Light() override;

    std::string toFileString() const override;
    void updateStatus() override;
    void displayInfo() const override;

    // 灯具特定方法
    void turnOn();
    void turnOff();
    void setBrightness(int level);

    // Getters
    bool getIsOn() const;
    int getBrightness() const;

    friend std::istream& operator>>(std::istream& is, Light& light);
};