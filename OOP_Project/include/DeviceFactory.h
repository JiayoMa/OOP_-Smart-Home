// DeviceFactory.h
#pragma once

#include "Device.h"       // ���� Device �ඨ��
#include "DeviceParams.h" // ���� DeviceParams �ṹ�嶨��

// �����豸��������
class DeviceFactory {
public:
    virtual ~DeviceFactory() = default;
    virtual Device* createDevice() = 0;
    virtual Device* createDeviceWithParams(const DeviceParams& params) = 0;
};