// SensorFactory.cpp
#include "SensorFactory.h"

Device* SensorFactory::createDevice() {
    DeviceParams defaultParams;
    defaultParams.id = 0; 
    defaultParams.name = "默认传感器";
    defaultParams.location = "默认传感器位置";
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