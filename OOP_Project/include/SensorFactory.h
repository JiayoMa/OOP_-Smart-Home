
#pragma once
#include "DeviceFactory.h"
#include "Sensor.h" // 确保 Sensor 类已定义

class SensorFactory : public DeviceFactory {
public:
    Device* createDevice() override;
    Device* createDeviceWithParams(const DeviceParams& params) override;
};