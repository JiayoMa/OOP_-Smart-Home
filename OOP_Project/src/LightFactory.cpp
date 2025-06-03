// LightFactory.cpp
#include "LightFactory.h"

Device* LightFactory::createDevice() {
    DeviceParams defaultParams;
    defaultParams.id = 0;
    defaultParams.name = "Ĭ�ϵƾ�";
    defaultParams.location = "Ĭ�ϵƾ�λ��";
    return new Light(defaultParams.id, defaultParams.name, defaultParams.importance,
        defaultParams.powerConsumption, defaultParams.location,
        defaultParams.isOn, defaultParams.brightness);
}

Device* LightFactory::createDeviceWithParams(const DeviceParams& params) {
    return new Light(params.id, params.name, params.importance, params.powerConsumption,
        params.location, params.isOn, params.brightness);
}