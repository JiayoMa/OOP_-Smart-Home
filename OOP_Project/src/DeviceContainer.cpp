// DeviceContainer.cpp
#include "Device.h"
#include "DeviceContainer.h"
#include "DeviceFactory.h"

#include "Sensor.h" 
#include "Light.h"
#include "AC.h"     

DeviceContainer::DeviceContainer(SmartLogger& loggerRef) : logger(loggerRef) {
    logger.INFO("设备容器已初始化。");
}

DeviceContainer::~DeviceContainer() {
    logger.INFO("设备容器正在销毁。正在删除所有设备...");
    for (Device* device : devices) {
        delete device;
    }
    devices.clear();
    logger.INFO("所有设备已删除。");
}

bool DeviceContainer::isIdDuplicate(int id) const {
    auto it = std::find_if(devices.begin(), devices.end(),
        [id](const Device* d) { return d->getId() == id; });
    return it != devices.end();
}

Device* DeviceContainer::addDeviceFromParams(DeviceType type, const DeviceParams& params) {
    if (isIdDuplicate(params.id)) {
        logger.ALERT("添加设备失败。设备ID " + std::to_string(params.id) + " 已存在。", params.id);
        throw InvalidParameterException("设备ID " + std::to_string(params.id) + " 已存在。");
    }

    Device* newDevice = nullptr;
    try {
        switch (type) {
        case DeviceType::SENSOR:
            newDevice = sensorFactory.createDeviceWithParams(params);
            break;
        case DeviceType::LIGHT:
            newDevice = lightFactory.createDeviceWithParams(params);
            break;
        case DeviceType::AC:
            newDevice = acFactory.createDeviceWithParams(params);
            break;
        default:
            logger.ALERT("未找到未知设备类型的工厂。");
            throw FactoryNotFoundException("指定的设备类型未知。");
        }
    }
    catch (const std::bad_alloc& e) {
        logger.ALERT("为新设备分配内存失败：" + std::string(e.what()), params.id);
        throw; // 重新抛出 std::bad_alloc
    }
    catch (const BaseSmartHomeException& e) { // 捕获我们自定义的异常
        logger.ALERT("创建设备时出错：" + std::string(e.what()), params.id);
        if (newDevice) { // 理论上如果工厂抛出异常，newDevice不会被赋值
            delete newDevice;
            newDevice = nullptr;
        }
        throw; // 重新抛出自定义异常
    }
    catch (const std::exception& e) { // 捕获其他标准异常
        logger.ALERT("创建设备时发生标准库异常：" + std::string(e.what()), params.id);
        if (newDevice) {
            delete newDevice;
            newDevice = nullptr;
        }
        throw InvalidParameterException("创建设备时发生未知错误：" + std::string(e.what())); // 包装成我们的异常类型
    }


    if (newDevice) {
        // DeviceParams 现在包含 emergencyPowerOff 和 location, 工厂的 createDeviceWithParams 应该处理它们
        // 或者在 Device 基类构造函数中处理
        devices.push_back(newDevice);
        logger.INFO("设备已添加：" + newDevice->getName() + " (ID: " + std::to_string(newDevice->getId()) + ")", newDevice->getId());
        return newDevice;
    }
    // 如果 newDevice 为 nullptr 但没有抛出异常（理论上不应该发生）
    logger.ALERT("添加设备失败，未知原因，设备未创建。", params.id);
    return nullptr;
}


Device* DeviceContainer::findDeviceById(int id) {
    auto it = std::find_if(devices.begin(), devices.end(),
        [id](const Device* d) { return d->getId() == id; });
    if (it != devices.end()) {
        return *it;
    }
    logger.DEBUG("未找到ID为 " + std::to_string(id) + " 的设备。", id); // DEBUG级别，因为查找失败不一定是错误
    return nullptr;
}

const Device* DeviceContainer::findDeviceById(int id) const {
    auto it = std::find_if(devices.begin(), devices.end(),
        [id](const Device* d) { return d->getId() == id; });
    if (it != devices.end()) {
        return *it;
    }
    logger.DEBUG("未找到ID为 " + std::to_string(id) + " 的设备 (const)。", id);
    return nullptr;
}


