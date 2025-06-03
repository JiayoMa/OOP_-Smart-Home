// main.cpp
#include <iostream>
#include <vector>
#include <string>
#include <limits>    // 为了 std::numeric_limits
#include <algorithm> // 为了 std::tolower
#include <fstream>
#include <sstream>
#include <cctype>    // 为了 std::tolower
#include <thread>    // 为了多线程
#include <mutex>     // 为了互斥锁
#include <chrono>    // 为了 std::chrono::seconds

// 智能家居系统头文件
#include "DeviceParams.h" 
#include "DeviceContainer.h"
#include "User.h"
#include "DeviceFactory.h" // 包含设备工厂
#include "SensorFactory.h"
#include "UserManager.h"
#include "SmartLogger.h"
#include "CustomExceptions.h"
#include "AC.h" // 为了 ACMode, FanSpeed 枚举 (如果它们在handleAddDeviceFromKeyboard中直接使用)


// 全局日志记录器实例
// 可以选择 FileLogger 或 ConsoleLogger
// SmartLogger gLogger(new ConsoleLogger(), LogLevel::DEBUG); 
SmartLogger gLogger(new FileLogger("Log.txt"), LogLevel::DEBUG);

// 当前登录用户
User* currentUser = nullptr;

// 多线程相关
std::mutex deviceContainerMutex; // 保护对DeviceContainer的并发访问
bool g_isRunning = true;         // 全局运行标志，用于通知线程停止

// 前向声明菜单处理函数
void printMainMenu(const User* user);
bool initialAuthAndLogin(UserManager& userManager, User*& targetUser);
void handleAddDeviceFromKeyboard(DeviceContainer& container, const User* currentUser); // 添加currentUser以进行权限检查
void handleSimulateScenes(User* user); // User 内部有设备列表引用
void handleChangeUser(UserManager& userManager, User*& targetUser);
void handleUpdateDevice(DeviceContainer& container, const User* currentUser);


// 环境更新线程函数 (多线程场景模拟 - Req 4.2)
void environmentUpdateRoutine(DeviceContainer& container) {
    srand(static_cast<unsigned int>(time(0))); // 初始化随机数种子
    gLogger.INFO("环境更新线程已启动。", -1, std::this_thread::get_id());
    while (g_isRunning) {
        // 实际应用中，这里会从文件或网络读取环境数据
        // 为演示，这里仅模拟随机变化
        std::this_thread::sleep_for(std::chrono::seconds(15)); // 每15秒更新一次
        if (!g_isRunning) break;

        { // 临界区开始
            std::lock_guard<std::mutex> lock(deviceContainerMutex);
            gLogger.DEBUG("环境更新线程：模拟环境变化...", -1, std::this_thread::get_id());

            bool sensorUpdated = false;
            for (Device* dev_ptr : container.getAllDevicePtrs()) {
                if (Sensor* sensor = dynamic_cast<Sensor*>(dev_ptr)) {
                    double oldTemp = sensor->getTemperature();
                    double newTemp = oldTemp + (rand() % 20 - 10) / 10.0; // 温度变化 -1.0 到 +0.9 °C
                    sensor->setTemperature(newTemp);

                    double oldHum = sensor->getHumidity();
                    double newHum = oldHum + (rand() % 20 - 10) / 5.0; // 湿度变化 -2.0% 到 +1.8%
                    sensor->setHumidity(newHum);

                    double oldCO2 = sensor->getCO2Concentration();
                    double newCO2 = oldCO2 + (rand() % 20 - 10) * 0.0005; // CO2变化 -0.005 到 +0.0045 (0.5% 到 0.45%)
                    sensor->setCO2Concentration(newCO2);

                    gLogger.INFO("环境更新：传感器 " + sensor->getName() +
                        " 更新为 温度=" + std::to_string(newTemp) + "°C, 湿度=" +
                        std::to_string(newHum) + "%, CO2=" + std::to_string(newCO2 * 100) + "%",
                        sensor->getId(), std::this_thread::get_id());
                    sensorUpdated = true;
                    // 通常只会更新一个或几个特定传感器，而不是所有传感器都随机变
                    // 这里仅为演示，所以更新找到的第一个传感器并跳出
                    // break; 
                }
            }
            if (!sensorUpdated) {
                gLogger.DEBUG("环境更新线程：未找到传感器进行模拟更新。", -1, std::this_thread::get_id());
            }
            // 更新环境后，可以触发规则检查。这里简化处理，规则检查由用户手动触发或在其他地方。
            // if (currentUser && sensorUpdated) { // 如果有用户登录且传感器数据变化
            //     currentUser->runTemperatureHumidityRule();
            //     currentUser->runFireEmergencyRule();
            // }

        } // 临界区结束
    }
    gLogger.INFO("环境更新线程已停止。", -1, std::this_thread::get_id());
}


