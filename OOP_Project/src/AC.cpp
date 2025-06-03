// AC.cpp
#include "AC.h"
#include <iostream>
#include <sstream>
#include <stdexcept> // For std::stoi, std::stod exceptions

// ACMode ת������ʵ��
std::string acModeUserString(ACMode mode) {
    switch (mode) {
    case ACMode::COOL: return "����";
    case ACMode::HEAT: return "����";
    case ACMode::FAN:  return "�ͷ�";
    case ACMode::OFF:  return "�ر�";
    default: return "δ֪ģʽ";
    }
}
std::string acModeToString(ACMode mode) { // �����ļ��洢
    return std::to_string(static_cast<int>(mode));
}
ACMode stringToACMode(const std::string& s) {
    try {
        int val = std::stoi(s);
        // ����ʵ��ö��ֵ��Χ���м�飬���� 0-3 ��Ӧ COOL,HEAT,FAN,OFF
        if (val >= static_cast<int>(ACMode::COOL) && val <= static_cast<int>(ACMode::OFF)) {
            return static_cast<ACMode>(val);
        }
        // ��־Ӧ�ɵ����ߴ����ͨ��ȫ����־��¼
        std::cerr << "����: δ֪�Ŀյ�ģʽֵ '" << s << "'��Ĭ��Ϊ�رա�" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "����: �����յ�ģʽֵ '" << s << "' ʧ��: " << e.what() << "��Ĭ��Ϊ�رա�" << std::endl;
    }
    return ACMode::OFF;
}

// FanSpeed ת������ʵ��
std::string fanSpeedUserString(FanSpeed speed) {
    switch (speed) {
    case FanSpeed::LOW:    return "�ͷ�";
    case FanSpeed::MEDIUM: return "�з�";
    case FanSpeed::HIGH:   return "�߷�";
    case FanSpeed::AUTO:   return "�Զ�";
    default: return "δ֪����";
    }
}
std::string fanSpeedToString(FanSpeed speed) { // �����ļ��洢
    return std::to_string(static_cast<int>(speed));
}
FanSpeed stringToFanSpeed(const std::string& s) {
    try {
        int val = std::stoi(s);
        // ���� 0-3 ��Ӧ LOW,MEDIUM,HIGH,AUTO
        if (val >= static_cast<int>(FanSpeed::LOW) && val <= static_cast<int>(FanSpeed::AUTO)) {
            return static_cast<FanSpeed>(val);
        }
        std::cerr << "����: δ֪�ķ����ٶ�ֵ '" << s << "'��Ĭ��Ϊ�Զ���" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "����: ���������ٶ�ֵ '" << s << "' ʧ��: " << e.what() << "��Ĭ��Ϊ�Զ���" << std::endl;
    }
    return FanSpeed::AUTO;
}


AC::AC(int id, const std::string& name, DeviceImportance importance, double powerConsumption,
    const std::string& location, ACMode mode, double temp, FanSpeed speed)
    : Device(id, name, importance, powerConsumption, location),
    mode(mode), targetTemperature(temp), fanSpeed(speed) {
}

AC::~AC() {
    // std::cout << "�յ� " << name << " (ID: " << id << ") �����١�" << std::endl;
}

std::string AC::toFileString() const {
    std::stringstream ss;
    ss << "AC," // ���ͱ�ʶ
        << Device::toFileString() // ���û����ȡͨ�ò���
        << "," << acModeToString(getMode()) // ʹ������ת������
        << "," << getTargetTemperature()
        << "," << fanSpeedToString(getFanSpeed()); // ʹ������ת������
    return ss.str();
}

void AC::updateStatus() {
    std::cout << "���ڸ��¿յ� " << name << " (ID: " << id << ") ��״̬:" << std::endl; //
    std::cout << "  ��ǰģʽ: " << acModeUserString(mode) << std::endl; //
    if (mode != ACMode::OFF) {
        std::cout << "  Ŀ���¶�: " << targetTemperature << " ��C" << std::endl; //
        std::cout << "  ����: " << fanSpeedUserString(fanSpeed) << std::endl; //
    }
}

