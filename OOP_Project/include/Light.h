// Light.h
#pragma once
#include "Device.h"

class Light : public Device {
private:
    bool isOn;              // ����״̬
    int brightness;         // ���� (�ٷֱ� 0-100)

public:
    Light(int id, const std::string& name, DeviceImportance importance, double powerConsumption,
        const std::string& location = "Ĭ�ϵƾ�λ��",
        bool on = false, int bright = 50);
    ~Light() override;

    std::string toFileString() const override;
    void updateStatus() override;
    void displayInfo() const override;

    // �ƾ��ض�����
    void turnOn();
    void turnOff();
    void setBrightness(int level);

    // Getters
    bool getIsOn() const;
    int getBrightness() const;

    friend std::istream& operator>>(std::istream& is, Light& light);
};