bool DeviceContainer::deleteDeviceById(int id, const User& currentUser) {
    if (currentUser.getRole() != UserRole::ADMIN) {
        logger.ALERT("权限不足：用户 '" + currentUser.getUsername() + "' 尝试删除设备ID " + std::to_string(id) + "。", id);
        throw PermissionDeniedException("只有管理员才能删除设备。");
    }

    auto it = std::find_if(devices.begin(), devices.end(),
        [id](const Device* d) { return d->getId() == id; });

    if (it != devices.end()) {
        Device* toDelete = *it;
        std::string deviceName = toDelete->getName();
        devices.erase(it); // 从向量中移除指针
        delete toDelete;   // 删除实际对象
        logger.INFO("设备ID " + std::to_string(id) + " (" + deviceName + ") 已被管理员 '" + currentUser.getUsername() + "' 删除。", id);
        return true;
    }
    logger.ALERT("删除失败：未找到设备ID " + std::to_string(id) + "。", id);
    throw DeviceNotFoundException(id);
}

bool DeviceContainer::updateDevice(int id, const DeviceParams& newParams, const User& currentUser) {
    if (currentUser.getRole() != UserRole::ADMIN) {
        logger.ALERT("权限不足：用户 '" + currentUser.getUsername() + "' 尝试更新设备ID " + std::to_string(id) + "。", id);
        throw PermissionDeniedException("只有管理员才能修改设备。");
    }

    Device* oldDevice = nullptr;
    size_t oldDeviceIndex = -1;
    for (size_t i = 0; i < devices.size(); ++i) {
        if (devices[i]->getId() == id) {
            oldDevice = devices[i];
            oldDeviceIndex = i;
            break;
        }
    }

    if (!oldDevice) {
        logger.ALERT("无法更新。未找到设备ID " + std::to_string(id) + "。", id);
        throw DeviceNotFoundException(id);
    }

    DeviceType type;
    if (dynamic_cast<Sensor*>(oldDevice)) type = DeviceType::SENSOR;
    else if (dynamic_cast<Light*>(oldDevice)) type = DeviceType::LIGHT;
    else if (dynamic_cast<AC*>(oldDevice)) type = DeviceType::AC;
    else {
        logger.ALERT("无法更新。设备ID " + std::to_string(id) + " 的类型未知。", id);
        throw InvalidParameterException("待更新设备的类型未知。");
    }

    // 如果允许更改ID，需要检查新ID newParams.id 是否与现有ID冲突 (除了当前设备本身)
    if (id != newParams.id) { // 如果ID被更改
        for (const auto& dev : devices) {
            if (dev->getId() == newParams.id) { // 新ID已存在于其他设备
                logger.ALERT("无法更新。新的设备ID " + std::to_string(newParams.id) + " 已被其他设备使用。", newParams.id);
                throw InvalidParameterException("更新操作中的新设备ID已存在。");
            }
        }
    }


    Device* updatedDevice = nullptr;
    try {
        switch (type) { // 使用与旧设备相同的类型创建新设备
        case DeviceType::SENSOR: updatedDevice = sensorFactory.createDeviceWithParams(newParams); break;
        case DeviceType::LIGHT:  updatedDevice = lightFactory.createDeviceWithParams(newParams);  break;
        case DeviceType::AC:     updatedDevice = acFactory.createDeviceWithParams(newParams);     break;
        default: throw FactoryNotFoundException("更新时设备类型未知"); // 理论上不会发生
        }
    }
    catch (const std::exception& e) { // 包括自定义异常和std::bad_alloc
        logger.ALERT("创建更新后的设备时出错：" + std::string(e.what()), id);
        if (updatedDevice) delete updatedDevice; // 清理，以防万一
        throw; // 重新抛出捕获的异常
    }


    if (updatedDevice) {
        delete devices[oldDeviceIndex];      // 删除旧设备对象
        devices[oldDeviceIndex] = updatedDevice; // 将向量中的指针替换为新设备
        logger.INFO("设备ID " + std::to_string(id) + " (新ID可能为 " + std::to_string(newParams.id) + ") 已被管理员 '" + currentUser.getUsername() + "' 更新。", newParams.id);
        return true;
    }

    // Fallback - should not be reached if exceptions are handled properly
    logger.ALERT("完成设备ID " + std::to_string(id) + " 的更新时失败。", id);
    return false;
}


