// User.cpp
#include "User.h"
#include "Sensor.h" // 为了 dynamic_cast
#include "AC.h"     // 为了 dynamic_cast
#include "Light.h"  // 为了 dynamic_cast
#include <iostream> // 为了场景模拟中的输出 (但优先使用logger)

User::User(const std::string& name, const std::string& pwd, UserRole r,
    std::vector<Device*>& deviceList, SmartLogger& loggerRef)
    : username(name), password(pwd), role(r), devices(deviceList), logger(loggerRef) {
    logger.INFO("用户 '" + username + "' 已创建。", -1);
}

std::string User::getUsername() const {
    return username;
}

std::string User::getPassword() const {
    // 警告: 在实际系统中不应直接返回明文密码
    return password;
}

UserRole User::getRole() const {
    return role;
}

const std::vector<Device*>& User::getDevices() const {
    return devices;
}
void User::runTemperatureHumidityRule(double tempThreshold,double humidityThresholdHigh ) {
    std::cout << "用户 '" << username << "' 正在执行温湿度超限规则检测..." << std::endl;
    logger.INFO("用户 '" + username + "' 正在执行温湿度超限规则检测...", -1, std::this_thread::get_id());
    bool condition_met_high = false;
    bool condition_met_low = false;

    for (Device* device_ptr : devices) {
        if (Sensor* sensor = dynamic_cast<Sensor*>(device_ptr)) {
            // 检测高温高湿
            if (sensor->getTemperature() > tempThreshold || sensor->getHumidity() > humidityThresholdHigh) {
                condition_met_high = true;
                std::cout << "警报: 传感器 " << sensor->getName() << " (ID: " << sensor->getId()
                    << ") 检测到温度 " << sensor->getTemperature() << "°C, 湿度 " << sensor->getHumidity()
                    << "%。温湿度超限！(过高)" << std::endl;
                logger.ALERT("警报: 传感器 " + sensor->getName() + " (ID: " + std::to_string(sensor->getId()) +
                    ") 检测到温度 " + std::to_string(sensor->getTemperature()) +
                    "°C, 湿度 " + std::to_string(sensor->getHumidity()) + "%。温湿度超限！(过高)", sensor->getId());

                // 尝试开启空调制冷
                for (Device* acDevice_ptr : devices) {
                    if (AC* ac = dynamic_cast<AC*>(acDevice_ptr)) {
                        if (ac->getMode() == ACMode::OFF || ac->getMode() == ACMode::HEAT) {
                            std::cout << "正在为传感器 " << sensor->getName() << " 的高温警报开启空调 " << ac->getName()
                                << " (ID: " << ac->getId() << ") 并设置为制冷模式，目标温度22.0°C。" << std::endl;
                            logger.INFO("正在为传感器 " + sensor->getName() + " 的高温警报开启空调 " + ac->getName() +
                                " (ID: " + std::to_string(ac->getId()) + ") 并设置为制冷模式，目标温度22.0°C。", ac->getId());
                            ac->setMode(ACMode::COOL);
                            ac->setTargetTemperature(22.0); // 设置一个舒适的制冷温度
                        }
                        else if (ac->getMode() == ACMode::COOL) {
                            std::cout << "空调 " << ac->getName() << " 已处于制冷模式。" << std::endl;
                            logger.INFO("空调 " + ac->getName() + " 已处于制冷模式。", ac->getId());
                        }
                    }
                }
            }
            // 检测低温
            else if (sensor->getTemperature() < 24) {
                condition_met_low = true;
                std::cout << "警报: 传感器 " << sensor->getName() << " (ID: " << sensor->getId()
                    << ") 检测到温度 " << sensor->getTemperature() << "°C。温度过低！" << std::endl;
                logger.ALERT("警报: 传感器 " + sensor->getName() + " (ID: " + std::to_string(sensor->getId()) +
                    ") 检测到温度 " + std::to_string(sensor->getTemperature()) + "°C。温度过低！", sensor->getId());

                // 尝试开启空调制热
                for (Device* acDevice_ptr : devices) {
                    if (AC* ac = dynamic_cast<AC*>(acDevice_ptr)) {
                        if (ac->getMode() == ACMode::OFF || ac->getMode() == ACMode::COOL) {
                            std::cout << "正在为传感器 " << sensor->getName() << " 的低温警报开启空调 " << ac->getName()
                                << " (ID: " << ac->getId() << ") 并设置为制热模式，目标温度26.0°C。" << std::endl;
                            logger.INFO("正在为传感器 " + sensor->getName() + " 的低温警报开启空调 " + ac->getName() +
                                " (ID: " + std::to_string(ac->getId()) + ") 并设置为制热模式，目标温度26.0°C。", ac->getId());
                            ac->setMode(ACMode::HEAT);
                            ac->setTargetTemperature(26.0); // 设置一个舒适的制热温度
                        }
                        else if (ac->getMode() == ACMode::HEAT) {
                            std::cout << "空调 " << ac->getName() << " 已处于制热模式。" << std::endl;
                            logger.INFO("空调 " + ac->getName() + " 已处于制热模式。", ac->getId());
                        }
                    }
                }
            }
        }
    }

    if (!condition_met_high && !condition_met_low) {
        std::cout << "温湿度检测正常，未触发空调调整。" << std::endl;
        logger.INFO("温湿度检测正常，未触发空调调整。", -1, std::this_thread::get_id());
    }
}
void User::runFireEmergencyRule(double co2FireThreshold) {
    logger.INFO("用户 '" + username + "' 正在执行火情紧急处理规则检测...", -1, std::this_thread::get_id());
    std::cout << "用户" << username << "正在执行火情紧急处理规则检测..." << std::endl;
    bool fire_detected = false;
    for (Device* device_ptr : devices) {
        if (Sensor* sensor = dynamic_cast<Sensor*>(device_ptr)) {
            if (sensor->getCO2Concentration() > co2FireThreshold) {
                fire_detected = true;
                logger.ALERT("严重警报: 传感器 " + sensor->getName() + " (ID: " + std::to_string(sensor->getId()) +
                    ") 检测到CO2浓度为 " + std::to_string(sensor->getCO2Concentration() * 100) +
                    "%！可能发生火情！正在执行紧急措施！", sensor->getId());
                std::cout << "严重警报: 传感器 " << sensor->getName() << " (ID: " << sensor->getId()
                    << ") 检测到CO2浓度为 " << sensor->getCO2Concentration() * 100
                    << "%！可能发生火情！正在执行紧急措施！" << std::endl;
                // 关闭所有灯具和空调，并尝试关闭非关键设备电源
                for (Device* devToShutdown : devices) {
                    if (Light* light = dynamic_cast<Light*>(devToShutdown)) {
                        if (light->getIsOn()) {
                            light->turnOff();
                            logger.INFO("火警紧急处理：关闭灯具 " + light->getName() + " (ID: " + std::to_string(light->getId()) + ")。", light->getId());
                            std::cout << "火警紧急处理：关闭灯具 " << light->getName() << " (ID: " << std::to_string(light->getId()) << ")。" << light->getId() << std::endl;
                        }
                    }
                    else if (AC* ac = dynamic_cast<AC*>(devToShutdown)) {
                        if (ac->getMode() != ACMode::OFF) {
                            ac->setMode(ACMode::OFF);
                            logger.INFO("火警紧急处理：关闭空调 " + ac->getName() + " (ID: " + std::to_string(ac->getId()) + ")。", ac->getId());
                            std::cout<< "火警紧急处理：关闭空调 "<<ac->getName() << " (ID: " << std::to_string(ac->getId()) << ")。"<< ac->getId() <<std::endl;
                        }
                    }
                    // 对非危急设备执行紧急断电
                    if (devToShutdown->getImportance() != DeviceImportance::CRITICAL) {
                        if (!devToShutdown->isEmergencyPowerOff()) { // 避免重复操作
                            devToShutdown->setEmergencyPowerOff(true);
                            logger.INFO("火警紧急处理：对非危急设备 " + devToShutdown->getName() +
                                " (ID: " + std::to_string(devToShutdown->getId()) + ") 执行紧急断电。", devToShutdown->getId());
                            std::cout << "火警紧急处理：对非危急设备 " << devToShutdown->getName() << " (ID: " << std::to_string(devToShutdown->getId()) << ") 执行紧急断电。" << devToShutdown->getId() << std::endl;
                        }
                    }
                    else {
                        logger.INFO("火警紧急处理：设备 " + devToShutdown->getName() +
                            " (ID: " + std::to_string(devToShutdown->getId()) + ") 为危急设备，不执行自动断电。", devToShutdown->getId());
                        std::cout << "火警紧急处理：设备 " << devToShutdown->getName()
                            << " (ID: " << devToShutdown->getId() << ") 为危急设备，不执行自动断电。" << std::endl;
                    }
                }
            }
        }
    }
    if (!fire_detected) {
        logger.INFO("CO2浓度检测正常，未触发火情紧急处理。", -1, std::this_thread::get_id());
        std::cout << "CO2浓度检测正常，未触发火情紧急处理。" << std::endl;
    }
}