int main() {
    // 设置控制台输出中文（如果需要，特定于Windows）
    // #ifdef _WIN32
    // system("chcp 65001 > nul"); // 设置控制台为UTF-8编码，可能需要管理员权限或对字体有要求
    // #endif

    gLogger.INFO("智能家居控制系统启动中...", -1, std::this_thread::get_id());

    DeviceContainer deviceContainer(gLogger);
    UserManager userManager("user.txt", deviceContainer.getAllDevicePtrs(), gLogger);

    // --- 初始用户认证 ---
    if (!initialAuthAndLogin(userManager, currentUser)) {
        gLogger.INFO("用户认证失败或选择退出。系统即将关闭。", -1, std::this_thread::get_id());
        return 0;
    }
    if (currentUser) {
        gLogger.INFO("用户 '" + currentUser->getUsername() + "' 登录成功", -1, std::this_thread::get_id());
    }
    deviceContainer.importDevicesFromFileLogOnly("devices.txt");

    // --- 启动环境更新线程 ---
    std::thread envUpdateThread(environmentUpdateRoutine, std::ref(deviceContainer));


    // --- 主循环 ---
    char choice_char;
    do {
        printMainMenu(currentUser);
        std::cin >> choice_char;

        if (std::cin.peek() == '\n') { std::cin.ignore(); }
        else if (std::cin.fail() || std::cin.eof()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            gLogger.ALERT("检测到无效输入。请重试。", -1, std::this_thread::get_id());
            std::cout << "输入无效，请重新选择菜单项。" << std::endl;
            continue;
        }

        char lower_choice = std::tolower(choice_char);

        if (lower_choice == 'q') {
            gLogger.INFO("用户选择退出。正在保存设备信息并关闭系统...", -1, std::this_thread::get_id());
            try {
                std::lock_guard<std::mutex> lock(deviceContainerMutex);
                deviceContainer.saveDevicesToFile("devices.txt");
            }
            catch (const BaseSmartHomeException& e) {
                gLogger.ALERT("退出时保存设备失败：" + std::string(e.what()), -1, std::this_thread::get_id());
                std::cerr << "错误：保存设备时发生问题 - " << e.what() << std::endl;
            }
            break;
        }

        int choice_int = -1;
        if (lower_choice >= '0' && lower_choice <= '9') {
            choice_int = lower_choice - '0';
        }
        else {
            gLogger.ALERT("无效的菜单选择字符：" + std::string(1, choice_char), -1, std::this_thread::get_id());
            std::cout << "无效选择。请输入菜单中的数字或 'Q' 退出。" << std::endl;
            continue;
        }

        try {
            
            std::lock_guard<std::mutex> lock(deviceContainerMutex);

            switch (choice_int) {
            case 0:
                handleChangeUser(userManager, currentUser);
                break;
            case 1:
                if (currentUser) {
                    gLogger.INFO("正在显示用户 '" + currentUser->getUsername() + "' 的信息。", -1, std::this_thread::get_id());
                    std::cout << "\n--- 当前用户信息 ---" << std::endl;
                    std::cout << "用户名: " << currentUser->getUsername() << std::endl;
                    std::cout << "用户角色: " << roleToString(currentUser->getRole()) << std::endl;
                }
                else {
                    gLogger.INFO("当前没有用户登录。", -1, std::this_thread::get_id());
                    std::cout << "当前没有用户登录。" << std::endl;
                }
                break;
            case 2:
                deviceContainer.importDevicesFromFile("device_cn.txt");
                std::cout << "设备导入过程已完成。详情请查看日志。" << std::endl;
                break;
            case 3:
                handleAddDeviceFromKeyboard(deviceContainer, currentUser);
                break;
            case 4:
                deviceContainer.displayAllDevices();
                break;
            case 5:
                if (!currentUser) { gLogger.ALERT("此操作需要登录。"); std::cout << "请先登录。\n"; break; }
                std::cout << "请输入要查找的设备ID：";
                int findId;
                std::cin >> findId;
                if (std::cin.fail()) { std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); std::cout << "ID输入无效。\n"; break; }
                deviceContainer.displayDeviceDetails(findId);
                break;
            case 6:
                if (!currentUser) { gLogger.ALERT("此操作需要登录。"); std::cout << "请先登录。\n"; break; }
                std::cout << "请输入要删除的设备ID：";
                int deleteId;
                std::cin >> deleteId;
                if (std::cin.fail()) { std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); std::cout << "ID输入无效。\n"; break; }
                deviceContainer.deleteDeviceById(deleteId, *currentUser); // 现在需要传入 currentUser
                std::cout << "尝试删除设备ID " << deleteId << "。详情请查看日志。" << std::endl;
                break;
            case 7:
                if (!currentUser) { gLogger.ALERT("此操作需要登录。"); std::cout << "请先登录。\n"; break; }
                deviceContainer.saveDevicesToFile("devices.txt");
                std::cout << "设备保存过程已完成。详情请查看日志。" << std::endl;
                break;
            case 8:
                if (!currentUser) {
                    gLogger.ALERT("模拟场景需要用户登录。");
                    std::cout << "请先登录以模拟智能场景。\n";
                    break;
                }
                handleSimulateScenes(currentUser);
                break;
            case 9: // 更新设备 (示例)
                handleUpdateDevice(deviceContainer, currentUser);
                break;
                // case 9 was sort before, changing to update. Add sort menu if needed.

            default:
                gLogger.ALERT("无效的菜单选项整数：" + std::to_string(choice_int), -1, std::this_thread::get_id());
                std::cout << "该选项不可用。" << std::endl;
                break;
            }
        }
        catch (const PermissionDeniedException& e) {
            gLogger.ALERT("权限错误：" + std::string(e.what()), (currentUser ? -1 : -1), std::this_thread::get_id());
            std::cerr << "错误： " << e.what() << std::endl;
        }
        catch (const DeviceNotFoundException& e) {
            gLogger.ALERT("设备未找到错误：" + std::string(e.what()), -1, std::this_thread::get_id());
            std::cerr << "错误： " << e.what() << std::endl;
        }
        catch (const InvalidParameterException& e) {
            gLogger.ALERT("无效参数错误：" + std::string(e.what()), -1, std::this_thread::get_id());
            std::cerr << "错误： " << e.what() << std::endl;
        }
        catch (const FactoryNotFoundException& e) {
            gLogger.ALERT("工厂错误：" + std::string(e.what()), -1, std::this_thread::get_id());
            std::cerr << "错误： " << e.what() << std::endl;
        }
        catch (const BaseSmartHomeException& e) {
            gLogger.ALERT("智能家居系统错误：" + std::string(e.what()), -1, std::this_thread::get_id());
            std::cerr << "系统错误： " << e.what() << std::endl;
        }
        catch (const std::exception& e) {
            gLogger.ALERT("标准库异常：" + std::string(e.what()), -1, std::this_thread::get_id());
            std::cerr << "发生意外错误： " << e.what() << std::endl;
        }


        if (lower_choice != 'q') {
            std::cout << "\n按 Enter键 返回主菜单...";
            // std::cin.get(); // 可能会消耗掉上次 cin >> choice 留下的换行符
            // 一个更健壮的方法，用于混合使用 cin >> var 和后续可能的 getline
            std::string dummy;
            // 如果上一个输入是 cin >> var，则缓冲区中可能有一个换行符。
            // peek() 检查下一个字符。如果是换行符，则用 ignore() 消耗它。
            if (std::cin.peek() == '\n') {
                std::cin.ignore();
            }
            std::getline(std::cin, dummy); // 等待用户按 Enter
        }

    } while (true);

    // --- 系统关闭 ---
    gLogger.INFO("智能家居系统正在关闭...", -1, std::this_thread::get_id());
    g_isRunning = false; // 通知环境更新线程停止
    if (envUpdateThread.joinable()) {
        envUpdateThread.join();
        gLogger.INFO("环境更新线程已成功汇合。", -1, std::this_thread::get_id());
    }

    gLogger.INFO("智能家居系统已关闭。", -1, std::this_thread::get_id());
    return 0;
}

