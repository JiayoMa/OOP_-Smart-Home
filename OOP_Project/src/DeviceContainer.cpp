// DeviceContainer.cpp
#include "Device.h"
#include "DeviceContainer.h"
#include "DeviceFactory.h"

#include "Sensor.h" 
#include "Light.h"
#include "AC.h"     

DeviceContainer::DeviceContainer(SmartLogger& loggerRef) : logger(loggerRef) {
    logger.INFO("�豸�����ѳ�ʼ����");
}

DeviceContainer::~DeviceContainer() {
    logger.INFO("�豸�����������١�����ɾ�������豸...");
    for (Device* device : devices) {
        delete device;
    }
    devices.clear();
    logger.INFO("�����豸��ɾ����");
}

bool DeviceContainer::isIdDuplicate(int id) const {
    auto it = std::find_if(devices.begin(), devices.end(),
        [id](const Device* d) { return d->getId() == id; });
    return it != devices.end();
}

Device* DeviceContainer::addDeviceFromParams(DeviceType type, const DeviceParams& params) {
    if (isIdDuplicate(params.id)) {
        logger.ALERT("����豸ʧ�ܡ��豸ID " + std::to_string(params.id) + " �Ѵ��ڡ�", params.id);
        throw InvalidParameterException("�豸ID " + std::to_string(params.id) + " �Ѵ��ڡ�");
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
            logger.ALERT("δ�ҵ�δ֪�豸���͵Ĺ�����");
            throw FactoryNotFoundException("ָ�����豸����δ֪��");
        }
    }
    catch (const std::bad_alloc& e) {
        logger.ALERT("Ϊ���豸�����ڴ�ʧ�ܣ�" + std::string(e.what()), params.id);
        throw; // �����׳� std::bad_alloc
    }
    catch (const BaseSmartHomeException& e) { // ���������Զ�����쳣
        logger.ALERT("�����豸ʱ����" + std::string(e.what()), params.id);
        if (newDevice) { // ��������������׳��쳣��newDevice���ᱻ��ֵ
            delete newDevice;
            newDevice = nullptr;
        }
        throw; // �����׳��Զ����쳣
    }
    catch (const std::exception& e) { // ����������׼�쳣
        logger.ALERT("�����豸ʱ������׼���쳣��" + std::string(e.what()), params.id);
        if (newDevice) {
            delete newDevice;
            newDevice = nullptr;
        }
        throw InvalidParameterException("�����豸ʱ����δ֪����" + std::string(e.what())); // ��װ�����ǵ��쳣����
    }


    if (newDevice) {
        // DeviceParams ���ڰ��� emergencyPowerOff �� location, ������ createDeviceWithParams Ӧ�ô�������
        // ������ Device ���๹�캯���д���
        devices.push_back(newDevice);
        logger.INFO("�豸����ӣ�" + newDevice->getName() + " (ID: " + std::to_string(newDevice->getId()) + ")", newDevice->getId());
        return newDevice;
    }
    // ��� newDevice Ϊ nullptr ��û���׳��쳣�������ϲ�Ӧ�÷�����
    logger.ALERT("����豸ʧ�ܣ�δ֪ԭ���豸δ������", params.id);
    return nullptr;
}


Device* DeviceContainer::findDeviceById(int id) {
    auto it = std::find_if(devices.begin(), devices.end(),
        [id](const Device* d) { return d->getId() == id; });
    if (it != devices.end()) {
        return *it;
    }
    logger.DEBUG("δ�ҵ�IDΪ " + std::to_string(id) + " ���豸��", id); // DEBUG������Ϊ����ʧ�ܲ�һ���Ǵ���
    return nullptr;
}

const Device* DeviceContainer::findDeviceById(int id) const {
    auto it = std::find_if(devices.begin(), devices.end(),
        [id](const Device* d) { return d->getId() == id; });
    if (it != devices.end()) {
        return *it;
    }
    logger.DEBUG("δ�ҵ�IDΪ " + std::to_string(id) + " ���豸 (const)��", id);
    return nullptr;
}


