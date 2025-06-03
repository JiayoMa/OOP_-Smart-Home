// DeviceFactory.h
#pragma once

#include "Device.h"       // 包含 Device 类定义
#include "DeviceParams.h" // 包含 DeviceParams 结构体定义

// 抽象设备工厂基类
class DeviceFactory {
public:
    virtual ~DeviceFactory() = default;
    virtual Device* createDevice() = 0;
    virtual Device* createDeviceWithParams(const DeviceParams& params) = 0;
};