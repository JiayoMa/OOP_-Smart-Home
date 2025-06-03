// Device.cpp
#include "Device.h"
#include "DeviceContainer.h"
// DeviceImportance ö��ת������ʵ��
std::string importanceToString(DeviceImportance imp) { //
    return std::to_string(static_cast<int>(imp)); //
}

DeviceImportance stringToImportance(const std::string& s) { //
    try {
        int val = std::stoi(s); //
        if (val >= static_cast<int>(DeviceImportance::LOW) && val <= static_cast<int>(DeviceImportance::CRITICAL)) { //
            return static_cast<DeviceImportance>(val); //
        }
        // ע�⣺����ֱ�������cerr���ܲ������ʵ����Ӧ��ͨ����־ϵͳ���׳��쳣
        std::cerr << "����: ��Ҫ��ֵ '" << s << "' ����Ԥ�ڷ�Χ��Ĭ��Ϊ �С�" << std::endl; //
    }
    catch (const std::exception& e) {
        std::cerr << "����: ������Ҫ��ֵ '" << s << "' ʧ��: " << e.what() << "��Ĭ��Ϊ �С�" << std::endl; //
    }
    return DeviceImportance::MEDIUM; // Ĭ�Ϸ����е���Ҫ��
}


Device::Device(int id, const std::string& name, DeviceImportance importance,
    double powerConsumption, const std::string& location)
    : id(id), name(name), importance(importance),
    powerConsumption(powerConsumption), emergencyPowerOff(false), location(location) {
}

Device::~Device() {
    // std::cout << "�豸 " << name << " (ID: " << id << ") �����١�" << std::endl;
}

std::string Device::toFileString() const {
    std::stringstream ss;
    // �����ַ��� (Sensor, Light, AC) �� DeviceContainer::saveDevicesToFile ���
    // ����ֻ����ͨ�ò��֣������������ַ�����ǰ������
    ss << getId() << ","
        << getName() << ","
        << importanceToString(getImportance()) << ","
        << getPowerConsumption() << ","
        << (isEmergencyPowerOff() ? "1" : "0") << ","
        << getLocation();
    return ss.str(); //
}

int Device::getId() const { return id; } //
std::string Device::getName() const { return name; } //
DeviceImportance Device::getImportance() const { return importance; } //
double Device::getPowerConsumption() const { return powerConsumption; } //
bool Device::isEmergencyPowerOff() const { return emergencyPowerOff; } //
std::string Device::getLocation() const { return location; }


void Device::setId(int id) { this->id = id; } //
void Device::setName(const std::string& name) { this->name = name; } //
void Device::setImportance(DeviceImportance importance) { this->importance = importance; } //
void Device::setPowerConsumption(double consumption) { this->powerConsumption = consumption; } //
void Device::setEmergencyPowerOff(bool status) { this->emergencyPowerOff = status; } //
void Device::setLocation(const std::string& newLocation) { this->location = newLocation; }


void Device::displayInfo() const {
    std::cout << "�豸ID: " << id << ", ����: " << name
        << ", λ��: " << location
        << ", ��Ҫ�̶�: "; //
    switch (importance) { //
    case DeviceImportance::LOW: std::cout << "��"; break;
    case DeviceImportance::MEDIUM: std::cout << "��"; break;
    case DeviceImportance::HIGH: std::cout << "��"; break;
    case DeviceImportance::CRITICAL: std::cout << "Σ��"; break;
    default: std::cout << "δ֪"; break;
    }
    std::cout << ", ����: " << powerConsumption << " ��" // ���赥λ����
        << ", �����ϵ�״̬: " << (emergencyPowerOff ? "������" : "δ����"); //
    // ����Ӧ�õ����������Ȼ������Լ����ض���Ϣ
}

std::ostream& operator<<(std::ostream& os, const Device& device) {
    device.displayInfo(); // ���� displayInfo ���������
    return os;
}

std::istream& operator>>(std::istream& is, Device& device) {
    // �����������Ҫ����ʾ����ǳ��򵥵����룬ʵ����Ŀ��ͨ�����и����ӵ�UI���������
    std::cout << "�������豸ID: "; //
    is >> device.id;
    is.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // ������з�
    std::cout << "�������豸����: "; //
    std::getline(is, device.name);
    std::cout << "�������豸λ��: "; //
    std::getline(is, device.location);
    // ��Ҫ�̶Ⱥ͹��ĵ�ͨ��ͨ�����ṹ���ķ�ʽ����
    // ���磬ͨ�� DeviceParams �͹����������ض��� setter UI
    std::cout << "�����빦�� (��): "; //
    is >> device.powerConsumption;
    is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return is;
}