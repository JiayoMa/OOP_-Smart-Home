// LightFactory.cpp
#include "LightFactory.h"

Device* LightFactory::createDevice() {
    DeviceParams defaultParams;
    defaultParams.id = 0;
    defaultParams.name = "默认灯具";
    defaultParams.location = "默认灯具位置";
    return new Light(defaultParams.id, defaultParams.name, defaultParams.importance,
        defaultParams.powerConsumption, defaultParams.location,
        defaultParams.isOn, defaultParams.brightness);
}

Device* LightFactory::createDeviceWithParams(const DeviceParams& params) {
    return new Light(params.id, params.name, params.importance, params.powerConsumption,
        params.location, params.isOn, params.brightness);
}