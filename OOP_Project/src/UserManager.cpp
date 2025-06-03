// UserManager.cpp
#include "UserManager.h"
#include "User.h"
#include "Device.h"    // ���� Device �ඨ��
#include "CustomExceptions.h"
#include <fstream>
#include <sstream>
#include <iostream>   // ����ĳЩ��ʼ���󣬵�����ʹ��logger
#include <algorithm>  // Ϊ�� std::remove_if

UserManager::UserManager(const std::string& filePath, std::vector<Device*>& devices_ref_param, SmartLogger& loggerRef)
    : usersFilePath(filePath), allDevices_ref(devices_ref_param), logger(loggerRef) {
    logger.INFO("�û���������ʼ�������ڴ�����·�������û���" + filePath);
    loadUsersFromFile();
    if (users.empty()) {
        logger.INFO("δ�ҵ��û����ݻ��ļ�Ϊ�ա����ڴ���Ĭ�Ϲ���Ա�˻� (admin/admin)...");
        registerUser("admin", "admin", UserRole::ADMIN);
    }
}

void UserManager::loadUsersFromFile() {
    std::ifstream inFile(usersFilePath);
    if (!inFile.is_open()) {
        logger.ALERT("�޷����û��ļ����м��أ�" + usersFilePath);
        return;
    }
    users.clear(); // �������unique_ptr���ͷ��ڴ� 
    std::string line;
    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::string username, passwordHash, roleStr;
        if (std::getline(ss, username, ':') &&
            std::getline(ss, passwordHash, ':') &&
            std::getline(ss, roleStr)) {
            // �ڶ��ϴ���User����ͨ��unique_ptr�洢
            // ��logger���ô��ݸ�User���캯��
            users.push_back(std::make_unique<User>(username, passwordHash, stringToRole(roleStr), allDevices_ref, logger));
        }
        else {
            logger.ALERT("���棺�û��ļ��д��ڸ�ʽ������У�" + line);
        }
    }
    inFile.close();
    logger.INFO(std::to_string(users.size()) + " ���û��Ѵ��ļ����ء�");
}

void UserManager::saveUsersToFile() const {
    std::ofstream outFile(usersFilePath);
    if (!outFile.is_open()) {
        logger.ALERT("�����޷����û��ļ� '" + usersFilePath + "' ����д�롣");
        return;
    }

    for (const auto& user_ptr : users) { // ����unique_ptr
        if (user_ptr) { // ���ָ���Ƿ�ǿ�
            outFile << user_ptr->getUsername() << ":"
                << user_ptr->getPassword() << ":" // ע�⣺���ﱣ������������루���ϣ��
                << roleToString(user_ptr->getRole()) << std::endl;
        }
    }
    outFile.close();
    logger.INFO("�����û���Ϣ�ѱ��浽�ļ���" + usersFilePath);
}

User* UserManager::registerUser(const std::string& username, const std::string& password, UserRole role) {
    if (username.empty() || password.empty()) {
        logger.ALERT("ע��ʧ�ܣ��û��������벻��Ϊ�ա�");
        return nullptr;
    }
    for (const auto& user_ptr : users) {
        if (user_ptr && user_ptr->getUsername() == username) {
            logger.ALERT("ע��ʧ�ܣ��û��� '" + username + "' �Ѵ��ڡ�");
            return nullptr;
        }
    }

    // ��logger���ô��ݸ�User���캯��
    users.push_back(std::make_unique<User>(username, password, role, allDevices_ref, logger));
    saveUsersToFile();
    logger.INFO("�û� '" + username + "' ע��ɹ���", -1);
    return users.back().get(); // ���µ�unique_ptr����ԭʼָ�� 
}

User* UserManager::loginUser(const std::string& username, const std::string& password) {
    for (const auto& user_ptr : users) {
        if (user_ptr && user_ptr->getUsername() == username && user_ptr->getPassword() == password) { // ������֤
            logger.INFO("�û� '" + username + "' ��¼�ɹ���", -1);
            return user_ptr.get(); // ����ԭʼָ��
        }
    }
    logger.ALERT("��¼ʧ�ܣ���Ч���û��������룬����û� '" + username + "'��", -1);
    return nullptr;
}

bool UserManager::deleteUser(const std::string& usernameToDelete, const User& requestingUser) {
    if (requestingUser.getRole() != UserRole::ADMIN) {
        logger.ALERT("Ȩ�޲��㣺�û� '" + requestingUser.getUsername() + "' ����ɾ���û� '" +
            usernameToDelete + "'����û�й���ԱȨ�ޡ�", -1);
        throw PermissionDeniedException("ֻ�й���Ա����ɾ���û���");
    }

    if (requestingUser.getUsername() == usernameToDelete) {
        logger.ALERT("������Ч������Ա '" + requestingUser.getUsername() + "' ����ɾ���Լ���", -1);
        throw InvalidParameterException("����Ա����ɾ���Լ���");
    }

    auto it = std::remove_if(users.begin(), users.end(),
        [&](const std::unique_ptr<User>& u_ptr) {
            return u_ptr && u_ptr->getUsername() == usernameToDelete;
        });

    if (it != users.end()) {
        users.erase(it, users.end()); // ������unique_ptr���Ӷ�ɾ��User���� 
        saveUsersToFile();
        logger.INFO("�û� '" + usernameToDelete + "' �ѱ�����Ա '" + requestingUser.getUsername() + "' ɾ����", -1);
        return true;
    }
    else {
        logger.ALERT("ɾ��ʧ�ܣ�δ�ҵ��û� '" + usernameToDelete + "'��", -1);
        throw DeviceNotFoundException("�û� '" + usernameToDelete + "' δ�ҵ����޷�ɾ����"); // Or a UserNotFoundException
    }
}

User* UserManager::findUser(const std::string& username) {
    for (const auto& user_ptr : users) {
        if (user_ptr && user_ptr->getUsername() == username) {
            return user_ptr.get(); // ����ԭʼָ�� 
        }
    }
    logger.DEBUG("δ�ҵ��û� '" + username + "'��", -1); // ͨ������ʧ�ܲ��㾯����������������Ҫ
    return nullptr;
}