// --- 主菜单辅助函数实现 ---
void printMainMenu(const User* user) {
    std::cout << "\n=========== 智能家居主菜单 ===========\n";
    if (user) {
        std::cout << "当前用户: " << user->getUsername()
            << " (角色: " << roleToString(user->getRole()) << ")\n";
    }
    else {
        std::cout << "当前无用户登录。\n";
    }
    std::cout << "--------------------------------------------\n";
    std::cout << "0 ---- 切换用户 / 注册新用户\n";
    std::cout << "1 ---- 显示当前用户信息\n";
    std::cout << "2 ---- 从文件导入设备\n";
    std::cout << "3 ---- 从键盘添加设备\n";
    std::cout << "4 ---- 列表显示当前所有设备\n";
    std::cout << "5 ---- 按指定ID查找并显示设备\n";
    std::cout << "6 ---- 删除指定ID的设备 (管理员权限)\n";
    std::cout << "7 ---- 保存所有设备信息至文件\n";
    std::cout << "8 ---- 智能场景模拟\n";
    std::cout << "9 ---- 更新设备信息 (管理员权限)\n";
    std::cout << "Q ---- 退出系统\n";
    std::cout << "============================================\n";
    std::cout << "请输入您的选择：";
}


bool initialAuthAndLogin(UserManager& userManager, User*& targetUser) {
    while (targetUser == nullptr) {
        std::cout << "\n=========== 用户认证 ===========\n";
        std::cout << "1. 用户登录\n";
        std::cout << "2. 注册新用户\n";
        std::cout << "3. 退出系统\n";
        std::cout << "================================\n";
        std::cout << "请选择操作：";
        char authChoice_char;
        std::cin >> authChoice_char;
        // 清理输入缓冲区，防止影响后续的 getline
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::string username, password;
        switch (authChoice_char) {
        case '1':
            std::cout << "请输入用户名："; std::getline(std::cin, username);
            std::cout << "请输入密码："; std::getline(std::cin, password);
            targetUser = userManager.loginUser(username, password);
            if (targetUser) {
                // userManager.loginUser 内部已记录日志
                std::cout << "登录成功！欢迎您，" << targetUser->getUsername() << "。" << std::endl;
                return true;
            }
            else {
                std::cout << "登录失败，请检查您的用户名和密码是否正确。" << std::endl;
            }
            break;
        case '2':
            std::cout << "--- 注册新用户 ---\n";
            std::cout << "请输入新用户名："; std::getline(std::cin, username);
            std::cout << "请输入密码："; std::getline(std::cin, password);
            targetUser = userManager.registerUser(username, password, UserRole::USER); // 默认注册为普通用户
            if (targetUser) {
                // userManager.registerUser 内部已记录日志
                std::cout << "注册成功！您已自动登录，" << targetUser->getUsername() << "。" << std::endl;
                return true;
            }
            else {
                std::cout << "注册失败。可能是用户名已存在或输入无效。" << std::endl;
            }
            break;
        case '3':
            gLogger.INFO("用户在认证阶段选择退出系统。", -1, std::this_thread::get_id());
            std::cout << "您选择了退出系统。" << std::endl;
            return false;
        default:
            gLogger.ALERT("无效的认证选项：" + std::string(1, authChoice_char), -1, std::this_thread::get_id());
            std::cout << "无效的选项，请重新输入。" << std::endl;
            break;
        }
    }
    return false; // 理论上循环会处理，为编译器满意
}

