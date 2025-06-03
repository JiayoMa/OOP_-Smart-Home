
#pragma once
#include "DeviceFactory.h"
#include "Sensor.h" // ȷ�� Sensor ���Ѷ���

class SensorFactory : public DeviceFactory {
public:
    Device* createDevice() override;
    Device* createDeviceWithParams(const DeviceParams& params) override;
};