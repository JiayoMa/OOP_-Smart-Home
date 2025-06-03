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
// 工厂头文件
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
    // --- 设备管理 ---
    Device* addDeviceFromParams(DeviceType type, const DeviceParams& params);
    Device* findDeviceById(int id); // 查找设备 (可修改)
    const Device* findDeviceById(int id) const; // 查找设备 (不可修改)
    // 更新设备：找到旧设备，删除，然后用新参数创建并替换
    bool updateDevice(int id, const DeviceParams& newParams, const User& currentUser); // 添加用户权限检查
    bool deleteDeviceById(int id, const User& currentUser); 

    void displayAllDevices() const;
    void displayDeviceDetails(int id) const;

    // --- 排序 ---
    void sortByPowerConsumption();
    void sortByLocation();

    // --- 文件输入/输出 ---
    void importDevicesFromFile(const std::string& filename);
    void importDevicesFromFileLogOnly(const std::string& filename);
    void saveDevicesToFile(const std::string& filename) const;

    // --- 统计 ---
    size_t getDeviceCount() const;
    size_t getCountByType(DeviceType type) const; // 按类型统计设备数量

    // 访问器
    const std::vector<Device*>& getAllDevicePtrs() const; // 获取设备指针列表 (const)
    std::vector<Device*>& getAllDevicePtrs();             // 获取设备指针列表 (可修改，谨慎使用)
};