void handleChangeUser(UserManager& userManager, User*& targetUser) {
    if (targetUser) {
        gLogger.INFO("用户 '" + targetUser->getUsername() + "' 已注销。", -1, std::this_thread::get_id());
        std::cout << "用户 '" << targetUser->getUsername() << "' 已注销。" << std::endl;
        targetUser = nullptr;
    }
    if (!initialAuthAndLogin(userManager, targetUser)) {
        gLogger.INFO("在切换用户过程中，用户认证失败或选择退出。", -1, std::this_thread::get_id());
        // 不在此处退出整个程序，仅将 targetUser 置为 nullptr，主循环将继续
        // 如果需要在此处退出，可以抛出特定异常或设置全局退出标志
        std::cout << "未能完成用户认证。" << std::endl;
        // g_isRunning = false; // 如果需要退出整个程序
    }
    else {
        if (targetUser) {
            gLogger.INFO("用户 '" + targetUser->getUsername() + "' 现已登录。", -1, std::this_thread::get_id());
        }
    }
}


void handleAddDeviceFromKeyboard(DeviceContainer& container, const User* currentUser) {
    if (!currentUser) {
        gLogger.ALERT("添加设备需要用户登录。");
        std::cout << "请先登录才能添加设备。" << std::endl;
        return;
    }
    // 权限检查：例如，只有管理员可以添加某些类型的设备，或所有设备
    // if (currentUser->getRole() != UserRole::ADMIN) {
    //     gLogger.ALERT("权限不足：用户 '" + currentUser->getUsername() + "' 尝试从键盘添加设备。");
    //     std::cout << "权限不足，无法添加设备。" << std::endl;
    //     throw PermissionDeniedException("只有管理员才能从键盘添加设备。"); // 或者不抛异常，仅提示
    //     return;
    // }


    std::cout << "\n--- 从键盘添加新设备 ---\n";
    DeviceParams params;
    DeviceType typeEnum;

    std::cout << "请选择设备类型 (1: 环境传感器, 2: 智能灯具, 3: 空调): ";
    int typeChoice;
    std::cin >> typeChoice;
    if (std::cin.fail() || typeChoice < 1 || typeChoice > 3) {
        gLogger.ALERT("从键盘添加设备时选择了无效的设备类型。");
        std::cout << "无效的设备类型。操作中止。" << std::endl;
        std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "请输入设备ID (整数): "; std::cin >> params.id;
    if (std::cin.fail() || container.findDeviceById(params.id) != nullptr) {
        gLogger.ALERT("无效的设备ID或ID " + std::to_string(params.id) + " 已存在。", params.id);
        std::cout << "ID输入无效或该ID已存在。操作中止。" << std::endl;
        std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "请输入设备名称: "; std::getline(std::cin, params.name);
    if (params.name.empty()) params.name = "未命名设备";

    std::cout << "请输入设备重要程度 (0:低, 1:中, 2:高, 3:危急): ";
    int impChoice; std::cin >> impChoice;
    params.importance = stringToImportance(std::to_string(impChoice)); // stringToImportance处理无效输入

    std::cout << "请输入设备功耗 (瓦特): "; std::cin >> params.powerConsumption;
    if (std::cin.fail()) { /* 错误处理 */ std::cout << "功耗输入无效。" << std::endl; std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); return; }


    std::cout << "请输入设备位置: ";
    // 如果上一个输入是 cin >> var，这里需要 consume newline
    if (std::cin.peek() == '\n') { std::cin.ignore(); }
    std::getline(std::cin, params.location);
    if (params.location.empty()) params.location = "未知位置";


    try {
        switch (typeChoice) {
        case 1:
            typeEnum = DeviceType::SENSOR;
            std::cout << "请输入初始温度 (摄氏度): "; std::cin >> params.temperature;
            std::cout << "请输入初始湿度 (%): "; std::cin >> params.humidity;
            std::cout << "请输入初始CO2浓度 (例如, 0.04 代表 4%): "; std::cin >> params.co2Concentration;
            break;
        case 2:
            typeEnum = DeviceType::LIGHT;
            std::cout << "灯具是否初始开启? (1 代表是, 0 代表否): ";
            int isOnChoice; std::cin >> isOnChoice; params.isOn = (isOnChoice == 1);
            if (params.isOn) {
                std::cout << "请输入初始亮度 (0-100%): "; std::cin >> params.brightness;
            }
            else {
                params.brightness = 0;
            }
            break;
        case 3:
            typeEnum = DeviceType::AC;
            int modeChoice, fanChoice;
            // 注意：这里的数字需要与AC.h/cpp中定义的枚举和转换函数对应
            std::cout << "请输入空调模式 (0:制冷, 1:制热, 2:送风, 3:关闭): "; std::cin >> modeChoice;
            if (modeChoice >= 0 && modeChoice <= 3) params.acMode = static_cast<ACMode>(modeChoice);
            else { std::cout << "模式选择无效，默认为关闭。\n"; params.acMode = ACMode::OFF; }


            if (params.acMode != ACMode::OFF) {
                std::cout << "请输入目标温度 (摄氏度): "; std::cin >> params.targetTemperature;
                std::cout << "请输入风速 (0:低风, 1:中风, 2:高风, 3:自动): "; std::cin >> fanChoice;
                if (fanChoice >= 0 && fanChoice <= 3) params.fanSpeed = static_cast<FanSpeed>(fanChoice);
                else { std::cout << "风速选择无效，默认为自动。\n"; params.fanSpeed = FanSpeed::AUTO; }
            }
            else {
                params.targetTemperature = 25;
                params.fanSpeed = FanSpeed::AUTO;
            }
            break;
        }
        // 确保在 switch case 块的末尾或循环的下一次迭代之前，任何未被 getline 清理的换行符都被处理
        if (std::cin.peek() == '\n') { std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); }
        else { std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); }


        container.addDeviceFromParams(typeEnum, params);
        std::cout << "设备 '" << params.name << "' (ID: " << params.id << ") 已成功创建。" << std::endl;

    }
    catch (const BaseSmartHomeException& e) {
        gLogger.ALERT("从键盘添加设备失败：" + std::string(e.what()), params.id, std::this_thread::get_id());
        std::cerr << "添加设备时发生错误: " << e.what() << std::endl;
        if (std::cin.fail()) { std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); }
    }
    catch (const std::exception& e) {
        gLogger.ALERT("从键盘添加设备时发生标准库异常：" + std::string(e.what()), params.id, std::this_thread::get_id());
        std::cerr << "发生意外错误: " << e.what() << std::endl;
        if (std::cin.fail()) { std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); }
    }
}