bool DeviceContainer::deleteDeviceById(int id, const User& currentUser) {
    if (currentUser.getRole() != UserRole::ADMIN) {
        logger.ALERT("Ȩ�޲��㣺�û� '" + currentUser.getUsername() + "' ����ɾ���豸ID " + std::to_string(id) + "��", id);
        throw PermissionDeniedException("ֻ�й���Ա����ɾ���豸��");
    }

    auto it = std::find_if(devices.begin(), devices.end(),
        [id](const Device* d) { return d->getId() == id; });

    if (it != devices.end()) {
        Device* toDelete = *it;
        std::string deviceName = toDelete->getName();
        devices.erase(it); // ���������Ƴ�ָ��
        delete toDelete;   // ɾ��ʵ�ʶ���
        logger.INFO("�豸ID " + std::to_string(id) + " (" + deviceName + ") �ѱ�����Ա '" + currentUser.getUsername() + "' ɾ����", id);
        return true;
    }
    logger.ALERT("ɾ��ʧ�ܣ�δ�ҵ��豸ID " + std::to_string(id) + "��", id);
    throw DeviceNotFoundException(id);
}

bool DeviceContainer::updateDevice(int id, const DeviceParams& newParams, const User& currentUser) {
    if (currentUser.getRole() != UserRole::ADMIN) {
        logger.ALERT("Ȩ�޲��㣺�û� '" + currentUser.getUsername() + "' ���Ը����豸ID " + std::to_string(id) + "��", id);
        throw PermissionDeniedException("ֻ�й���Ա�����޸��豸��");
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
        logger.ALERT("�޷����¡�δ�ҵ��豸ID " + std::to_string(id) + "��", id);
        throw DeviceNotFoundException(id);
    }

    DeviceType type;
    if (dynamic_cast<Sensor*>(oldDevice)) type = DeviceType::SENSOR;
    else if (dynamic_cast<Light*>(oldDevice)) type = DeviceType::LIGHT;
    else if (dynamic_cast<AC*>(oldDevice)) type = DeviceType::AC;
    else {
        logger.ALERT("�޷����¡��豸ID " + std::to_string(id) + " ������δ֪��", id);
        throw InvalidParameterException("�������豸������δ֪��");
    }

    // ����������ID����Ҫ�����ID newParams.id �Ƿ�������ID��ͻ (���˵�ǰ�豸����)
    if (id != newParams.id) { // ���ID������
        for (const auto& dev : devices) {
            if (dev->getId() == newParams.id) { // ��ID�Ѵ����������豸
                logger.ALERT("�޷����¡��µ��豸ID " + std::to_string(newParams.id) + " �ѱ������豸ʹ�á�", newParams.id);
                throw InvalidParameterException("���²����е����豸ID�Ѵ��ڡ�");
            }
        }
    }


    Device* updatedDevice = nullptr;
    try {
        switch (type) { // ʹ������豸��ͬ�����ʹ������豸
        case DeviceType::SENSOR: updatedDevice = sensorFactory.createDeviceWithParams(newParams); break;
        case DeviceType::LIGHT:  updatedDevice = lightFactory.createDeviceWithParams(newParams);  break;
        case DeviceType::AC:     updatedDevice = acFactory.createDeviceWithParams(newParams);     break;
        default: throw FactoryNotFoundException("����ʱ�豸����δ֪"); // �����ϲ��ᷢ��
        }
    }
    catch (const std::exception& e) { // �����Զ����쳣��std::bad_alloc
        logger.ALERT("�������º���豸ʱ����" + std::string(e.what()), id);
        if (updatedDevice) delete updatedDevice; // �����Է���һ
        throw; // �����׳�������쳣
    }


    if (updatedDevice) {
        delete devices[oldDeviceIndex];      // ɾ�����豸����
        devices[oldDeviceIndex] = updatedDevice; // �������е�ָ���滻Ϊ���豸
        logger.INFO("�豸ID " + std::to_string(id) + " (��ID����Ϊ " + std::to_string(newParams.id) + ") �ѱ�����Ա '" + currentUser.getUsername() + "' ���¡�", newParams.id);
        return true;
    }

    // Fallback - should not be reached if exceptions are handled properly
    logger.ALERT("����豸ID " + std::to_string(id) + " �ĸ���ʱʧ�ܡ�", id);
    return false;
}


void DeviceContainer::displayAllDevices() const {
    if (devices.empty()) {
        logger.INFO("û���豸����ʾ��");
        std::cout << "��ǰ�豸�б�Ϊ�ա�" << std::endl;
        return;
    }
    std::cout << "\n--- �����豸�б� ---" << std::endl;
    for (const auto& device : devices) {
        if (device) { // ����İ�ȫ���
            device->displayInfo();
            std::cout << std::endl;
        }
    }
}

void DeviceContainer::displayDeviceDetails(int id) const {
    const Device* device = findDeviceById(id);
    if (device) {
        std::cout << "\n--- �豸���� (ID: " << id << ") ---" << std::endl;
        device->displayInfo();
        std::cout << std::endl;
    }
    else {
        std::cout << "δ�ҵ�IDΪ " << id << " ���豸��" << std::endl;
        // findDeviceById �Ѿ���¼����־
    }
}

