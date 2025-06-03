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
    // std::cout << "���������� " << name << " (ID: " << id << ") �����١�" << std::endl;
}

std::string Sensor::toFileString() const {
    std::stringstream ss;
    ss << "SENSOR," // ���ͱ�ʶ
        << Device::toFileString() // ���û����ȡͨ�ò���
        << "," << getTemperature()
        << "," << getHumidity()
        << "," << getCO2Concentration();
    return ss.str(); //
}

void Sensor::updateStatus() {
    // ģ��״̬���£����С���䶯����������
    std::cout << "���ڸ��»��������� " << name << " (ID: " << id << ") ��״̬:" << std::endl; //

    // ���� -0.5 �� +0.5 ֮�������¶ȱ仯
    temperature += (rand() % 11 - 5) * 0.1;
    // ���� -1.0 �� +1.0 ֮������ʪ�ȱ仯
    humidity += (rand() % 21 - 10) * 0.1;
    // ���� -0.001 �� +0.001 ֮������CO2Ũ�ȱ仯 (�� -0.1% �� +0.1%)
    co2Concentration += (rand() % 21 - 10) * 0.0001;

    // ȷ��ʪ�Ⱥ�CO2Ũ���ں���Χ��
    if (humidity < 0) humidity = 0;         //
    if (humidity > 100) humidity = 100;     //
    if (co2Concentration < 0) co2Concentration = 0; //
    // CO2Ũ�����޿��Ը���ʵ������趨������ 1.0 (100%)
    if (co2Concentration > 1.0) co2Concentration = 1.0;

    std::cout << "  ���¶�: " << temperature << " ��C" << std::endl; //
    std::cout << "  ��ʪ��: " << humidity << " %" << std::endl; //
    std::cout << "  ��CO2Ũ��: " << co2Concentration * 100 << " %" << std::endl; //
}

void Sensor::displayInfo() const {
    Device::displayInfo(); // ���û���� displayInfo
    std::cout << ", ����: ����������"
        << ", �¶�: " << temperature << " ��C"
        << ", ʪ��: " << humidity << " %"
        << ", CO2Ũ��: " << co2Concentration * 100 << " %"; //
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
    // �ȵ��û���� operator>> ����ȡͨ���豸��Ϣ
    is >> static_cast<Device&>(sensor);
    std::cout << "�������¶� (���϶�): "; //
    is >> sensor.temperature;
    std::cout << "������ʪ�� (%): "; //
    is >> sensor.humidity;
    std::cout << "������CO2Ũ�� (����, 0.06 ���� 6%): "; //
    is >> sensor.co2Concentration;
    is.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // ������з�
    return is;
}