void handleUpdateDevice(DeviceContainer& container, const User* currentUser) {
    if (!currentUser) {
        gLogger.ALERT("更新设备需要用户登录。");
        std::cout << "请先登录才能更新设备。" << std::endl;
        return;
    }
    // 权限检查已移至 DeviceContainer::updateDevice

    std::cout << "请输入要更新的设备ID：";
    int idToUpdate;
    std::cin >> idToUpdate;
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "ID输入无效。\n";
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    const Device* existingDevice = container.findDeviceById(idToUpdate);
    if (!existingDevice) {
        std::cout << "未找到ID为 " << idToUpdate << " 的设备。" << std::endl;
        return;
    }

    std::cout << "找到设备：" << existingDevice->getName() << "。请输入新的设备参数：" << std::endl;
    DeviceParams params;
    params.id = idToUpdate; // 默认ID保持不变，除非用户输入新的

    std::cout << "设备ID (如果不变则输入原ID " << idToUpdate << "): "; std::cin >> params.id;
    if (std::cin.fail()) { /* ... ID输入错误处理 ... */ return; }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "设备名称 (原: " << existingDevice->getName() << "): "; std::getline(std::cin, params.name);
    if (params.name.empty()) params.name = existingDevice->getName(); // 如果不输入则保持原样

    std::cout << "重要程度 (0:低, 1:中, 2:高, 3:危急, 原: " << static_cast<int>(existingDevice->getImportance()) << "): ";
    int impChoice; std::cin >> impChoice; params.importance = stringToImportance(std::to_string(impChoice));

    std::cout << "功耗 (原: " << existingDevice->getPowerConsumption() << "): "; std::cin >> params.powerConsumption;
    if (std::cin.fail()) { /* ...功耗输入错误处理 ... */ return; }

    std::cout << "位置 (原: " << existingDevice->getLocation() << "): ";
    if (std::cin.peek() == '\n') { std::cin.ignore(); }
    std::getline(std::cin, params.location);
    if (params.location.empty()) params.location = existingDevice->getLocation();

    // 根据设备类型获取特定参数
    if (const Sensor* sensor = dynamic_cast<const Sensor*>(existingDevice)) {
        std::cout << "温度 (原: " << sensor->getTemperature() << "): "; std::cin >> params.temperature;
        std::cout << "湿度 (原: " << sensor->getHumidity() << "): "; std::cin >> params.humidity;
        std::cout << "CO2浓度 (原: " << sensor->getCO2Concentration() << "): "; std::cin >> params.co2Concentration;
    }
    else if (const Light* light = dynamic_cast<const Light*>(existingDevice)) {
        std::cout << "灯是否开启 (1是/0否, 原: " << light->getIsOn() << "): "; int on; std::cin >> on; params.isOn = (on == 1);
        std::cout << "亮度 (原: " << light->getBrightness() << "): "; std::cin >> params.brightness;
    }
    else if (const AC* ac = dynamic_cast<const AC*>(existingDevice)) {
        std::cout << "模式 (0:制冷,1:制热,2:送风,3:关闭, 原: " << static_cast<int>(ac->getMode()) << "): "; int mode; std::cin >> mode; params.acMode = static_cast<ACMode>(mode);
        std::cout << "目标温度 (原: " << ac->getTargetTemperature() << "): "; std::cin >> params.targetTemperature;
        std::cout << "风速 (0:低,1:中,2:高,3:自动, 原: " << static_cast<int>(ac->getFanSpeed()) << "): "; int speed; std::cin >> speed; params.fanSpeed = static_cast<FanSpeed>(speed);
    }
    if (std::cin.peek() == '\n') { std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); }
    else { std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); }


    container.updateDevice(idToUpdate, params, *currentUser); // updateDevice 内部会进行权限检查和日志记录
    std::cout << "尝试更新设备ID " << idToUpdate << "。详情请查看日志。" << std::endl;
}


