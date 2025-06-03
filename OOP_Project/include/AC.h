// AC.h
#pragma once
#include "Device.h"
#include "AC.h"

// �յ�ģʽö��
enum class ACMode {
    COOL,   // ����
    HEAT,   // ����
    FAN,    // �ͷ�
    OFF     // �ر�
};

// ����ö��
enum class FanSpeed {
    LOW,    // �ͷ�
    MEDIUM, // �з�
    HIGH,   // �߷�
    AUTO    // �Զ�
};

// ��ACģʽö��ת��Ϊ�ַ��������ڱ����ļ�����ʾ��
std::string acModeUserString(ACMode mode); // �����û�������ʾ
std::string acModeToString(ACMode mode); // �����ļ��洢 (ͨ��������)
ACMode stringToACMode(const std::string& s); // ���ڴ��ļ�����

// ������ö��ת��Ϊ�ַ��������ڱ����ļ�����ʾ��
std::string fanSpeedUserString(FanSpeed speed); // �����û�������ʾ
std::string fanSpeedToString(FanSpeed speed); // �����ļ��洢
FanSpeed stringToFanSpeed(const std::string& s); // ���ڴ��ļ�����


class AC : public Device {
private:
    ACMode mode;                // ��ǰģʽ
    double targetTemperature;   // Ŀ���¶� (���϶�)
    FanSpeed fanSpeed;          // ��ǰ����

public:
    AC(int id, const std::string& name, DeviceImportance importance, double powerConsumption,
        const std::string& location = "Ĭ�Ͽյ�λ��",
        ACMode mode = ACMode::OFF, double temp = 22.0, FanSpeed speed = FanSpeed::AUTO);
    ~AC() override;

    std::string toFileString() const override;
    void updateStatus() override;
    void displayInfo() const override;

    // �յ��ض�����
    void setMode(ACMode m);
    void setTargetTemperature(double temp);
    void setFanSpeed(FanSpeed speed);

    // Getters
    ACMode getMode() const;
    double getTargetTemperature() const;
    FanSpeed getFanSpeed() const;

    friend std::istream& operator>>(std::istream& is, AC& ac);
};