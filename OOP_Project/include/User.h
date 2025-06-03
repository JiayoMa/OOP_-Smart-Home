// User.h
#pragma once
#include <string>
#include <vector>
#include "User.h"
#include "Device.h"      // ���� Device �ඨ��
#include "SmartLogger.h" // ���� SmartLogger ����

// �û���ɫö��
enum class UserRole {
    USER,  // ��ͨ�û�
    ADMIN  // ����Ա
};

// ���û���ɫö��ת��Ϊ�ַ��������ڱ����ļ�����ʾ��
inline std::string roleToString(UserRole role) {
    switch (role) {
    case UserRole::ADMIN: return "ADMIN";
    case UserRole::USER:  return "USER";
    default: return "USER"; // Ĭ�ϻ�δ֪��ɫ
    }
}

// ���ַ���ת��Ϊ�û���ɫö�٣����ڴ��ļ����أ�
inline UserRole stringToRole(const std::string& roleStr) {
    if (roleStr == "ADMIN") return UserRole::ADMIN;
    return UserRole::USER; // Ĭ��Ϊ��ͨ�û�
}

class User {
private:
    std::string username;
    std::string password; // ע�⣺ʵ��Ӧ����Ӧ�洢����Ĺ�ϣֵ
    UserRole role;
    std::vector<Device*>& devices; // �����豸�б� (����DeviceContainer)
    SmartLogger& logger;           // ������־��¼��

public:
    User(const std::string& name, const std::string& pwd, UserRole r,
        std::vector<Device*>& deviceList, SmartLogger& loggerRef);

    std::string getUsername() const;
    std::string getPassword() const; // ��ʵ��Ӧ���У���Ӧֱ�ӱ�¶����
    UserRole getRole() const;
    const std::vector<Device*>& getDevices() const; // ����û���Ҫֱ�ӷ����豸�б�

    // ���ܳ���ģ�ⷽ��
    void runTemperatureHumidityRule(double tempThreshold = 30.0,double humidityThreshold = 70.0);
    void runFireEmergencyRule(double co2FireThreshold = 0.06); // 6% CO2 Ũ��
};