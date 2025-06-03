
#include "Light.h"
#include <iostream>
#include <sstream>
#include "Device.h"

Light::Light(int id, const std::string& name, DeviceImportance importance, double powerConsumption,
    const std::string& location, bool on, int bright)
    : Device(id, name, importance, powerConsumption, location), isOn(on) {
    if (bright < 0) brightness = 0;
    else if (bright > 100) brightness = 100;
    else brightness = bright;

    if (brightness > 0 && !isOn) { // 如果亮度大于0但状态是关，则自动打开
        this->isOn = true;
    }
}

Light::~Light() {
    // std::cout << "智能灯具 " << name << " (ID: " << id << ") 已销毁。" << std::endl;
}

std::string Light::toFileString() const {
    std::stringstream ss;
    ss << "LIGHT," // 类型标识
        << Device::toFileString() // 调用基类获取通用部分
        << "," << (getIsOn() ? "1" : "0")
        << "," << getBrightness();
    return ss.str();
}

void Light::updateStatus() {
    // 灯具的状态通常由直接控制改变，这里可以简单显示当前状态
    std::cout << "正在更新智能灯具 " << name << " (ID: " << id << ") 的状态:" << std::endl; //
    std::cout << "  当前状态: " << (isOn ? "开启" : "关闭") << std::endl; //
    if (isOn) {
        std::cout << "  亮度: " << brightness << "%" << std::endl; //
    }
}

void Light::displayInfo() const {
    Device::displayInfo(); // 调用基类的 displayInfo
    std::cout << ", 类型: 智能灯具"
        << ", 状态: " << (isOn ? "开启" : "关闭"); //
    if (isOn) {
        std::cout << ", 亮度: " << brightness << "%"; //
    }
}

void Light::turnOn() {
    isOn = true;
    if (brightness == 0) brightness = 50; // 如果打开时亮度为0，设为默认亮度
    std::cout << "智能灯具 " << name << " (ID: " << id << ") 已开启。" << std::endl; //
}

void Light::turnOff() {
    isOn = false;
    // 亮度可以保持，下次打开时恢复；或者设为0，看设计选择
    // brightness = 0; 
    std::cout << "智能灯具 " << name << " (ID: " << id << ") 已关闭。" << std::endl; //
}

void Light::setBrightness(int level) {
    if (level >= 0 && level <= 100) {
        brightness = level;
        std::cout << "智能灯具 " << name << " (ID: " << id << ") 亮度已设置为 " << brightness << "%。" << std::endl; //
        if (brightness > 0 && !isOn) { // 如果设置了亮度且灯是关的，则打开灯
            turnOn(); //
        }
        else if (brightness == 0 && isOn) { // 如果亮度设为0且灯是开的，则关闭灯
            turnOff(); //
        }
    }
    else {
        std::cout << "设置失败：无效的亮度级别。亮度必须在 0 到 100 之间。" << std::endl; //
    }
}

bool Light::getIsOn() const { return isOn; }             //
int Light::getBrightness() const { return brightness; } //

std::istream& operator>>(std::istream& is, Light& light) {
    is >> static_cast<Device&>(light); // 读取通用设备信息
    std::cout << "灯是否初始开启? (1 代表是, 0 代表否): "; //
    int onStateChoice;
    is >> onStateChoice;
    light.isOn = (onStateChoice == 1); //

    if (light.isOn) {
        std::cout << "请输入初始亮度 (0-100%): "; //
        is >> light.brightness;
        if (light.brightness < 0) light.brightness = 0;       //
        if (light.brightness > 100) light.brightness = 100;   //
    }
    else {
        light.brightness = 0; // 关闭时亮度为0
    }
    is.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 清除换行符
    return is;
}