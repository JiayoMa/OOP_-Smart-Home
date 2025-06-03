// main.cpp
#include <iostream>
#include <vector>
#include <string>
#include <limits>    // Ϊ�� std::numeric_limits
#include <algorithm> // Ϊ�� std::tolower
#include <fstream>
#include <sstream>
#include <cctype>    // Ϊ�� std::tolower
#include <thread>    // Ϊ�˶��߳�
#include <mutex>     // Ϊ�˻�����
#include <chrono>    // Ϊ�� std::chrono::seconds

// ���ܼҾ�ϵͳͷ�ļ�
#include "DeviceParams.h" 
#include "DeviceContainer.h"
#include "User.h"
#include "DeviceFactory.h" // �����豸����
#include "SensorFactory.h"
#include "UserManager.h"
#include "SmartLogger.h"
#include "CustomExceptions.h"
#include "AC.h" // Ϊ�� ACMode, FanSpeed ö�� (���������handleAddDeviceFromKeyboard��ֱ��ʹ��)


// ȫ����־��¼��ʵ��
// ����ѡ�� FileLogger �� ConsoleLogger
// SmartLogger gLogger(new ConsoleLogger(), LogLevel::DEBUG); 
SmartLogger gLogger(new FileLogger("Log.txt"), LogLevel::DEBUG);

// ��ǰ��¼�û�
User* currentUser = nullptr;

// ���߳����
std::mutex deviceContainerMutex; // ������DeviceContainer�Ĳ�������
bool g_isRunning = true;         // ȫ�����б�־������֪ͨ�߳�ֹͣ

// ǰ�������˵�������
void printMainMenu(const User* user);
bool initialAuthAndLogin(UserManager& userManager, User*& targetUser);
void handleAddDeviceFromKeyboard(DeviceContainer& container, const User* currentUser); // ���currentUser�Խ���Ȩ�޼��
void handleSimulateScenes(User* user); // User �ڲ����豸�б�����
void handleChangeUser(UserManager& userManager, User*& targetUser);
void handleUpdateDevice(DeviceContainer& container, const User* currentUser);


// ���������̺߳��� (���̳߳���ģ�� - Req 4.2)
void environmentUpdateRoutine(DeviceContainer& container) {
    srand(static_cast<unsigned int>(time(0))); // ��ʼ�����������
    gLogger.INFO("���������߳���������", -1, std::this_thread::get_id());
    while (g_isRunning) {
        // ʵ��Ӧ���У��������ļ��������ȡ��������
        // Ϊ��ʾ�������ģ������仯
        std::this_thread::sleep_for(std::chrono::seconds(15)); // ÿ15�����һ��
        if (!g_isRunning) break;

        { // �ٽ�����ʼ
            std::lock_guard<std::mutex> lock(deviceContainerMutex);
            gLogger.DEBUG("���������̣߳�ģ�⻷���仯...", -1, std::this_thread::get_id());

            bool sensorUpdated = false;
            for (Device* dev_ptr : container.getAllDevicePtrs()) {
                if (Sensor* sensor = dynamic_cast<Sensor*>(dev_ptr)) {
                    double oldTemp = sensor->getTemperature();
                    double newTemp = oldTemp + (rand() % 20 - 10) / 10.0; // �¶ȱ仯 -1.0 �� +0.9 ��C
                    sensor->setTemperature(newTemp);

                    double oldHum = sensor->getHumidity();
                    double newHum = oldHum + (rand() % 20 - 10) / 5.0; // ʪ�ȱ仯 -2.0% �� +1.8%
                    sensor->setHumidity(newHum);

                    double oldCO2 = sensor->getCO2Concentration();
                    double newCO2 = oldCO2 + (rand() % 20 - 10) * 0.0005; // CO2�仯 -0.005 �� +0.0045 (0.5% �� 0.45%)
                    sensor->setCO2Concentration(newCO2);

                    gLogger.INFO("�������£������� " + sensor->getName() +
                        " ����Ϊ �¶�=" + std::to_string(newTemp) + "��C, ʪ��=" +
                        std::to_string(newHum) + "%, CO2=" + std::to_string(newCO2 * 100) + "%",
                        sensor->getId(), std::this_thread::get_id());
                    sensorUpdated = true;
                    // ͨ��ֻ�����һ���򼸸��ض������������������д������������
                    // �����Ϊ��ʾ�����Ը����ҵ��ĵ�һ��������������
                    // break; 
                }
            }
            if (!sensorUpdated) {
                gLogger.DEBUG("���������̣߳�δ�ҵ�����������ģ����¡�", -1, std::this_thread::get_id());
            }
            // ���»����󣬿��Դ��������顣����򻯴������������û��ֶ��������������ط���
            // if (currentUser && sensorUpdated) { // ������û���¼�Ҵ��������ݱ仯
            //     currentUser->runTemperatureHumidityRule();
            //     currentUser->runFireEmergencyRule();
            // }

        } // �ٽ�������
    }
    gLogger.INFO("���������߳���ֹͣ��", -1, std::this_thread::get_id());
}