void handleSimulateScenes(User* user) {
    if (!user) {
        gLogger.ALERT("模拟场景需要用户登录。", -1, std::this_thread::get_id());
        std::cout << "请先登录以模拟智能场景。" << std::endl;
        return;
    }
    // User对象内部有对设备列表的引用，可以直接调用其场景方法

    std::cout << "\n--- 智能场景模拟 ---" << std::endl;
    std::cout << "1. 模拟温湿度超限 (空调启动)" << std::endl;
    std::cout << "2. 模拟火灾检测 (CO2超标，尝试关闭电器)" << std::endl;
    std::cout << "请选择要模拟的场景：";
    int sceneChoice;
    std::cin >> sceneChoice;

    if (std::cin.fail()) {
        gLogger.ALERT("场景选择输入无效。", -1, std::this_thread::get_id());
        std::cout << "输入无效。" << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 清理换行符

    switch (sceneChoice) {
    case 1:
        gLogger.INFO("用户 '" + user->getUsername() + "' 触发温湿度超限场景模拟。", -1, std::this_thread::get_id());
        user->runTemperatureHumidityRule();
        std::cout << "温湿度超限场景模拟执行完毕。详情请查看日志。" << std::endl;
        break;
    case 2:
        gLogger.INFO("用户 '" + user->getUsername() + "' 触发火灾检测场景模拟。", -1, std::this_thread::get_id());
        user->runFireEmergencyRule();
        std::cout << "火灾检测场景模拟执行完毕。详情请查看日志。" << std::endl;
        break;
    default:
        gLogger.ALERT("无效的场景选择：" + std::to_string(sceneChoice), -1, std::this_thread::get_id());
        std::cout << "无效的场景选择。" << std::endl;
        break;
    }
}