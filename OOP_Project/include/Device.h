// Device.h
#pragma once
#include "Device.h"

#include <string>
#include <iostream>
#include <vector> 
#include <sstream> // ���� toFileString

// �豸����ö�٣����ڹ���������
enum class DeviceType {
    SENSOR,
    LIGHT,
    AC
};

// �豸��Ҫ�̶�ö��
enum class DeviceImportance {
    LOW,     // ��
    MEDIUM,  // ��
    HIGH,    // ��
    CRITICAL // Σ��
};

// ����Ҫ�̶�ö��ת��Ϊ�ַ��������ڱ����ļ�����ʾ��
std::string importanceToString(DeviceImportance imp);
// ���ַ���ת��Ϊ��Ҫ�̶�ö�٣����ڴ��ļ����أ�
DeviceImportance stringToImportance(const std::string& s);


class Device {
protected:
    int id;
    std::string name;
    DeviceImportance importance;
    double powerConsumption;    
    bool emergencyPowerOff;    
    std::string location;     
public:
    Device(int id, const std::string& name, DeviceImportance importance,
        double powerConsumption, const std::string& location = "Ĭ��λ��");
    virtual ~Device();
    virtual std::string toFileString() const;
    virtual void updateStatus() = 0;
    // Getters
    int getId() const;
    std::string getName() const;
    DeviceImportance getImportance() const;
    double getPowerConsumption() const;
    bool isEmergencyPowerOff() const;
    std::string getLocation() const;
    void setId(int id); 
    void setName(const std::string& name);
    void setImportance(DeviceImportance importance);
    void setPowerConsumption(double consumption);
    void setEmergencyPowerOff(bool status);
    void setLocation(const std::string& newLocation);
    virtual void displayInfo() const;
    friend std::ostream& operator<<(std::ostream& os, const Device& device);
    friend std::istream& operator>>(std::istream& is, Device& device);
};