int main() {
    // ���ÿ���̨������ģ������Ҫ���ض���Windows��
    // #ifdef _WIN32
    // system("chcp 65001 > nul"); // ���ÿ���̨ΪUTF-8���룬������Ҫ����ԱȨ�޻��������Ҫ��
    // #endif

    gLogger.INFO("���ܼҾӿ���ϵͳ������...", -1, std::this_thread::get_id());

    DeviceContainer deviceContainer(gLogger);
    UserManager userManager("user.txt", deviceContainer.getAllDevicePtrs(), gLogger);

    // --- ��ʼ�û���֤ ---
    if (!initialAuthAndLogin(userManager, currentUser)) {
        gLogger.INFO("�û���֤ʧ�ܻ�ѡ���˳���ϵͳ�����رա�", -1, std::this_thread::get_id());
        return 0;
    }
    if (currentUser) {
        gLogger.INFO("�û� '" + currentUser->getUsername() + "' ��¼�ɹ�", -1, std::this_thread::get_id());
    }
    deviceContainer.importDevicesFromFileLogOnly("devices.txt");

    // --- �������������߳� ---
    std::thread envUpdateThread(environmentUpdateRoutine, std::ref(deviceContainer));


    // --- ��ѭ�� ---
    char choice_char;
    do {
        printMainMenu(currentUser);
        std::cin >> choice_char;

        if (std::cin.peek() == '\n') { std::cin.ignore(); }
        else if (std::cin.fail() || std::cin.eof()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            gLogger.ALERT("��⵽��Ч���롣�����ԡ�", -1, std::this_thread::get_id());
            std::cout << "������Ч��������ѡ��˵��" << std::endl;
            continue;
        }

        char lower_choice = std::tolower(choice_char);

        if (lower_choice == 'q') {
            gLogger.INFO("�û�ѡ���˳������ڱ����豸��Ϣ���ر�ϵͳ...", -1, std::this_thread::get_id());
            try {
                std::lock_guard<std::mutex> lock(deviceContainerMutex);
                deviceContainer.saveDevicesToFile("devices.txt");
            }
            catch (const BaseSmartHomeException& e) {
                gLogger.ALERT("�˳�ʱ�����豸ʧ�ܣ�" + std::string(e.what()), -1, std::this_thread::get_id());
                std::cerr << "���󣺱����豸ʱ�������� - " << e.what() << std::endl;
            }
            break;
        }

        int choice_int = -1;
        if (lower_choice >= '0' && lower_choice <= '9') {
            choice_int = lower_choice - '0';
        }
        else {
            gLogger.ALERT("��Ч�Ĳ˵�ѡ���ַ���" + std::string(1, choice_char), -1, std::this_thread::get_id());
            std::cout << "��Чѡ��������˵��е����ֻ� 'Q' �˳���" << std::endl;
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
                    gLogger.INFO("������ʾ�û� '" + currentUser->getUsername() + "' ����Ϣ��", -1, std::this_thread::get_id());
                    std::cout << "\n--- ��ǰ�û���Ϣ ---" << std::endl;
                    std::cout << "�û���: " << currentUser->getUsername() << std::endl;
                    std::cout << "�û���ɫ: " << roleToString(currentUser->getRole()) << std::endl;
                }
                else {
                    gLogger.INFO("��ǰû���û���¼��", -1, std::this_thread::get_id());
                    std::cout << "��ǰû���û���¼��" << std::endl;
                }
                break;
            case 2:
                deviceContainer.importDevicesFromFile("device_cn.txt");
                std::cout << "�豸�����������ɡ�������鿴��־��" << std::endl;
                break;
            case 3:
                handleAddDeviceFromKeyboard(deviceContainer, currentUser);
                break;
            case 4:
                deviceContainer.displayAllDevices();
                break;
            case 5:
                if (!currentUser) { gLogger.ALERT("�˲�����Ҫ��¼��"); std::cout << "���ȵ�¼��\n"; break; }
                std::cout << "������Ҫ���ҵ��豸ID��";
                int findId;
                std::cin >> findId;
                if (std::cin.fail()) { std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); std::cout << "ID������Ч��\n"; break; }
                deviceContainer.displayDeviceDetails(findId);
                break;
            case 6:
                if (!currentUser) { gLogger.ALERT("�˲�����Ҫ��¼��"); std::cout << "���ȵ�¼��\n"; break; }
                std::cout << "������Ҫɾ�����豸ID��";
                int deleteId;
                std::cin >> deleteId;
                if (std::cin.fail()) { std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); std::cout << "ID������Ч��\n"; break; }
                deviceContainer.deleteDeviceById(deleteId, *currentUser); // ������Ҫ���� currentUser
                std::cout << "����ɾ���豸ID " << deleteId << "��������鿴��־��" << std::endl;
                break;
            case 7:
                if (!currentUser) { gLogger.ALERT("�˲�����Ҫ��¼��"); std::cout << "���ȵ�¼��\n"; break; }
                deviceContainer.saveDevicesToFile("devices.txt");
                std::cout << "�豸�����������ɡ�������鿴��־��" << std::endl;
                break;
            case 8:
                if (!currentUser) {
                    gLogger.ALERT("ģ�ⳡ����Ҫ�û���¼��");
                    std::cout << "���ȵ�¼��ģ�����ܳ�����\n";
                    break;
                }
                handleSimulateScenes(currentUser);
                break;
            case 9: // �����豸 (ʾ��)
                handleUpdateDevice(deviceContainer, currentUser);
                break;
                // case 9 was sort before, changing to update. Add sort menu if needed.

            default:
                gLogger.ALERT("��Ч�Ĳ˵�ѡ��������" + std::to_string(choice_int), -1, std::this_thread::get_id());
                std::cout << "��ѡ����á�" << std::endl;
                break;
            }
        }
        catch (const PermissionDeniedException& e) {
            gLogger.ALERT("Ȩ�޴���" + std::string(e.what()), (currentUser ? -1 : -1), std::this_thread::get_id());
            std::cerr << "���� " << e.what() << std::endl;
        }
        catch (const DeviceNotFoundException& e) {
            gLogger.ALERT("�豸δ�ҵ�����" + std::string(e.what()), -1, std::this_thread::get_id());
            std::cerr << "���� " << e.what() << std::endl;
        }
        catch (const InvalidParameterException& e) {
            gLogger.ALERT("��Ч��������" + std::string(e.what()), -1, std::this_thread::get_id());
            std::cerr << "���� " << e.what() << std::endl;
        }
        catch (const FactoryNotFoundException& e) {
            gLogger.ALERT("��������" + std::string(e.what()), -1, std::this_thread::get_id());
            std::cerr << "���� " << e.what() << std::endl;
        }
        catch (const BaseSmartHomeException& e) {
            gLogger.ALERT("���ܼҾ�ϵͳ����" + std::string(e.what()), -1, std::this_thread::get_id());
            std::cerr << "ϵͳ���� " << e.what() << std::endl;
        }
        catch (const std::exception& e) {
            gLogger.ALERT("��׼���쳣��" + std::string(e.what()), -1, std::this_thread::get_id());
            std::cerr << "����������� " << e.what() << std::endl;
        }


        if (lower_choice != 'q') {
            std::cout << "\n�� Enter�� �������˵�...";
            // std::cin.get(); // ���ܻ����ĵ��ϴ� cin >> choice ���µĻ��з�
            // һ������׳�ķ��������ڻ��ʹ�� cin >> var �ͺ������ܵ� getline
            std::string dummy;
            // �����һ�������� cin >> var���򻺳����п�����һ�����з���
            // peek() �����һ���ַ�������ǻ��з������� ignore() ��������
            if (std::cin.peek() == '\n') {
                std::cin.ignore();
            }
            std::getline(std::cin, dummy); // �ȴ��û��� Enter
        }

    } while (true);

    // --- ϵͳ�ر� ---
    gLogger.INFO("���ܼҾ�ϵͳ���ڹر�...", -1, std::this_thread::get_id());
    g_isRunning = false; // ֪ͨ���������߳�ֹͣ
    if (envUpdateThread.joinable()) {
        envUpdateThread.join();
        gLogger.INFO("���������߳��ѳɹ���ϡ�", -1, std::this_thread::get_id());
    }

    gLogger.INFO("���ܼҾ�ϵͳ�ѹرա�", -1, std::this_thread::get_id());
    return 0;
}

