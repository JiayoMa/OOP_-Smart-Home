// LightFactory.h
#pragma once
#include "DeviceFactory.h"
#include "Light.h" // ȷ�� Light ���Ѷ���

class LightFactory : public DeviceFactory {
public:
    Device* createDevice() override;
    Device* createDeviceWithParams(const DeviceParams& params) override;
};