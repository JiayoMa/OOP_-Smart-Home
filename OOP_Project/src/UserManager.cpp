// UserManager.cpp
#include "UserManager.h"
#include "User.h"
#include "Device.h"    // 包含 Device 类定义
#include "CustomExceptions.h"
#include <fstream>
#include <sstream>
#include <iostream>   // 用于某些初始错误，但优先使用logger
#include <algorithm>  // 为了 std::remove_if

UserManager::UserManager(const std::string& filePath, std::vector<Device*>& devices_ref_param, SmartLogger& loggerRef)
    : usersFilePath(filePath), allDevices_ref(devices_ref_param), logger(loggerRef) {
    logger.INFO("用户管理器初始化。正在从以下路径加载用户：" + filePath);
    loadUsersFromFile();
    if (users.empty()) {
        logger.INFO("未找到用户数据或文件为空。正在创建默认管理员账户 (admin/admin)...");
        registerUser("admin", "admin", UserRole::ADMIN);
    }
}

void UserManager::loadUsersFromFile() {
    std::ifstream inFile(usersFilePath);
    if (!inFile.is_open()) {
        logger.ALERT("无法打开用户文件进行加载：" + usersFilePath);
        return;
    }
    users.clear(); // 清除现有unique_ptr，释放内存 
    std::string line;
    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::string username, passwordHash, roleStr;
        if (std::getline(ss, username, ':') &&
            std::getline(ss, passwordHash, ':') &&
            std::getline(ss, roleStr)) {
            // 在堆上创建User对象并通过unique_ptr存储
            // 将logger引用传递给User构造函数
            users.push_back(std::make_unique<User>(username, passwordHash, stringToRole(roleStr), allDevices_ref, logger));
        }
        else {
            logger.ALERT("警告：用户文件中存在格式错误的行：" + line);
        }
    }
    inFile.close();
    logger.INFO(std::to_string(users.size()) + " 个用户已从文件加载。");
}

void UserManager::saveUsersToFile() const {
    std::ofstream outFile(usersFilePath);
    if (!outFile.is_open()) {
        logger.ALERT("错误：无法打开用户文件 '" + usersFilePath + "' 进行写入。");
        return;
    }

    for (const auto& user_ptr : users) { // 遍历unique_ptr
        if (user_ptr) { // 检查指针是否非空
            outFile << user_ptr->getUsername() << ":"
                << user_ptr->getPassword() << ":" // 注意：这里保存的是明文密码（或哈希）
                << roleToString(user_ptr->getRole()) << std::endl;
        }
    }
    outFile.close();
    logger.INFO("所有用户信息已保存到文件：" + usersFilePath);
}

User* UserManager::registerUser(const std::string& username, const std::string& password, UserRole role) {
    if (username.empty() || password.empty()) {
        logger.ALERT("注册失败：用户名和密码不能为空。");
        return nullptr;
    }
    for (const auto& user_ptr : users) {
        if (user_ptr && user_ptr->getUsername() == username) {
            logger.ALERT("注册失败：用户名 '" + username + "' 已存在。");
            return nullptr;
        }
    }

    // 将logger引用传递给User构造函数
    users.push_back(std::make_unique<User>(username, password, role, allDevices_ref, logger));
    saveUsersToFile();
    logger.INFO("用户 '" + username + "' 注册成功。", -1);
    return users.back().get(); // 从新的unique_ptr返回原始指针 
}

User* UserManager::loginUser(const std::string& username, const std::string& password) {
    for (const auto& user_ptr : users) {
        if (user_ptr && user_ptr->getUsername() == username && user_ptr->getPassword() == password) { // 密码验证
            logger.INFO("用户 '" + username + "' 登录成功。", -1);
            return user_ptr.get(); // 返回原始指针
        }
    }
    logger.ALERT("登录失败：无效的用户名或密码，针对用户 '" + username + "'。", -1);
    return nullptr;
}

bool UserManager::deleteUser(const std::string& usernameToDelete, const User& requestingUser) {
    if (requestingUser.getRole() != UserRole::ADMIN) {
        logger.ALERT("权限不足：用户 '" + requestingUser.getUsername() + "' 尝试删除用户 '" +
            usernameToDelete + "'，但没有管理员权限。", -1);
        throw PermissionDeniedException("只有管理员才能删除用户。");
    }

    if (requestingUser.getUsername() == usernameToDelete) {
        logger.ALERT("操作无效：管理员 '" + requestingUser.getUsername() + "' 不能删除自己。", -1);
        throw InvalidParameterException("管理员不能删除自己。");
    }

    auto it = std::remove_if(users.begin(), users.end(),
        [&](const std::unique_ptr<User>& u_ptr) {
            return u_ptr && u_ptr->getUsername() == usernameToDelete;
        });

    if (it != users.end()) {
        users.erase(it, users.end()); // 这会擦除unique_ptr，从而删除User对象 
        saveUsersToFile();
        logger.INFO("用户 '" + usernameToDelete + "' 已被管理员 '" + requestingUser.getUsername() + "' 删除。", -1);
        return true;
    }
    else {
        logger.ALERT("删除失败：未找到用户 '" + usernameToDelete + "'。", -1);
        throw DeviceNotFoundException("用户 '" + usernameToDelete + "' 未找到，无法删除。"); // Or a UserNotFoundException
    }
}

User* UserManager::findUser(const std::string& username) {
    for (const auto& user_ptr : users) {
        if (user_ptr && user_ptr->getUsername() == username) {
            return user_ptr.get(); // 返回原始指针 
        }
    }
    logger.DEBUG("未找到用户 '" + username + "'。", -1); // 通常查找失败不算警报，除非上下文需要
    return nullptr;
}