// --- ���˵���������ʵ�� ---
void printMainMenu(const User* user) {
    std::cout << "\n=========== ���ܼҾ����˵� ===========\n";
    if (user) {
        std::cout << "��ǰ�û�: " << user->getUsername()
            << " (��ɫ: " << roleToString(user->getRole()) << ")\n";
    }
    else {
        std::cout << "��ǰ���û���¼��\n";
    }
    std::cout << "--------------------------------------------\n";
    std::cout << "0 ---- �л��û� / ע�����û�\n";
    std::cout << "1 ---- ��ʾ��ǰ�û���Ϣ\n";
    std::cout << "2 ---- ���ļ������豸\n";
    std::cout << "3 ---- �Ӽ�������豸\n";
    std::cout << "4 ---- �б���ʾ��ǰ�����豸\n";
    std::cout << "5 ---- ��ָ��ID���Ҳ���ʾ�豸\n";
    std::cout << "6 ---- ɾ��ָ��ID���豸 (����ԱȨ��)\n";
    std::cout << "7 ---- ���������豸��Ϣ���ļ�\n";
    std::cout << "8 ---- ���ܳ���ģ��\n";
    std::cout << "9 ---- �����豸��Ϣ (����ԱȨ��)\n";
    std::cout << "Q ---- �˳�ϵͳ\n";
    std::cout << "============================================\n";
    std::cout << "����������ѡ��";
}