void AC::displayInfo() const {
    Device::displayInfo(); // ���û���� displayInfo
    std::cout << ", ����: �յ�"
        << ", ģʽ: " << acModeUserString(mode); //
    if (mode != ACMode::OFF) {
        std::cout << ", Ŀ���¶�: " << targetTemperature << " ��C"
            << ", ����: " << fanSpeedUserString(fanSpeed); //
    }
}

void AC::setMode(ACMode m) {
    mode = m;
    std::cout << "�յ� " << name << " (ID: " << id << ") ģʽ������Ϊ " << acModeUserString(mode) << "��" << std::endl; //
    if (mode == ACMode::OFF) {
        // ����ѡ���ڹر�ʱ����Ŀ���¶Ȼ򱣳�
        // targetTemperature = 25.0; // ���磬Ĭ�Ϲر�ʱ�Ĵ����¶�
    }
}

void AC::setTargetTemperature(double temp) {
    if (mode == ACMode::OFF) {
        std::cout << "�յ� " << name << " (ID: " << id << ") ��ǰΪ�ر�״̬���޷�����Ŀ���¶ȡ�" << std::endl; //
        return;
    }
    // ��������¶ȷ�Χ���
    if (temp < 16.0 || temp > 30.0) { // ʾ����Χ
        std::cout << "�յ� " << name << " (ID: " << id << ") ��Ч��Ŀ���¶ȣ�" << temp << "��C���¶�Ӧ��16-30��C֮�䡣" << std::endl; //
        return;
    }
    targetTemperature = temp;
    std::cout << "�յ� " << name << " (ID: " << id << ") Ŀ���¶�������Ϊ " << targetTemperature << " ��C��" << std::endl; //
}

void AC::setFanSpeed(FanSpeed speed) {
    if (mode == ACMode::OFF) {
        std::cout << "�յ� " << name << " (ID: " << id << ") ��ǰΪ�ر�״̬���޷����÷��١�" << std::endl; //
        return;
    }
    fanSpeed = speed;
    std::cout << "�յ� " << name << " (ID: " << id << ") ����������Ϊ " << fanSpeedUserString(fanSpeed) << "��" << std::endl; //
}

ACMode AC::getMode() const { return mode; } //
double AC::getTargetTemperature() const { return targetTemperature; } //
FanSpeed AC::getFanSpeed() const { return fanSpeed; } //

std::istream& operator>>(std::istream& is, AC& ac) {
    is >> static_cast<Device&>(ac); // ��ȡͨ���豸��Ϣ
    int modeChoice, fanChoice;
    std::cout << "��ѡ��յ�ģʽ (0:����, 1:����, 2:�ͷ�, 3:�ر�): "; //
    is >> modeChoice;
    // ȷ�� modeChoice �� ACMode ö�ٵ���Ч��Χ��
    if (modeChoice >= static_cast<int>(ACMode::COOL) && modeChoice <= static_cast<int>(ACMode::OFF)) {
        ac.mode = static_cast<ACMode>(modeChoice);
    }
    else {
        std::cout << "��Ч��ģʽѡ��Ĭ��Ϊ�رա�" << std::endl;
        ac.mode = ACMode::OFF;
    }


    if (ac.mode != ACMode::OFF) {
        std::cout << "������Ŀ���¶� (���϶�): "; //
        is >> ac.targetTemperature;
        std::cout << "��ѡ����� (0:�ͷ�, 1:�з�, 2:�߷�, 3:�Զ�): "; //
        is >> fanChoice;
        if (fanChoice >= static_cast<int>(FanSpeed::LOW) && fanChoice <= static_cast<int>(FanSpeed::AUTO)) {
            ac.fanSpeed = static_cast<FanSpeed>(fanChoice);
        }
        else {
            std::cout << "��Ч�ķ���ѡ��Ĭ��Ϊ�Զ���" << std::endl;
            ac.fanSpeed = FanSpeed::AUTO;
        }
    }
    is.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // ������з�
    return is;
}