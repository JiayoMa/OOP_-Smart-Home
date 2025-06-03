
#pragma once
#include <stdexcept>
#include <string>

// ���ܼҾ�ϵͳ�Զ����쳣����
class BaseSmartHomeException : public std::runtime_error {
public:
    explicit BaseSmartHomeException(const std::string& message)
        : std::runtime_error(message) {}
};

// ���Ҳ����ض��豸���͵Ĺ���ʱ���쳣
class FactoryNotFoundException : public BaseSmartHomeException {
public:
    explicit FactoryNotFoundException(const std::string& deviceType)
        : BaseSmartHomeException("δ�ҵ��豸���� '" + deviceType + "' �Ĺ���") {
    }
};

// �豸���������������Чʱ���쳣
class InvalidParameterException : public BaseSmartHomeException {
public:
    explicit InvalidParameterException(const std::string& message)
        : BaseSmartHomeException("��Ч����: " + message) {
    }
};

// ��ͨ��ID��������ʶ���Ҳ����豸ʱ���쳣
class DeviceNotFoundException : public BaseSmartHomeException {
public:
    explicit DeviceNotFoundException(int deviceId)
        : BaseSmartHomeException("δ�ҵ��豸 ID: " + std::to_string(deviceId)) {
    }
    explicit DeviceNotFoundException(const std::string& deviceName)
        : BaseSmartHomeException("δ�ҵ��豸����: " + deviceName) {
    }
};

// Ȩ�޲����쳣
class PermissionDeniedException : public BaseSmartHomeException {
public:
    explicit PermissionDeniedException(const std::string& message)
        : BaseSmartHomeException("Ȩ�޲���: " + message) {
    }
};