void DeviceContainer::sortByPowerConsumption() {
    std::sort(devices.begin(), devices.end(), [](const Device* a, const Device* b) {
        return a->getPowerConsumption() < b->getPowerConsumption();
        });
    logger.INFO("�豸�Ѱ���������");
}

void DeviceContainer::sortByLocation() {
    std::sort(devices.begin(), devices.end(), [](const Device* a, const Device* b) {
        return a->getLocation() < b->getLocation(); // ���� std::string �� < ��������������Ҫ��
        });
    logger.INFO("�豸�Ѱ�λ������");
}

size_t DeviceContainer::getDeviceCount() const {
    return devices.size();
}

size_t DeviceContainer::getCountByType(DeviceType type) const {
    size_t count = 0;
    for (const auto& device : devices) {
        if (!device) continue; // ��ȫ���
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
        logger.ALERT("�����޷����豸�ļ� '" + filename + "' ���ж�ȡ��");
        std::cout << "�����޷����豸�ļ� '" << filename << "' ���ж�ȡ��" << std::endl;
        throw InvalidParameterException("�޷����豸�ļ����ж�ȡ��" + filename);
    }

    logger.INFO("��ʼ���ļ� '" + filename + "' �����豸...");
    //std::cout << "��ʼ���ļ� '" << filename << "' �����豸..." << std::endl;
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
        // ͨ�ò���������Ҫ7��
        if (segments.size() < 7) {
            logger.ALERT("���棺�ļ� '" + filename + "' �� " + std::to_string(lineNumber) + " �в������㡣������" + line);
           // std::cout << "���棺�ļ� '" << filename << "' �� " << std::to_string(lineNumber) << " �в������㡣������" << line << std::endl;
            skippedCount++;
            continue;
        }

        std::string typeStr = segments[0];
        DeviceParams params;
        try {
            params.id = std::stoi(segments[1]);
            if (isIdDuplicate(params.id)) {
                logger.ALERT("���棺�����ļ��� " + std::to_string(lineNumber) + " �е��豸ID " + segments[1] + " �Ѵ��ڡ��������롣", params.id);
                //std::cout << "���棺�����ļ��� " << std::to_string(lineNumber) << " �е��豸ID " << segments[1] << " �Ѵ��ڡ��������롣" << std::endl;
                skippedCount++;
                continue;
            }
            params.name = segments[2];
            params.importance = stringToImportance(segments[3]);
            params.powerConsumption = std::stod(segments[4]);
            params.emergencyPowerOff = (segments[5] == "1");
            params.location = segments[6]; // ������λ��

            DeviceType deviceTypeEnum;
            bool paramsOk = true;
            if ((typeStr == "SENSOR" || typeStr == "Sensor")) {
                if (segments.size() < 7 + 3) { // 7ͨ�� + 3�������ض�
                    logger.ALERT("���棺�ļ��� " + std::to_string(lineNumber) + " �� (������) �������㡣������");
                   // std::cout << "���棺�ļ��� " + std::to_string(lineNumber) << " �� (������) �������㡣������" << std::endl;
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
                if (segments.size() < 7 + 2) { // 7ͨ�� + 2�ƾ��ض�
                    logger.ALERT("���棺�ļ��� " + std::to_string(lineNumber) + " �� (�ƾ�) �������㡣������");
                    //std::cout << "���棺�ļ��� " + std::to_string(lineNumber) << " �� (�ƾ�) �������㡣������" << std::endl;
                    paramsOk = false;
                }
                else {
                    params.isOn = (segments[7] == "1");
                    params.brightness = std::stoi(segments[8]);
                    deviceTypeEnum = DeviceType::LIGHT;
                }
            }
            else if ((typeStr == "AC" || typeStr == "ac")) {
                if (segments.size() < 7 + 3) { // 7ͨ�� + 3�յ��ض�
                    logger.ALERT("���棺�ļ��� " + std::to_string(lineNumber) + " �� (�յ�) �������㡣������");
                    //std::cout << "���棺�ļ��� " + std::to_string(lineNumber) << " �� (�յ�) �������㡣������" << std::endl;
                    paramsOk = false;
                }
                else {
                    params.acMode = stringToACMode(segments[7]); // ��������λ���ֶζ�����
                    params.targetTemperature = std::stod(segments[8]);
                    params.fanSpeed = stringToFanSpeed(segments[9]);
                    deviceTypeEnum = DeviceType::AC;
                }
            }
            else {
                logger.ALERT("���棺�ļ��� " + std::to_string(lineNumber) + " ���豸���� '" + typeStr + "' �޷�ʶ��������" + line);

                paramsOk = false;
            }

            if (paramsOk) {
                addDeviceFromParams(deviceTypeEnum, params); // ʹ����������ӷ���
                importedCount++;
            }
            else {
                skippedCount++;
            }

        }
        catch (const std::invalid_argument& e) {
            logger.ALERT("�����ļ��� " + std::to_string(lineNumber) + " ��ʱ������Ч��������'" + line + "'���������飺" + e.what());

            skippedCount++;
        }
        catch (const std::out_of_range& e) {
            logger.ALERT("�����ļ��� " + std::to_string(lineNumber) + " ��ʱ����Խ�����'" + line + "'���������飺" + e.what());

            skippedCount++;
        }
        catch (const BaseSmartHomeException& e) { // �������� addDeviceFromParams ���Զ����쳣
            logger.ALERT("���ļ��� " + std::to_string(lineNumber) + " ������豸ʱ����" + e.what());

            skippedCount++;
        }
        catch (const std::exception& e) {
            logger.ALERT("�����ļ��� " + std::to_string(lineNumber) + " ��ʱ����δ֪����'" + line + "'���������飺" + e.what());

            skippedCount++;
        }
    }
    inFile.close();
    logger.INFO("�豸�Ѵ� '" + filename + "' �ļ�������ϡ��ɹ����� " + std::to_string(importedCount) + " �������� " + std::to_string(skippedCount) + " ����");
}
void DeviceContainer::importDevicesFromFile(const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        logger.ALERT("�����޷����豸�ļ� '" + filename + "' ���ж�ȡ��");
        std::cout << "�����޷����豸�ļ� '" << filename << "' ���ж�ȡ��" << std::endl;
        throw InvalidParameterException("�޷����豸�ļ����ж�ȡ��" + filename);
    }

    logger.INFO("��ʼ���ļ� '" + filename + "' �����豸...");
    std::cout << "��ʼ���ļ� '" << filename << "' �����豸..." << std::endl;
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
        // ͨ�ò���������Ҫ7��
        if (segments.size() < 7) {
            logger.ALERT("���棺�ļ� '" + filename + "' �� " + std::to_string(lineNumber) + " �в������㡣������" + line);
            std::cout << "���棺�ļ� '" << filename << "' �� " << std::to_string(lineNumber) << " �в������㡣������" << line << std::endl;
            skippedCount++;
            continue;
        }

        std::string typeStr = segments[0];
        DeviceParams params;
        try {
            params.id = std::stoi(segments[1]);
            if (isIdDuplicate(params.id)) {
                logger.ALERT("���棺�����ļ��� " + std::to_string(lineNumber) + " �е��豸ID " + segments[1] + " �Ѵ��ڡ��������롣", params.id);
                std::cout << "���棺�����ļ��� " << std::to_string(lineNumber) << " �е��豸ID " << segments[1] << " �Ѵ��ڡ��������롣" << std::endl;
                skippedCount++;
                continue;
            }
            params.name = segments[2];
            params.importance = stringToImportance(segments[3]);
            params.powerConsumption = std::stod(segments[4]);
            params.emergencyPowerOff = (segments[5] == "1");
            params.location = segments[6]; // ������λ��

            DeviceType deviceTypeEnum;
            bool paramsOk = true;
            if ((typeStr == "SENSOR" || typeStr == "Sensor")) {
                if (segments.size() < 7 + 3) { // 7ͨ�� + 3�������ض�
                    logger.ALERT("���棺�ļ��� " + std::to_string(lineNumber) + " �� (������) �������㡣������");
                    std::cout << "���棺�ļ��� " + std::to_string(lineNumber) << " �� (������) �������㡣������" << std::endl;
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
                if (segments.size() < 7 + 2) { // 7ͨ�� + 2�ƾ��ض�
                    logger.ALERT("���棺�ļ��� " + std::to_string(lineNumber) + " �� (�ƾ�) �������㡣������");
                    std::cout << "���棺�ļ��� " + std::to_string(lineNumber) << " �� (�ƾ�) �������㡣������" << std::endl;
                    paramsOk = false;
                }
                else {
                    params.isOn = (segments[7] == "1");
                    params.brightness = std::stoi(segments[8]);
                    deviceTypeEnum = DeviceType::LIGHT;
                }
            }
            else if ((typeStr == "AC" || typeStr == "ac")) {
                if (segments.size() < 7 + 3) { // 7ͨ�� + 3�յ��ض�
                    logger.ALERT("���棺�ļ��� " + std::to_string(lineNumber) + " �� (�յ�) �������㡣������");
                    std::cout << "���棺�ļ��� " + std::to_string(lineNumber) << " �� (�յ�) �������㡣������" << std::endl;
                    paramsOk = false;
                }
                else {
                    params.acMode = stringToACMode(segments[7]); // ��������λ���ֶζ�����
                    params.targetTemperature = std::stod(segments[8]);
                    params.fanSpeed = stringToFanSpeed(segments[9]);
                    deviceTypeEnum = DeviceType::AC;
                }
            }
            else {
                logger.ALERT("���棺�ļ��� " + std::to_string(lineNumber) + " ���豸���� '" + typeStr + "' �޷�ʶ��������" + line);
                std::cout << "���棺�ļ��� " + std::to_string(lineNumber) + " ���豸���� '" + typeStr + "' �޷�ʶ��������" << line << std::endl;
                paramsOk = false;
            }

            if (paramsOk) {
                addDeviceFromParams(deviceTypeEnum, params); // ʹ����������ӷ���
                importedCount++;
            }
            else {
                skippedCount++;
            }

        }
        catch (const std::invalid_argument& e) {
            logger.ALERT("�����ļ��� " + std::to_string(lineNumber) + " ��ʱ������Ч��������'" + line + "'���������飺" + e.what());
            std::cout << "�����ļ��� " + std::to_string(lineNumber) + " ��ʱ������Ч��������'" + line + "'���������飺" << e.what() << std::endl;
            skippedCount++;
        }
        catch (const std::out_of_range& e) {
            logger.ALERT("�����ļ��� " + std::to_string(lineNumber) + " ��ʱ����Խ�����'" + line + "'���������飺" + e.what());
            std::cout << "�����ļ��� " + std::to_string(lineNumber) + " ��ʱ����Խ�����'" + line + "'���������飺" << e.what() << std::endl;
            skippedCount++;
        }
        catch (const BaseSmartHomeException& e) { // �������� addDeviceFromParams ���Զ����쳣
            logger.ALERT("���ļ��� " + std::to_string(lineNumber) + " ������豸ʱ����" + e.what());
            std::cout << "���ļ��� " + std::to_string(lineNumber) + " ������豸ʱ����" << e.what() << std::endl;
            skippedCount++;
        }
        catch (const std::exception& e) {
            logger.ALERT("�����ļ��� " + std::to_string(lineNumber) + " ��ʱ����δ֪����'" + line + "'���������飺" + e.what());
            std::cout << "�����ļ��� " + std::to_string(lineNumber) + " ��ʱ����δ֪����'" + line + "'���������飺" << e.what() << std::endl;
            skippedCount++;
        }
    }
    inFile.close();
    logger.INFO("�豸�Ѵ� '" + filename + "' �ļ�������ϡ��ɹ����� " + std::to_string(importedCount) + " �������� " + std::to_string(skippedCount) + " ����");
    std::cout << "�豸�Ѵ� '" << filename << "' �ļ�������ϡ��ɹ����� " << std::to_string(importedCount) << " �������� " << std::to_string(skippedCount) << " ����" << std::endl;
}
void DeviceContainer::saveDevicesToFile(const std::string& filename) const {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        logger.ALERT("�����޷����豸�ļ� '" + filename + "' ����д�롣");
        throw InvalidParameterException("�޷����豸�ļ�����д�룺" + filename);
    }

    logger.INFO("��ʼ���豸��Ϣ���浽�ļ� '" + filename + "'...");
    outFile << "# �ļ���ʽ: TYPE,ID,Name,Importance(0-3),PowerConsumption,EmergencyPowerOff(0/1),Location,[specific_params...]\n";
    outFile << "# SENSOR,ID,Name,Imp,PC,EPO,Location,temperature,humidity,co2Concentration\n";
    outFile << "# LIGHT,ID,Name,Imp,PC,EPO,Location,isOn(0/1),brightness\n";
    outFile << "# AC,ID,Name,Imp,PC,EPO,Location,mode(0-3),targetTemperature,fanSpeed(0-3)\n";

    for (const auto& device : devices) {
        if (device) { // ��ȫ���
            outFile << device->toFileString() << std::endl; // Device�����������toFileString���ڴ����������У��������ͣ�
        }
    }
    outFile.close();
    logger.INFO("�����豸��Ϣ�ѱ����� '" + filename + "'�������� " + std::to_string(devices.size()) + " ���豸��");
}