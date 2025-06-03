
#pragma once
#include <stdexcept>
#include <string>

// 智能家居系统自定义异常基类
class BaseSmartHomeException : public std::runtime_error {
public:
    explicit BaseSmartHomeException(const std::string& message)
        : std::runtime_error(message) {}
};

// 当找不到特定设备类型的工厂时的异常
class FactoryNotFoundException : public BaseSmartHomeException {
public:
    explicit FactoryNotFoundException(const std::string& deviceType)
        : BaseSmartHomeException("未找到设备类型 '" + deviceType + "' 的工厂") {
    }
};

// 设备创建或操作参数无效时的异常
class InvalidParameterException : public BaseSmartHomeException {
public:
    explicit InvalidParameterException(const std::string& message)
        : BaseSmartHomeException("无效参数: " + message) {
    }
};

// 当通过ID或其他标识符找不到设备时的异常
class DeviceNotFoundException : public BaseSmartHomeException {
public:
    explicit DeviceNotFoundException(int deviceId)
        : BaseSmartHomeException("未找到设备 ID: " + std::to_string(deviceId)) {
    }
    explicit DeviceNotFoundException(const std::string& deviceName)
        : BaseSmartHomeException("未找到设备名称: " + deviceName) {
    }
};

// 权限不足异常
class PermissionDeniedException : public BaseSmartHomeException {
public:
    explicit PermissionDeniedException(const std::string& message)
        : BaseSmartHomeException("权限不足: " + message) {
    }
};