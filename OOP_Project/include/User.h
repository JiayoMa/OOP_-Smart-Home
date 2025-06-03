// User.h
#pragma once
#include <string>
#include <vector>
#include "User.h"
#include "Device.h"      // 包含 Device 类定义
#include "SmartLogger.h" // 包含 SmartLogger 定义

// 用户角色枚举
enum class UserRole {
    USER,  // 普通用户
    ADMIN  // 管理员
};

// 将用户角色枚举转换为字符串（用于保存文件和显示）
inline std::string roleToString(UserRole role) {
    switch (role) {
    case UserRole::ADMIN: return "ADMIN";
    case UserRole::USER:  return "USER";
    default: return "USER"; // 默认或未知角色
    }
}

// 将字符串转换为用户角色枚举（用于从文件加载）
inline UserRole stringToRole(const std::string& roleStr) {
    if (roleStr == "ADMIN") return UserRole::ADMIN;
    return UserRole::USER; // 默认为普通用户
}

class User {
private:
    std::string username;
    std::string password; // 注意：实际应用中应存储密码的哈希值
    UserRole role;
    std::vector<Device*>& devices; // 引用设备列表 (来自DeviceContainer)
    SmartLogger& logger;           // 引用日志记录器

public:
    User(const std::string& name, const std::string& pwd, UserRole r,
        std::vector<Device*>& deviceList, SmartLogger& loggerRef);

    std::string getUsername() const;
    std::string getPassword() const; // 在实际应用中，不应直接暴露密码
    UserRole getRole() const;
    const std::vector<Device*>& getDevices() const; // 如果用户需要直接访问设备列表

    // 智能场景模拟方法
    void runTemperatureHumidityRule(double tempThreshold = 30.0,double humidityThreshold = 70.0);
    void runFireEmergencyRule(double co2FireThreshold = 0.06); // 6% CO2 浓度
};