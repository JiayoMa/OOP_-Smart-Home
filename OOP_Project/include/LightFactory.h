// LightFactory.h
#pragma once
#include "DeviceFactory.h"
#include "Light.h" // 确保 Light 类已定义

class LightFactory : public DeviceFactory {
public:
    Device* createDevice() override;
    Device* createDeviceWithParams(const DeviceParams& params) override;
};