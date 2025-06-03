
#pragma once

#include <string>
#include "Device.h" // Ϊ�� DeviceImportance
#include "AC.h"     // Ϊ�� ACMode, FanSpeed

struct DeviceParams {
    int id = 0;
    std::string name = "Ĭ���豸";
    DeviceImportance importance = DeviceImportance::MEDIUM;
    double powerConsumption = 0.0;
    bool emergencyPowerOff = false;
    std::string location = "Ĭ��λ��";

    // Sensor�ض�����
    double temperature = 20.0;
    double humidity = 50.0;
    double co2Concentration = 0.04; 
    // Light�ض�����
    bool isOn = false;
    int brightness = 50;      
    // AC�ض�����
    ACMode acMode = ACMode::OFF;
    double targetTemperature = 22.0;
    FanSpeed fanSpeed = FanSpeed::AUTO;
};