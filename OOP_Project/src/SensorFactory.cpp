// SensorFactory.cpp
#include "SensorFactory.h"

Device* SensorFactory::createDevice() {
    DeviceParams defaultParams;
    defaultParams.id = 0; 
    defaultParams.name = "Ĭ�ϴ�����";
    defaultParams.location = "Ĭ�ϴ�����λ��";
    return new Sensor(defaultParams.id, defaultParams.name, defaultParams.importance,
        defaultParams.powerConsumption, defaultParams.location,
        defaultParams.temperature, defaultParams.humidity,
        defaultParams.co2Concentration);
}

Device* SensorFactory::createDeviceWithParams(const DeviceParams& params) {
    return new Sensor(params.id, params.name, params.importance, params.powerConsumption,
        params.location, params.temperature, params.humidity,
        params.co2Concentration);
}