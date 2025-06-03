// ACFactory.h
#pragma once
#include "DeviceFactory.h"
#include "AC.h" // ȷ�� AC ���Ѷ���

class ACFactory : public DeviceFactory {
public:
    Device* createDevice() override;
    Device* createDeviceWithParams(const DeviceParams& params) override;
};