bool initialAuthAndLogin(UserManager& userManager, User*& targetUser) {
    while (targetUser == nullptr) {
        std::cout << "\n=========== �û���֤ ===========\n";
        std::cout << "1. �û���¼\n";
        std::cout << "2. ע�����û�\n";
        std::cout << "3. �˳�ϵͳ\n";
        std::cout << "================================\n";
        std::cout << "��ѡ�������";
        char authChoice_char;
        std::cin >> authChoice_char;
        // �������뻺��������ֹӰ������� getline
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::string username, password;
        switch (authChoice_char) {
        case '1':
            std::cout << "�������û�����"; std::getline(std::cin, username);
            std::cout << "���������룺"; std::getline(std::cin, password);
            targetUser = userManager.loginUser(username, password);
            if (targetUser) {
                // userManager.loginUser �ڲ��Ѽ�¼��־
                std::cout << "��¼�ɹ�����ӭ����" << targetUser->getUsername() << "��" << std::endl;
                return true;
            }
            else {
                std::cout << "��¼ʧ�ܣ����������û����������Ƿ���ȷ��" << std::endl;
            }
            break;
        case '2':
            std::cout << "--- ע�����û� ---\n";
            std::cout << "���������û�����"; std::getline(std::cin, username);
            std::cout << "���������룺"; std::getline(std::cin, password);
            targetUser = userManager.registerUser(username, password, UserRole::USER); // Ĭ��ע��Ϊ��ͨ�û�
            if (targetUser) {
                // userManager.registerUser �ڲ��Ѽ�¼��־
                std::cout << "ע��ɹ��������Զ���¼��" << targetUser->getUsername() << "��" << std::endl;
                return true;
            }
            else {
                std::cout << "ע��ʧ�ܡ��������û����Ѵ��ڻ�������Ч��" << std::endl;
            }
            break;
        case '3':
            gLogger.INFO("�û�����֤�׶�ѡ���˳�ϵͳ��", -1, std::this_thread::get_id());
            std::cout << "��ѡ�����˳�ϵͳ��" << std::endl;
            return false;
        default:
            gLogger.ALERT("��Ч����֤ѡ�" + std::string(1, authChoice_char), -1, std::this_thread::get_id());
            std::cout << "��Ч��ѡ����������롣" << std::endl;
            break;
        }
    }
    return false; // ������ѭ���ᴦ��Ϊ����������
}

