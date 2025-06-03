// Sensor.h
#pragma once
#include "Device.h"
#include "Sensor.h"
class Sensor : public Device {
private:
    double temperature;        // �¶� (���϶�)
    double humidity;           // ʪ�� (�ٷֱ�)
    double co2Concentration;   // ������̼Ũ�� (���� 0.04 ��ʾ 4%)

public:
    Sensor(int id, const std::string& name, DeviceImportance importance, double powerConsumption,
        const std::string& location = "Ĭ�ϴ�����λ��",
        double temp = 20.0, double hum = 50.0, double co2 = 0.04);

    ~Sensor() override;

    std::string toFileString() const override;
    void updateStatus() override;
    void displayInfo() const override;

    // Getters
    double getTemperature() const;
    double getHumidity() const;
    double getCO2Concentration() const;

    // Setters (���ڻ���ģ����ֶ�����)
    void setTemperature(double temp);
    void setHumidity(double hum);
    void setCO2Concentration(double co2);

    // ����������ȡ�������ض����ݣ�����ʾ������ԣ�
    friend std::istream& operator>>(std::istream& is, Sensor& sensor);
};