void DeviceContainer::displayAllDevices() const {
    if (devices.empty()) {
        logger.INFO("没有设备可显示。");
        std::cout << "当前设备列表为空。" << std::endl;
        return;
    }
    std::cout << "\n--- 所有设备列表 ---" << std::endl;
    for (const auto& device : devices) {
        if (device) { // 额外的安全检查
            device->displayInfo();
            std::cout << std::endl;
        }
    }
}

void DeviceContainer::displayDeviceDetails(int id) const {
    const Device* device = findDeviceById(id);
    if (device) {
        std::cout << "\n--- 设备详情 (ID: " << id << ") ---" << std::endl;
        device->displayInfo();
        std::cout << std::endl;
    }
    else {
        std::cout << "未找到ID为 " << id << " 的设备。" << std::endl;
        // findDeviceById 已经记录了日志
    }
}

void DeviceContainer::sortByPowerConsumption() {
    std::sort(devices.begin(), devices.end(), [](const Device* a, const Device* b) {
        return a->getPowerConsumption() < b->getPowerConsumption();
        });
    logger.INFO("设备已按功耗排序。");
}

void DeviceContainer::sortByLocation() {
    std::sort(devices.begin(), devices.end(), [](const Device* a, const Device* b) {
        return a->getLocation() < b->getLocation(); // 假设 std::string 的 < 操作符符合排序要求
        });
    logger.INFO("设备已按位置排序。");
}

size_t DeviceContainer::getDeviceCount() const {
    return devices.size();
}

size_t DeviceContainer::getCountByType(DeviceType type) const {
    size_t count = 0;
    for (const auto& device : devices) {
        if (!device) continue; // 安全检查
        switch (type) {
        case DeviceType::SENSOR: if (dynamic_cast<const Sensor*>(device)) count++; break;
        case DeviceType::LIGHT:  if (dynamic_cast<const Light*>(device))  count++; break;
        case DeviceType::AC:     if (dynamic_cast<const AC*>(device))     count++; break;
        }
    }
    return count;
}

const std::vector<Device*>& DeviceContainer::getAllDevicePtrs() const {
    return devices;
}
std::vector<Device*>& DeviceContainer::getAllDevicePtrs() {
    return devices;
}

