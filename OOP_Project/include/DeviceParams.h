
#pragma once

#include <string>
#include "Device.h" // 为了 DeviceImportance
#include "AC.h"     // 为了 ACMode, FanSpeed

struct DeviceParams {
    int id = 0;
    std::string name = "默认设备";
    DeviceImportance importance = DeviceImportance::MEDIUM;
    double powerConsumption = 0.0;
    bool emergencyPowerOff = false;
    std::string location = "默认位置";

    // Sensor特定参数
    double temperature = 20.0;
    double humidity = 50.0;
    double co2Concentration = 0.04; 
    // Light特定参数
    bool isOn = false;
    int brightness = 50;      
    // AC特定参数
    ACMode acMode = ACMode::OFF;
    double targetTemperature = 22.0;
    FanSpeed fanSpeed = FanSpeed::AUTO;
};