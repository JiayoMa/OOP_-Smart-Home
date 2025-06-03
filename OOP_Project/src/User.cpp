// User.cpp
#include "User.h"
#include "Sensor.h" // Ϊ�� dynamic_cast
#include "AC.h"     // Ϊ�� dynamic_cast
#include "Light.h"  // Ϊ�� dynamic_cast
#include <iostream> // Ϊ�˳���ģ���е���� (������ʹ��logger)

User::User(const std::string& name, const std::string& pwd, UserRole r,
    std::vector<Device*>& deviceList, SmartLogger& loggerRef)
    : username(name), password(pwd), role(r), devices(deviceList), logger(loggerRef) {
    logger.INFO("�û� '" + username + "' �Ѵ�����", -1);
}

std::string User::getUsername() const {
    return username;
}

std::string User::getPassword() const {
    // ����: ��ʵ��ϵͳ�в�Ӧֱ�ӷ�����������
    return password;
}

UserRole User::getRole() const {
    return role;
}

const std::vector<Device*>& User::getDevices() const {
    return devices;
}
void User::runTemperatureHumidityRule(double tempThreshold,double humidityThresholdHigh ) {
    std::cout << "�û� '" << username << "' ����ִ����ʪ�ȳ��޹�����..." << std::endl;
    logger.INFO("�û� '" + username + "' ����ִ����ʪ�ȳ��޹�����...", -1, std::this_thread::get_id());
    bool condition_met_high = false;
    bool condition_met_low = false;

    for (Device* device_ptr : devices) {
        if (Sensor* sensor = dynamic_cast<Sensor*>(device_ptr)) {
            // �����¸�ʪ
            if (sensor->getTemperature() > tempThreshold || sensor->getHumidity() > humidityThresholdHigh) {
                condition_met_high = true;
                std::cout << "����: ������ " << sensor->getName() << " (ID: " << sensor->getId()
                    << ") ��⵽�¶� " << sensor->getTemperature() << "��C, ʪ�� " << sensor->getHumidity()
                    << "%����ʪ�ȳ��ޣ�(����)" << std::endl;
                logger.ALERT("����: ������ " + sensor->getName() + " (ID: " + std::to_string(sensor->getId()) +
                    ") ��⵽�¶� " + std::to_string(sensor->getTemperature()) +
                    "��C, ʪ�� " + std::to_string(sensor->getHumidity()) + "%����ʪ�ȳ��ޣ�(����)", sensor->getId());

                // ���Կ����յ�����
                for (Device* acDevice_ptr : devices) {
                    if (AC* ac = dynamic_cast<AC*>(acDevice_ptr)) {
                        if (ac->getMode() == ACMode::OFF || ac->getMode() == ACMode::HEAT) {
                            std::cout << "����Ϊ������ " << sensor->getName() << " �ĸ��¾��������յ� " << ac->getName()
                                << " (ID: " << ac->getId() << ") ������Ϊ����ģʽ��Ŀ���¶�22.0��C��" << std::endl;
                            logger.INFO("����Ϊ������ " + sensor->getName() + " �ĸ��¾��������յ� " + ac->getName() +
                                " (ID: " + std::to_string(ac->getId()) + ") ������Ϊ����ģʽ��Ŀ���¶�22.0��C��", ac->getId());
                            ac->setMode(ACMode::COOL);
                            ac->setTargetTemperature(22.0); // ����һ�����ʵ������¶�
                        }
                        else if (ac->getMode() == ACMode::COOL) {
                            std::cout << "�յ� " << ac->getName() << " �Ѵ�������ģʽ��" << std::endl;
                            logger.INFO("�յ� " + ac->getName() + " �Ѵ�������ģʽ��", ac->getId());
                        }
                    }
                }
            }
            // ������
            else if (sensor->getTemperature() < 24) {
                condition_met_low = true;
                std::cout << "����: ������ " << sensor->getName() << " (ID: " << sensor->getId()
                    << ") ��⵽�¶� " << sensor->getTemperature() << "��C���¶ȹ��ͣ�" << std::endl;
                logger.ALERT("����: ������ " + sensor->getName() + " (ID: " + std::to_string(sensor->getId()) +
                    ") ��⵽�¶� " + std::to_string(sensor->getTemperature()) + "��C���¶ȹ��ͣ�", sensor->getId());

                // ���Կ����յ�����
                for (Device* acDevice_ptr : devices) {
                    if (AC* ac = dynamic_cast<AC*>(acDevice_ptr)) {
                        if (ac->getMode() == ACMode::OFF || ac->getMode() == ACMode::COOL) {
                            std::cout << "����Ϊ������ " << sensor->getName() << " �ĵ��¾��������յ� " << ac->getName()
                                << " (ID: " << ac->getId() << ") ������Ϊ����ģʽ��Ŀ���¶�26.0��C��" << std::endl;
                            logger.INFO("����Ϊ������ " + sensor->getName() + " �ĵ��¾��������յ� " + ac->getName() +
                                " (ID: " + std::to_string(ac->getId()) + ") ������Ϊ����ģʽ��Ŀ���¶�26.0��C��", ac->getId());
                            ac->setMode(ACMode::HEAT);
                            ac->setTargetTemperature(26.0); // ����һ�����ʵ������¶�
                        }
                        else if (ac->getMode() == ACMode::HEAT) {
                            std::cout << "�յ� " << ac->getName() << " �Ѵ�������ģʽ��" << std::endl;
                            logger.INFO("�յ� " + ac->getName() + " �Ѵ�������ģʽ��", ac->getId());
                        }
                    }
                }
            }
        }
    }

    if (!condition_met_high && !condition_met_low) {
        std::cout << "��ʪ�ȼ��������δ�����յ�������" << std::endl;
        logger.INFO("��ʪ�ȼ��������δ�����յ�������", -1, std::this_thread::get_id());
    }
}
void User::runFireEmergencyRule(double co2FireThreshold) {
    logger.INFO("�û� '" + username + "' ����ִ�л���������������...", -1, std::this_thread::get_id());
    std::cout << "�û�" << username << "����ִ�л���������������..." << std::endl;
    bool fire_detected = false;
    for (Device* device_ptr : devices) {
        if (Sensor* sensor = dynamic_cast<Sensor*>(device_ptr)) {
            if (sensor->getCO2Concentration() > co2FireThreshold) {
                fire_detected = true;
                logger.ALERT("���ؾ���: ������ " + sensor->getName() + " (ID: " + std::to_string(sensor->getId()) +
                    ") ��⵽CO2Ũ��Ϊ " + std::to_string(sensor->getCO2Concentration() * 100) +
                    "%�����ܷ������飡����ִ�н�����ʩ��", sensor->getId());
                std::cout << "���ؾ���: ������ " << sensor->getName() << " (ID: " << sensor->getId()
                    << ") ��⵽CO2Ũ��Ϊ " << sensor->getCO2Concentration() * 100
                    << "%�����ܷ������飡����ִ�н�����ʩ��" << std::endl;
                // �ر����еƾߺͿյ��������Թرշǹؼ��豸��Դ
                for (Device* devToShutdown : devices) {
                    if (Light* light = dynamic_cast<Light*>(devToShutdown)) {
                        if (light->getIsOn()) {
                            light->turnOff();
                            logger.INFO("�𾯽��������رյƾ� " + light->getName() + " (ID: " + std::to_string(light->getId()) + ")��", light->getId());
                            std::cout << "�𾯽��������رյƾ� " << light->getName() << " (ID: " << std::to_string(light->getId()) << ")��" << light->getId() << std::endl;
                        }
                    }
                    else if (AC* ac = dynamic_cast<AC*>(devToShutdown)) {
                        if (ac->getMode() != ACMode::OFF) {
                            ac->setMode(ACMode::OFF);
                            logger.INFO("�𾯽��������رտյ� " + ac->getName() + " (ID: " + std::to_string(ac->getId()) + ")��", ac->getId());
                            std::cout<< "�𾯽��������رտյ� "<<ac->getName() << " (ID: " << std::to_string(ac->getId()) << ")��"<< ac->getId() <<std::endl;
                        }
                    }
                    // �Է�Σ���豸ִ�н����ϵ�
                    if (devToShutdown->getImportance() != DeviceImportance::CRITICAL) {
                        if (!devToShutdown->isEmergencyPowerOff()) { // �����ظ�����
                            devToShutdown->setEmergencyPowerOff(true);
                            logger.INFO("�𾯽��������Է�Σ���豸 " + devToShutdown->getName() +
                                " (ID: " + std::to_string(devToShutdown->getId()) + ") ִ�н����ϵ硣", devToShutdown->getId());
                            std::cout << "�𾯽��������Է�Σ���豸 " << devToShutdown->getName() << " (ID: " << std::to_string(devToShutdown->getId()) << ") ִ�н����ϵ硣" << devToShutdown->getId() << std::endl;
                        }
                    }
                    else {
                        logger.INFO("�𾯽��������豸 " + devToShutdown->getName() +
                            " (ID: " + std::to_string(devToShutdown->getId()) + ") ΪΣ���豸����ִ���Զ��ϵ硣", devToShutdown->getId());
                        std::cout << "�𾯽��������豸 " << devToShutdown->getName()
                            << " (ID: " << devToShutdown->getId() << ") ΪΣ���豸����ִ���Զ��ϵ硣" << std::endl;
                    }
                }
            }
        }
    }
    if (!fire_detected) {
        logger.INFO("CO2Ũ�ȼ��������δ���������������", -1, std::this_thread::get_id());
        std::cout << "CO2Ũ�ȼ��������δ���������������" << std::endl;
    }
}

