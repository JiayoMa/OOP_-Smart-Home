// ACFactory.h
#pragma once
#include "DeviceFactory.h"
#include "AC.h" // 确保 AC 类已定义

class ACFactory : public DeviceFactory {
public:
    Device* createDevice() override;
    Device* createDeviceWithParams(const DeviceParams& params) override;
};