void handleChangeUser(UserManager& userManager, User*& targetUser) {
    if (targetUser) {
        gLogger.INFO("�û� '" + targetUser->getUsername() + "' ��ע����", -1, std::this_thread::get_id());
        std::cout << "�û� '" << targetUser->getUsername() << "' ��ע����" << std::endl;
        targetUser = nullptr;
    }
    if (!initialAuthAndLogin(userManager, targetUser)) {
        gLogger.INFO("���л��û������У��û���֤ʧ�ܻ�ѡ���˳���", -1, std::this_thread::get_id());
        // ���ڴ˴��˳��������򣬽��� targetUser ��Ϊ nullptr����ѭ��������
        // �����Ҫ�ڴ˴��˳��������׳��ض��쳣������ȫ���˳���־
        std::cout << "δ������û���֤��" << std::endl;
        // g_isRunning = false; // �����Ҫ�˳���������
    }
    else {
        if (targetUser) {
            gLogger.INFO("�û� '" + targetUser->getUsername() + "' ���ѵ�¼��", -1, std::this_thread::get_id());
        }
    }
}


void handleAddDeviceFromKeyboard(DeviceContainer& container, const User* currentUser) {
    if (!currentUser) {
        gLogger.ALERT("����豸��Ҫ�û���¼��");
        std::cout << "���ȵ�¼��������豸��" << std::endl;
        return;
    }
    // Ȩ�޼�飺���磬ֻ�й���Ա�������ĳЩ���͵��豸���������豸
    // if (currentUser->getRole() != UserRole::ADMIN) {
    //     gLogger.ALERT("Ȩ�޲��㣺�û� '" + currentUser->getUsername() + "' ���ԴӼ�������豸��");
    //     std::cout << "Ȩ�޲��㣬�޷�����豸��" << std::endl;
    //     throw PermissionDeniedException("ֻ�й���Ա���ܴӼ�������豸��"); // ���߲����쳣������ʾ
    //     return;
    // }


    std::cout << "\n--- �Ӽ���������豸 ---\n";
    DeviceParams params;
    DeviceType typeEnum;

    std::cout << "��ѡ���豸���� (1: ����������, 2: ���ܵƾ�, 3: �յ�): ";
    int typeChoice;
    std::cin >> typeChoice;
    if (std::cin.fail() || typeChoice < 1 || typeChoice > 3) {
        gLogger.ALERT("�Ӽ�������豸ʱѡ������Ч���豸���͡�");
        std::cout << "��Ч���豸���͡�������ֹ��" << std::endl;
        std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "�������豸ID (����): "; std::cin >> params.id;
    if (std::cin.fail() || container.findDeviceById(params.id) != nullptr) {
        gLogger.ALERT("��Ч���豸ID��ID " + std::to_string(params.id) + " �Ѵ��ڡ�", params.id);
        std::cout << "ID������Ч���ID�Ѵ��ڡ�������ֹ��" << std::endl;
        std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "�������豸����: "; std::getline(std::cin, params.name);
    if (params.name.empty()) params.name = "δ�����豸";

    std::cout << "�������豸��Ҫ�̶� (0:��, 1:��, 2:��, 3:Σ��): ";
    int impChoice; std::cin >> impChoice;
    params.importance = stringToImportance(std::to_string(impChoice)); // stringToImportance������Ч����

    std::cout << "�������豸���� (����): "; std::cin >> params.powerConsumption;
    if (std::cin.fail()) { /* ������ */ std::cout << "����������Ч��" << std::endl; std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); return; }


    std::cout << "�������豸λ��: ";
    // �����һ�������� cin >> var��������Ҫ consume newline
    if (std::cin.peek() == '\n') { std::cin.ignore(); }
    std::getline(std::cin, params.location);
    if (params.location.empty()) params.location = "δ֪λ��";


    try {
        switch (typeChoice) {
        case 1:
            typeEnum = DeviceType::SENSOR;
            std::cout << "�������ʼ�¶� (���϶�): "; std::cin >> params.temperature;
            std::cout << "�������ʼʪ�� (%): "; std::cin >> params.humidity;
            std::cout << "�������ʼCO2Ũ�� (����, 0.04 ���� 4%): "; std::cin >> params.co2Concentration;
            break;
        case 2:
            typeEnum = DeviceType::LIGHT;
            std::cout << "�ƾ��Ƿ��ʼ����? (1 ������, 0 �����): ";
            int isOnChoice; std::cin >> isOnChoice; params.isOn = (isOnChoice == 1);
            if (params.isOn) {
                std::cout << "�������ʼ���� (0-100%): "; std::cin >> params.brightness;
            }
            else {
                params.brightness = 0;
            }
            break;
        case 3:
            typeEnum = DeviceType::AC;
            int modeChoice, fanChoice;
            // ע�⣺�����������Ҫ��AC.h/cpp�ж����ö�ٺ�ת��������Ӧ
            std::cout << "������յ�ģʽ (0:����, 1:����, 2:�ͷ�, 3:�ر�): "; std::cin >> modeChoice;
            if (modeChoice >= 0 && modeChoice <= 3) params.acMode = static_cast<ACMode>(modeChoice);
            else { std::cout << "ģʽѡ����Ч��Ĭ��Ϊ�رա�\n"; params.acMode = ACMode::OFF; }


            if (params.acMode != ACMode::OFF) {
                std::cout << "������Ŀ���¶� (���϶�): "; std::cin >> params.targetTemperature;
                std::cout << "��������� (0:�ͷ�, 1:�з�, 2:�߷�, 3:�Զ�): "; std::cin >> fanChoice;
                if (fanChoice >= 0 && fanChoice <= 3) params.fanSpeed = static_cast<FanSpeed>(fanChoice);
                else { std::cout << "����ѡ����Ч��Ĭ��Ϊ�Զ���\n"; params.fanSpeed = FanSpeed::AUTO; }
            }
            else {
                params.targetTemperature = 25;
                params.fanSpeed = FanSpeed::AUTO;
            }
            break;
        }
        // ȷ���� switch case ���ĩβ��ѭ������һ�ε���֮ǰ���κ�δ�� getline ����Ļ��з���������
        if (std::cin.peek() == '\n') { std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); }
        else { std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); }


        container.addDeviceFromParams(typeEnum, params);
        std::cout << "�豸 '" << params.name << "' (ID: " << params.id << ") �ѳɹ�������" << std::endl;

    }
    catch (const BaseSmartHomeException& e) {
        gLogger.ALERT("�Ӽ�������豸ʧ�ܣ�" + std::string(e.what()), params.id, std::this_thread::get_id());
        std::cerr << "����豸ʱ��������: " << e.what() << std::endl;
        if (std::cin.fail()) { std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); }
    }
    catch (const std::exception& e) {
        gLogger.ALERT("�Ӽ�������豸ʱ������׼���쳣��" + std::string(e.what()), params.id, std::this_thread::get_id());
        std::cerr << "�����������: " << e.what() << std::endl;
        if (std::cin.fail()) { std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); }
    }
}

