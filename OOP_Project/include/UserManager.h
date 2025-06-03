// UserManager.h
#pragma once
#include <vector>
#include <string>
#include <memory> // Ϊ�� std::unique_ptr
#include "User.h" 
#include "SmartLogger.h" // ���� SmartLogger ����

class UserManager {
private:
    std::vector<std::unique_ptr<User>> users;    // �洢ָ��User�����unique_ptr
    std::string usersFilePath;                   // �û������ļ�·��
    std::vector<Device*>& allDevices_ref;        // ��DeviceContainer���豸ָ������������
    SmartLogger& logger;                         // ������־��¼��

    void loadUsersFromFile();   // ���ļ������û�����
    void saveUsersToFile() const; // �����û����ݵ��ļ�
public:
    UserManager(const std::string& filePath, std::vector<Device*>& devices, SmartLogger& loggerRef);
    // ���unique_ptr�����ڴ棬����Ҫ��ʽ�� ~UserManager

    // ����ԭʼָ�� (User*) �Է���ʹ�ã�������Ȩ�Թ�unique_ptr����
    User* registerUser(const std::string& username, const std::string& password, UserRole role = UserRole::USER);
    User* loginUser(const std::string& username, const std::string& password);
    // ɾ���û�ʱ��Ҫִ��ɾ���������û� (ͨ���ǹ���Ա)
    bool deleteUser(const std::string& usernameToDelete, const User& requestingUser);
    User* findUser(const std::string& username); // ����ԭʼָ��
};