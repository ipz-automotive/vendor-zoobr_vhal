#define LOG_TAG "automotive.vehicle@2.0-zoobr-service"
#include <android/log.h>
#include <hidl/HidlTransportSupport.h>

#include <iostream>

#include <vhal_v2_0/DefaultVehicleConnector.h>
#include <vhal_v2_0/DefaultVehicleHal.h>

#include "ZoobrVehicleHalManager.h"

using ::android::hardware::automotive::vehicle::V2_0::VehiclePropertyStore;
using ::android::hardware::automotive::vehicle::V2_0::impl::DefaultVehicleConnector;
using ::android::hardware::automotive::vehicle::V2_0::impl::DefaultVehicleHal;
using ::android::hardware::automotive::vehicle::V2_0::impl::ZoobrVehicleHalManager;

int main(int /* argc */, char* /* argv */ []) {
    auto store = std::make_unique<VehiclePropertyStore>();
    auto connector = std::make_unique<DefaultVehicleConnector>();
    auto hal = std::make_unique<DefaultVehicleHal>(store.get(), connector.get());
    auto service = android::sp<ZoobrVehicleHalManager>::make(hal.get());
    connector->setValuePool(hal->getValuePool());

    android::hardware::configureRpcThreadpool(4, true /* callerWillJoin */);

    ALOGI("Registering as service...");
    android::status_t status = service->registerAsService();

    if (status != android::OK) {
        ALOGE("Unable to register vehicle service (%d)", status);
        return 1;
    }

    ALOGI("Ready");
    android::hardware::joinRpcThreadpool();

    return 0;
}