void handleUpdateDevice(DeviceContainer& container, const User* currentUser) {
    if (!currentUser) {
        gLogger.ALERT("�����豸��Ҫ�û���¼��");
        std::cout << "���ȵ�¼���ܸ����豸��" << std::endl;
        return;
    }
    // Ȩ�޼�������� DeviceContainer::updateDevice

    std::cout << "������Ҫ���µ��豸ID��";
    int idToUpdate;
    std::cin >> idToUpdate;
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "ID������Ч��\n";
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    const Device* existingDevice = container.findDeviceById(idToUpdate);
    if (!existingDevice) {
        std::cout << "δ�ҵ�IDΪ " << idToUpdate << " ���豸��" << std::endl;
        return;
    }

    std::cout << "�ҵ��豸��" << existingDevice->getName() << "���������µ��豸������" << std::endl;
    DeviceParams params;
    params.id = idToUpdate; // Ĭ��ID���ֲ��䣬�����û������µ�

    std::cout << "�豸ID (�������������ԭID " << idToUpdate << "): "; std::cin >> params.id;
    if (std::cin.fail()) { /* ... ID��������� ... */ return; }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "�豸���� (ԭ: " << existingDevice->getName() << "): "; std::getline(std::cin, params.name);
    if (params.name.empty()) params.name = existingDevice->getName(); // ����������򱣳�ԭ��

    std::cout << "��Ҫ�̶� (0:��, 1:��, 2:��, 3:Σ��, ԭ: " << static_cast<int>(existingDevice->getImportance()) << "): ";
    int impChoice; std::cin >> impChoice; params.importance = stringToImportance(std::to_string(impChoice));

    std::cout << "���� (ԭ: " << existingDevice->getPowerConsumption() << "): "; std::cin >> params.powerConsumption;
    if (std::cin.fail()) { /* ...������������� ... */ return; }

    std::cout << "λ�� (ԭ: " << existingDevice->getLocation() << "): ";
    if (std::cin.peek() == '\n') { std::cin.ignore(); }
    std::getline(std::cin, params.location);
    if (params.location.empty()) params.location = existingDevice->getLocation();

    // �����豸���ͻ�ȡ�ض�����
    if (const Sensor* sensor = dynamic_cast<const Sensor*>(existingDevice)) {
        std::cout << "�¶� (ԭ: " << sensor->getTemperature() << "): "; std::cin >> params.temperature;
        std::cout << "ʪ�� (ԭ: " << sensor->getHumidity() << "): "; std::cin >> params.humidity;
        std::cout << "CO2Ũ�� (ԭ: " << sensor->getCO2Concentration() << "): "; std::cin >> params.co2Concentration;
    }
    else if (const Light* light = dynamic_cast<const Light*>(existingDevice)) {
        std::cout << "���Ƿ��� (1��/0��, ԭ: " << light->getIsOn() << "): "; int on; std::cin >> on; params.isOn = (on == 1);
        std::cout << "���� (ԭ: " << light->getBrightness() << "): "; std::cin >> params.brightness;
    }
    else if (const AC* ac = dynamic_cast<const AC*>(existingDevice)) {
        std::cout << "ģʽ (0:����,1:����,2:�ͷ�,3:�ر�, ԭ: " << static_cast<int>(ac->getMode()) << "): "; int mode; std::cin >> mode; params.acMode = static_cast<ACMode>(mode);
        std::cout << "Ŀ���¶� (ԭ: " << ac->getTargetTemperature() << "): "; std::cin >> params.targetTemperature;
        std::cout << "���� (0:��,1:��,2:��,3:�Զ�, ԭ: " << static_cast<int>(ac->getFanSpeed()) << "): "; int speed; std::cin >> speed; params.fanSpeed = static_cast<FanSpeed>(speed);
    }
    if (std::cin.peek() == '\n') { std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); }
    else { std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); }


    container.updateDevice(idToUpdate, params, *currentUser); // updateDevice �ڲ������Ȩ�޼�����־��¼
    std::cout << "���Ը����豸ID " << idToUpdate << "��������鿴��־��" << std::endl;
}


