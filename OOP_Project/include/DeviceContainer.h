// DeviceContainer.h
#pragma once

#include <vector>
#include <string>
#include <algorithm> 
#include <memory>    
#include <fstream>
#include <sstream>
#include <iostream> 

#include "Device.h"
#include "DeviceParams.h"
#include "CustomExceptions.h"
#include "SmartLogger.h" 
#include "User.h"
// ����ͷ�ļ�
#include "SensorFactory.h"
#include "LightFactory.h"
#include "ACFactory.h"


class DeviceContainer {
private:
    std::vector<Device*> devices; 
    SmartLogger& logger;          


    SensorFactory sensorFactory;
    LightFactory lightFactory;
    ACFactory acFactory;


    bool isIdDuplicate(int id) const;

public:
    DeviceContainer(SmartLogger& loggerRef);
    ~DeviceContainer();


    DeviceContainer(const DeviceContainer&) = delete;
    DeviceContainer& operator=(const DeviceContainer&) = delete;
    DeviceContainer(DeviceContainer&&) = default;
    DeviceContainer& operator=(DeviceContainer&&) = default; 
    // --- �豸���� ---
    Device* addDeviceFromParams(DeviceType type, const DeviceParams& params);
    Device* findDeviceById(int id); // �����豸 (���޸�)
    const Device* findDeviceById(int id) const; // �����豸 (�����޸�)
    // �����豸���ҵ����豸��ɾ����Ȼ�����²����������滻
    bool updateDevice(int id, const DeviceParams& newParams, const User& currentUser); // ����û�Ȩ�޼��
    bool deleteDeviceById(int id, const User& currentUser); 

    void displayAllDevices() const;
    void displayDeviceDetails(int id) const;

    // --- ���� ---
    void sortByPowerConsumption();
    void sortByLocation();

    // --- �ļ�����/��� ---
    void importDevicesFromFile(const std::string& filename);
    void importDevicesFromFileLogOnly(const std::string& filename);
    void saveDevicesToFile(const std::string& filename) const;

    // --- ͳ�� ---
    size_t getDeviceCount() const;
    size_t getCountByType(DeviceType type) const; // ������ͳ���豸����

    // ������
    const std::vector<Device*>& getAllDevicePtrs() const; // ��ȡ�豸ָ���б� (const)
    std::vector<Device*>& getAllDevicePtrs();             // ��ȡ�豸ָ���б� (���޸ģ�����ʹ��)
};