// ACFactory.cpp
#include "ACFactory.h"

Device* ACFactory::createDevice() {
    DeviceParams defaultParams;
    defaultParams.id = 0;
    defaultParams.name = "Ĭ�Ͽյ�";
    defaultParams.location = "Ĭ�Ͽյ�λ��";
    return new AC(defaultParams.id, defaultParams.name, defaultParams.importance,
        defaultParams.powerConsumption, defaultParams.location,
        defaultParams.acMode, defaultParams.targetTemperature,
        defaultParams.fanSpeed);
}

Device* ACFactory::createDeviceWithParams(const DeviceParams& params) {
    return new AC(params.id, params.name, params.importance, params.powerConsumption,
        params.location, params.acMode, params.targetTemperature,
        params.fanSpeed);
}