void handleSimulateScenes(User* user) {
    if (!user) {
        gLogger.ALERT("ģ�ⳡ����Ҫ�û���¼��", -1, std::this_thread::get_id());
        std::cout << "���ȵ�¼��ģ�����ܳ�����" << std::endl;
        return;
    }
    // User�����ڲ��ж��豸�б�����ã�����ֱ�ӵ����䳡������

    std::cout << "\n--- ���ܳ���ģ�� ---" << std::endl;
    std::cout << "1. ģ����ʪ�ȳ��� (�յ�����)" << std::endl;
    std::cout << "2. ģ����ּ�� (CO2���꣬���Թرյ���)" << std::endl;
    std::cout << "��ѡ��Ҫģ��ĳ�����";
    int sceneChoice;
    std::cin >> sceneChoice;

    if (std::cin.fail()) {
        gLogger.ALERT("����ѡ��������Ч��", -1, std::this_thread::get_id());
        std::cout << "������Ч��" << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // �����з�

    switch (sceneChoice) {
    case 1:
        gLogger.INFO("�û� '" + user->getUsername() + "' ������ʪ�ȳ��޳���ģ�⡣", -1, std::this_thread::get_id());
        user->runTemperatureHumidityRule();
        std::cout << "��ʪ�ȳ��޳���ģ��ִ����ϡ�������鿴��־��" << std::endl;
        break;
    case 2:
        gLogger.INFO("�û� '" + user->getUsername() + "' �������ּ�ⳡ��ģ�⡣", -1, std::this_thread::get_id());
        user->runFireEmergencyRule();
        std::cout << "���ּ�ⳡ��ģ��ִ����ϡ�������鿴��־��" << std::endl;
        break;
    default:
        gLogger.ALERT("��Ч�ĳ���ѡ��" + std::to_string(sceneChoice), -1, std::this_thread::get_id());
        std::cout << "��Ч�ĳ���ѡ��" << std::endl;
        break;
    }
}