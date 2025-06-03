
#include "Light.h"
#include <iostream>
#include <sstream>
#include "Device.h"

Light::Light(int id, const std::string& name, DeviceImportance importance, double powerConsumption,
    const std::string& location, bool on, int bright)
    : Device(id, name, importance, powerConsumption, location), isOn(on) {
    if (bright < 0) brightness = 0;
    else if (bright > 100) brightness = 100;
    else brightness = bright;

    if (brightness > 0 && !isOn) { // ������ȴ���0��״̬�ǹأ����Զ���
        this->isOn = true;
    }
}

Light::~Light() {
    // std::cout << "���ܵƾ� " << name << " (ID: " << id << ") �����١�" << std::endl;
}

std::string Light::toFileString() const {
    std::stringstream ss;
    ss << "LIGHT," // ���ͱ�ʶ
        << Device::toFileString() // ���û����ȡͨ�ò���
        << "," << (getIsOn() ? "1" : "0")
        << "," << getBrightness();
    return ss.str();
}

void Light::updateStatus() {
    // �ƾߵ�״̬ͨ����ֱ�ӿ��Ƹı䣬������Լ���ʾ��ǰ״̬
    std::cout << "���ڸ������ܵƾ� " << name << " (ID: " << id << ") ��״̬:" << std::endl; //
    std::cout << "  ��ǰ״̬: " << (isOn ? "����" : "�ر�") << std::endl; //
    if (isOn) {
        std::cout << "  ����: " << brightness << "%" << std::endl; //
    }
}

void Light::displayInfo() const {
    Device::displayInfo(); // ���û���� displayInfo
    std::cout << ", ����: ���ܵƾ�"
        << ", ״̬: " << (isOn ? "����" : "�ر�"); //
    if (isOn) {
        std::cout << ", ����: " << brightness << "%"; //
    }
}

void Light::turnOn() {
    isOn = true;
    if (brightness == 0) brightness = 50; // �����ʱ����Ϊ0����ΪĬ������
    std::cout << "���ܵƾ� " << name << " (ID: " << id << ") �ѿ�����" << std::endl; //
}

void Light::turnOff() {
    isOn = false;
    // ���ȿ��Ա��֣��´δ�ʱ�ָ���������Ϊ0�������ѡ��
    // brightness = 0; 
    std::cout << "���ܵƾ� " << name << " (ID: " << id << ") �ѹرա�" << std::endl; //
}

void Light::setBrightness(int level) {
    if (level >= 0 && level <= 100) {
        brightness = level;
        std::cout << "���ܵƾ� " << name << " (ID: " << id << ") ����������Ϊ " << brightness << "%��" << std::endl; //
        if (brightness > 0 && !isOn) { // ��������������ҵ��ǹصģ���򿪵�
            turnOn(); //
        }
        else if (brightness == 0 && isOn) { // ���������Ϊ0�ҵ��ǿ��ģ���رյ�
            turnOff(); //
        }
    }
    else {
        std::cout << "����ʧ�ܣ���Ч�����ȼ������ȱ����� 0 �� 100 ֮�䡣" << std::endl; //
    }
}

bool Light::getIsOn() const { return isOn; }             //
int Light::getBrightness() const { return brightness; } //

std::istream& operator>>(std::istream& is, Light& light) {
    is >> static_cast<Device&>(light); // ��ȡͨ���豸��Ϣ
    std::cout << "���Ƿ��ʼ����? (1 ������, 0 �����): "; //
    int onStateChoice;
    is >> onStateChoice;
    light.isOn = (onStateChoice == 1); //

    if (light.isOn) {
        std::cout << "�������ʼ���� (0-100%): "; //
        is >> light.brightness;
        if (light.brightness < 0) light.brightness = 0;       //
        if (light.brightness > 100) light.brightness = 100;   //
    }
    else {
        light.brightness = 0; // �ر�ʱ����Ϊ0
    }
    is.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // ������з�
    return is;
}