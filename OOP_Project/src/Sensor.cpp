// Sensor.cpp
#include "Sensor.h"
#include <iostream> 
#include <sstream>  
#include <cstdlib> // For rand() in updateStatus

Sensor::Sensor(int id, const std::string& name, DeviceImportance importance, double powerConsumption,
    const std::string& location, double temp, double hum, double co2)
    : Device(id, name, importance, powerConsumption, location),
    temperature(temp), humidity(hum), co2Concentration(co2) {
}

Sensor::~Sensor() {
    // std::cout << "环境传感器 " << name << " (ID: " << id << ") 已销毁。" << std::endl;
}

std::string Sensor::toFileString() const {
    std::stringstream ss;
    ss << "SENSOR," // 类型标识
        << Device::toFileString() // 调用基类获取通用部分
        << "," << getTemperature()
        << "," << getHumidity()
        << "," << getCO2Concentration();
    return ss.str(); //
}

void Sensor::updateStatus() {
    // 模拟状态更新：随机小幅变动传感器读数
    std::cout << "正在更新环境传感器 " << name << " (ID: " << id << ") 的状态:" << std::endl; //

    // 产生 -0.5 到 +0.5 之间的随机温度变化
    temperature += (rand() % 11 - 5) * 0.1;
    // 产生 -1.0 到 +1.0 之间的随机湿度变化
    humidity += (rand() % 21 - 10) * 0.1;
    // 产生 -0.001 到 +0.001 之间的随机CO2浓度变化 (即 -0.1% 到 +0.1%)
    co2Concentration += (rand() % 21 - 10) * 0.0001;

    // 确保湿度和CO2浓度在合理范围内
    if (humidity < 0) humidity = 0;         //
    if (humidity > 100) humidity = 100;     //
    if (co2Concentration < 0) co2Concentration = 0; //
    // CO2浓度上限可以根据实际情况设定，例如 1.0 (100%)
    if (co2Concentration > 1.0) co2Concentration = 1.0;

    std::cout << "  新温度: " << temperature << " °C" << std::endl; //
    std::cout << "  新湿度: " << humidity << " %" << std::endl; //
    std::cout << "  新CO2浓度: " << co2Concentration * 100 << " %" << std::endl; //
}

void Sensor::displayInfo() const {
    Device::displayInfo(); // 调用基类的 displayInfo
    std::cout << ", 类型: 环境传感器"
        << ", 温度: " << temperature << " °C"
        << ", 湿度: " << humidity << " %"
        << ", CO2浓度: " << co2Concentration * 100 << " %"; //
}

double Sensor::getTemperature() const { return temperature; } //
double Sensor::getHumidity() const { return humidity; }       //
double Sensor::getCO2Concentration() const { return co2Concentration; } //

void Sensor::setTemperature(double temp) { this->temperature = temp; } //
void Sensor::setHumidity(double hum) {
    if (hum < 0) this->humidity = 0;
    else if (hum > 100) this->humidity = 100;
    else this->humidity = hum;
}
void Sensor::setCO2Concentration(double co2) {
    if (co2 < 0) this->co2Concentration = 0;
    // else if (co2 > 1.0) this->co2Concentration = 1.0; // Optional upper bound
    else this->co2Concentration = co2;
}

std::istream& operator>>(std::istream& is, Sensor& sensor) {
    // 先调用基类的 operator>> 来读取通用设备信息
    is >> static_cast<Device&>(sensor);
    std::cout << "请输入温度 (摄氏度): "; //
    is >> sensor.temperature;
    std::cout << "请输入湿度 (%): "; //
    is >> sensor.humidity;
    std::cout << "请输入CO2浓度 (例如, 0.06 代表 6%): "; //
    is >> sensor.co2Concentration;
    is.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 清除换行符
    return is;
}