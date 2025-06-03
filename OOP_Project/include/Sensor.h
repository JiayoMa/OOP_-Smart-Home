// Sensor.h
#pragma once
#include "Device.h"
#include "Sensor.h"
class Sensor : public Device {
private:
    double temperature;        // 温度 (摄氏度)
    double humidity;           // 湿度 (百分比)
    double co2Concentration;   // 二氧化碳浓度 (例如 0.04 表示 4%)

public:
    Sensor(int id, const std::string& name, DeviceImportance importance, double powerConsumption,
        const std::string& location = "默认传感器位置",
        double temp = 20.0, double hum = 50.0, double co2 = 0.04);

    ~Sensor() override;

    std::string toFileString() const override;
    void updateStatus() override;
    void displayInfo() const override;

    // Getters
    double getTemperature() const;
    double getHumidity() const;
    double getCO2Concentration() const;

    // Setters (用于环境模拟或手动调整)
    void setTemperature(double temp);
    void setHumidity(double hum);
    void setCO2Concentration(double co2);

    // 从输入流读取传感器特定数据（用于示例或测试）
    friend std::istream& operator>>(std::istream& is, Sensor& sensor);
};