void DeviceContainer::importDevicesFromFileLogOnly(const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        logger.ALERT("错误：无法打开设备文件 '" + filename + "' 进行读取。");
        std::cout << "错误：无法打开设备文件 '" << filename << "' 进行读取。" << std::endl;
        throw InvalidParameterException("无法打开设备文件进行读取：" + filename);
    }

    logger.INFO("开始从文件 '" + filename + "' 导入设备...");
    //std::cout << "开始从文件 '" << filename << "' 导入设备..." << std::endl;
    std::string line;
    int lineNumber = 0;
    int importedCount = 0;
    int skippedCount = 0;

    while (std::getline(inFile, line)) {
        lineNumber++;
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string segment;
        std::vector<std::string> segments;
        while (std::getline(ss, segment, ',')) {
            segments.push_back(segment);
        }

        // TYPE,ID,Name,Importance,PowerConsumption,EmergencyPowerOff,Location,[specific_params...]
        // 通用参数至少需要7个
        if (segments.size() < 7) {
            logger.ALERT("警告：文件 '" + filename + "' 第 " + std::to_string(lineNumber) + " 行参数不足。跳过：" + line);
           // std::cout << "警告：文件 '" << filename << "' 第 " << std::to_string(lineNumber) << " 行参数不足。跳过：" << line << std::endl;
            skippedCount++;
            continue;
        }

        std::string typeStr = segments[0];
        DeviceParams params;
        try {
            params.id = std::stoi(segments[1]);
            if (isIdDuplicate(params.id)) {
                logger.ALERT("警告：来自文件第 " + std::to_string(lineNumber) + " 行的设备ID " + segments[1] + " 已存在。跳过导入。", params.id);
                //std::cout << "警告：来自文件第 " << std::to_string(lineNumber) << " 行的设备ID " << segments[1] << " 已存在。跳过导入。" << std::endl;
                skippedCount++;
                continue;
            }
            params.name = segments[2];
            params.importance = stringToImportance(segments[3]);
            params.powerConsumption = std::stod(segments[4]);
            params.emergencyPowerOff = (segments[5] == "1");
            params.location = segments[6]; // 新增：位置

            DeviceType deviceTypeEnum;
            bool paramsOk = true;
            if ((typeStr == "SENSOR" || typeStr == "Sensor")) {
                if (segments.size() < 7 + 3) { // 7通用 + 3传感器特定
                    logger.ALERT("警告：文件第 " + std::to_string(lineNumber) + " 行 (传感器) 参数不足。跳过。");
                   // std::cout << "警告：文件第 " + std::to_string(lineNumber) << " 行 (传感器) 参数不足。跳过。" << std::endl;
                    paramsOk = false;
                }
                else {
                    params.temperature = std::stod(segments[7]);
                    params.humidity = std::stod(segments[8]);
                    params.co2Concentration = std::stod(segments[9]);
                    deviceTypeEnum = DeviceType::SENSOR;
                }
            }
            else if ((typeStr == "LIGHT" || typeStr == "Light")) {
                if (segments.size() < 7 + 2) { // 7通用 + 2灯具特定
                    logger.ALERT("警告：文件第 " + std::to_string(lineNumber) + " 行 (灯具) 参数不足。跳过。");
                    //std::cout << "警告：文件第 " + std::to_string(lineNumber) << " 行 (灯具) 参数不足。跳过。" << std::endl;
                    paramsOk = false;
                }
                else {
                    params.isOn = (segments[7] == "1");
                    params.brightness = std::stoi(segments[8]);
                    deviceTypeEnum = DeviceType::LIGHT;
                }
            }
            else if ((typeStr == "AC" || typeStr == "ac")) {
                if (segments.size() < 7 + 3) { // 7通用 + 3空调特定
                    logger.ALERT("警告：文件第 " + std::to_string(lineNumber) + " 行 (空调) 参数不足。跳过。");
                    //std::cout << "警告：文件第 " + std::to_string(lineNumber) << " 行 (空调) 参数不足。跳过。" << std::endl;
                    paramsOk = false;
                }
                else {
                    params.acMode = stringToACMode(segments[7]); // 索引已因位置字段而调整
                    params.targetTemperature = std::stod(segments[8]);
                    params.fanSpeed = stringToFanSpeed(segments[9]);
                    deviceTypeEnum = DeviceType::AC;
                }
            }
            else {
                logger.ALERT("警告：文件第 " + std::to_string(lineNumber) + " 行设备类型 '" + typeStr + "' 无法识别。跳过：" + line);

                paramsOk = false;
            }

            if (paramsOk) {
                addDeviceFromParams(deviceTypeEnum, params); // 使用容器的添加方法
                importedCount++;
            }
            else {
                skippedCount++;
            }

        }
        catch (const std::invalid_argument& e) {
            logger.ALERT("解析文件第 " + std::to_string(lineNumber) + " 行时发生无效参数错误：'" + line + "'。错误详情：" + e.what());

            skippedCount++;
        }
        catch (const std::out_of_range& e) {
            logger.ALERT("解析文件第 " + std::to_string(lineNumber) + " 行时发生越界错误：'" + line + "'。错误详情：" + e.what());

            skippedCount++;
        }
        catch (const BaseSmartHomeException& e) { // 捕获来自 addDeviceFromParams 的自定义异常
            logger.ALERT("从文件第 " + std::to_string(lineNumber) + " 行添加设备时出错：" + e.what());

            skippedCount++;
        }
        catch (const std::exception& e) {
            logger.ALERT("解析文件第 " + std::to_string(lineNumber) + " 行时发生未知错误：'" + line + "'。错误详情：" + e.what());

            skippedCount++;
        }
    }
    inFile.close();
    logger.INFO("设备已从 '" + filename + "' 文件导入完毕。成功导入 " + std::to_string(importedCount) + " 个，跳过 " + std::to_string(skippedCount) + " 个。");
}
void DeviceContainer::importDevicesFromFile(const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        logger.ALERT("错误：无法打开设备文件 '" + filename + "' 进行读取。");
        std::cout << "错误：无法打开设备文件 '" << filename << "' 进行读取。" << std::endl;
        throw InvalidParameterException("无法打开设备文件进行读取：" + filename);
    }

    logger.INFO("开始从文件 '" + filename + "' 导入设备...");
    std::cout << "开始从文件 '" << filename << "' 导入设备..." << std::endl;
    std::string line;
    int lineNumber = 0;
    int importedCount = 0;
    int skippedCount = 0;

    while (std::getline(inFile, line)) {
        lineNumber++;
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string segment;
        std::vector<std::string> segments;
        while (std::getline(ss, segment, ',')) {
            segments.push_back(segment);
        }

        // TYPE,ID,Name,Importance,PowerConsumption,EmergencyPowerOff,Location,[specific_params...]
        // 通用参数至少需要7个
        if (segments.size() < 7) {
            logger.ALERT("警告：文件 '" + filename + "' 第 " + std::to_string(lineNumber) + " 行参数不足。跳过：" + line);
            std::cout << "警告：文件 '" << filename << "' 第 " << std::to_string(lineNumber) << " 行参数不足。跳过：" << line << std::endl;
            skippedCount++;
            continue;
        }

        std::string typeStr = segments[0];
        DeviceParams params;
        try {
            params.id = std::stoi(segments[1]);
            if (isIdDuplicate(params.id)) {
                logger.ALERT("警告：来自文件第 " + std::to_string(lineNumber) + " 行的设备ID " + segments[1] + " 已存在。跳过导入。", params.id);
                std::cout << "警告：来自文件第 " << std::to_string(lineNumber) << " 行的设备ID " << segments[1] << " 已存在。跳过导入。" << std::endl;
                skippedCount++;
                continue;
            }
            params.name = segments[2];
            params.importance = stringToImportance(segments[3]);
            params.powerConsumption = std::stod(segments[4]);
            params.emergencyPowerOff = (segments[5] == "1");
            params.location = segments[6]; // 新增：位置

            DeviceType deviceTypeEnum;
            bool paramsOk = true;
            if ((typeStr == "SENSOR" || typeStr == "Sensor")) {
                if (segments.size() < 7 + 3) { // 7通用 + 3传感器特定
                    logger.ALERT("警告：文件第 " + std::to_string(lineNumber) + " 行 (传感器) 参数不足。跳过。");
                    std::cout << "警告：文件第 " + std::to_string(lineNumber) << " 行 (传感器) 参数不足。跳过。" << std::endl;
                    paramsOk = false;
                }
                else {
                    params.temperature = std::stod(segments[7]);
                    params.humidity = std::stod(segments[8]);
                    params.co2Concentration = std::stod(segments[9]);
                    deviceTypeEnum = DeviceType::SENSOR;
                }
            }
            else if ((typeStr == "LIGHT" || typeStr == "Light")) {
                if (segments.size() < 7 + 2) { // 7通用 + 2灯具特定
                    logger.ALERT("警告：文件第 " + std::to_string(lineNumber) + " 行 (灯具) 参数不足。跳过。");
                    std::cout << "警告：文件第 " + std::to_string(lineNumber) << " 行 (灯具) 参数不足。跳过。" << std::endl;
                    paramsOk = false;
                }
                else {
                    params.isOn = (segments[7] == "1");
                    params.brightness = std::stoi(segments[8]);
                    deviceTypeEnum = DeviceType::LIGHT;
                }
            }
            else if ((typeStr == "AC" || typeStr == "ac")) {
                if (segments.size() < 7 + 3) { // 7通用 + 3空调特定
                    logger.ALERT("警告：文件第 " + std::to_string(lineNumber) + " 行 (空调) 参数不足。跳过。");
                    std::cout << "警告：文件第 " + std::to_string(lineNumber) << " 行 (空调) 参数不足。跳过。" << std::endl;
                    paramsOk = false;
                }
                else {
                    params.acMode = stringToACMode(segments[7]); // 索引已因位置字段而调整
                    params.targetTemperature = std::stod(segments[8]);
                    params.fanSpeed = stringToFanSpeed(segments[9]);
                    deviceTypeEnum = DeviceType::AC;
                }
            }
            else {
                logger.ALERT("警告：文件第 " + std::to_string(lineNumber) + " 行设备类型 '" + typeStr + "' 无法识别。跳过：" + line);
                std::cout << "警告：文件第 " + std::to_string(lineNumber) + " 行设备类型 '" + typeStr + "' 无法识别。跳过：" << line << std::endl;
                paramsOk = false;
            }

            if (paramsOk) {
                addDeviceFromParams(deviceTypeEnum, params); // 使用容器的添加方法
                importedCount++;
            }
            else {
                skippedCount++;
            }

        }
        catch (const std::invalid_argument& e) {
            logger.ALERT("解析文件第 " + std::to_string(lineNumber) + " 行时发生无效参数错误：'" + line + "'。错误详情：" + e.what());
            std::cout << "解析文件第 " + std::to_string(lineNumber) + " 行时发生无效参数错误：'" + line + "'。错误详情：" << e.what() << std::endl;
            skippedCount++;
        }
        catch (const std::out_of_range& e) {
            logger.ALERT("解析文件第 " + std::to_string(lineNumber) + " 行时发生越界错误：'" + line + "'。错误详情：" + e.what());
            std::cout << "解析文件第 " + std::to_string(lineNumber) + " 行时发生越界错误：'" + line + "'。错误详情：" << e.what() << std::endl;
            skippedCount++;
        }
        catch (const BaseSmartHomeException& e) { // 捕获来自 addDeviceFromParams 的自定义异常
            logger.ALERT("从文件第 " + std::to_string(lineNumber) + " 行添加设备时出错：" + e.what());
            std::cout << "从文件第 " + std::to_string(lineNumber) + " 行添加设备时出错：" << e.what() << std::endl;
            skippedCount++;
        }
        catch (const std::exception& e) {
            logger.ALERT("解析文件第 " + std::to_string(lineNumber) + " 行时发生未知错误：'" + line + "'。错误详情：" + e.what());
            std::cout << "解析文件第 " + std::to_string(lineNumber) + " 行时发生未知错误：'" + line + "'。错误详情：" << e.what() << std::endl;
            skippedCount++;
        }
    }
    inFile.close();
    logger.INFO("设备已从 '" + filename + "' 文件导入完毕。成功导入 " + std::to_string(importedCount) + " 个，跳过 " + std::to_string(skippedCount) + " 个。");
    std::cout << "设备已从 '" << filename << "' 文件导入完毕。成功导入 " << std::to_string(importedCount) << " 个，跳过 " << std::to_string(skippedCount) << " 个。" << std::endl;
}
void DeviceContainer::saveDevicesToFile(const std::string& filename) const {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        logger.ALERT("错误：无法打开设备文件 '" + filename + "' 进行写入。");
        throw InvalidParameterException("无法打开设备文件进行写入：" + filename);
    }

    logger.INFO("开始将设备信息保存到文件 '" + filename + "'...");
    outFile << "# 文件格式: TYPE,ID,Name,Importance(0-3),PowerConsumption,EmergencyPowerOff(0/1),Location,[specific_params...]\n";
    outFile << "# SENSOR,ID,Name,Imp,PC,EPO,Location,temperature,humidity,co2Concentration\n";
    outFile << "# LIGHT,ID,Name,Imp,PC,EPO,Location,isOn(0/1),brightness\n";
    outFile << "# AC,ID,Name,Imp,PC,EPO,Location,mode(0-3),targetTemperature,fanSpeed(0-3)\n";

    for (const auto& device : devices) {
        if (device) { // 安全检查
            outFile << device->toFileString() << std::endl; // Device及其派生类的toFileString现在处理其完整行（包括类型）
        }
    }
    outFile.close();
    logger.INFO("所有设备信息已保存至 '" + filename + "'。共保存 " + std::to_string(devices.size()) + " 个设备。");
}