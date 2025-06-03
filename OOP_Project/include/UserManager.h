// UserManager.h
#pragma once
#include <vector>
#include <string>
#include <memory> // 为了 std::unique_ptr
#include "User.h" 
#include "SmartLogger.h" // 包含 SmartLogger 定义

class UserManager {
private:
    std::vector<std::unique_ptr<User>> users;    // 存储指向User对象的unique_ptr
    std::string usersFilePath;                   // 用户数据文件路径
    std::vector<Device*>& allDevices_ref;        // 对DeviceContainer中设备指针向量的引用
    SmartLogger& logger;                         // 引用日志记录器

    void loadUsersFromFile();   // 从文件加载用户数据
    void saveUsersToFile() const; // 保存用户数据到文件
public:
    UserManager(const std::string& filePath, std::vector<Device*>& devices, SmartLogger& loggerRef);
    // 如果unique_ptr处理内存，则不需要显式的 ~UserManager

    // 返回原始指针 (User*) 以方便使用，但所有权仍归unique_ptr所有
    User* registerUser(const std::string& username, const std::string& password, UserRole role = UserRole::USER);
    User* loginUser(const std::string& username, const std::string& password);
    // 删除用户时需要执行删除操作的用户 (通常是管理员)
    bool deleteUser(const std::string& usernameToDelete, const User& requestingUser);
    User* findUser(const std::string& username); // 返回原始指针
};