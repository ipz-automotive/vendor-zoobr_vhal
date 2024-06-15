#ifndef android_hardware_automotive_vehicle_V2_0_impl_ZoobrVehicleHal_H_
#define android_hardware_automotive_vehicle_V2_0_impl_ZoobrVehicleHal_H_

#include <vhal_v2_0/VehicleHalManager.h>
#include <vhal_v2_0/VehicleUtils.h>
namespace android {
namespace hardware {
namespace automotive {
namespace vehicle {
namespace V2_0 {

namespace impl {

class ZoobrVehicleHalManager : public IVehicle {
public:
    ZoobrVehicleHalManager(VehicleHal* vehicleHal)
        : mHal(vehicleHal),
          mSubscriptionManager(std::bind(&ZoobrVehicleHalManager::onAllClientsUnsubscribed,
                                         this, std::placeholders::_1)) {
        init();
    }

    virtual ~ZoobrVehicleHalManager();

    void init();

    // ---------------------------------------------------------------------------------------------
    // Methods derived from IVehicle
    Return<void> getAllPropConfigs(getAllPropConfigs_cb _hidl_cb)  override;
    Return<void> getPropConfigs(const hidl_vec<int32_t>& properties,
                                getPropConfigs_cb _hidl_cb)  override;
    Return<void> get(const VehiclePropValue& requestedPropValue,
                     get_cb _hidl_cb)  override;
    Return<StatusCode> set(const VehiclePropValue& value)  override;
    Return<StatusCode> subscribe(const sp<IVehicleCallback>& callback,
                                const hidl_vec<SubscribeOptions>& options)  override;
    Return<StatusCode> unsubscribe(const sp<IVehicleCallback>& callback,
                                   int32_t propId)  override;
    Return<void> debugDump(debugDump_cb _hidl_cb = nullptr) override;

    Return<void> debug(const hidl_handle& fd, const hidl_vec<hidl_string>& options) override;

  private:
    // Set unit test class as friend class to test private functions.
    friend class VehicleHalManagerTestHelper;

    using VehiclePropValuePtr = VehicleHal::VehiclePropValuePtr;
    // Returns true if needs to call again shortly.
    using RetriableAction = std::function<bool()>;

    // ---------------------------------------------------------------------------------------------
    // Events received from VehicleHal
    void onHalEvent(VehiclePropValuePtr  v);
    void onHalPropertySetError(StatusCode errorCode, int32_t property,
                               int32_t areaId);

    // ---------------------------------------------------------------------------------------------
    // This method will be called from BatchingConsumer thread
    void onBatchHalEvent(const std::vector<VehiclePropValuePtr >& values);

    void handlePropertySetEvent(const VehiclePropValue& value);

    const VehiclePropConfig* getPropConfigOrNull(int32_t prop) const;

    bool checkWritePermission(const VehiclePropConfig &config) const;
    bool checkReadPermission(const VehiclePropConfig &config) const;
    void onAllClientsUnsubscribed(int32_t propertyId);

    // Dump and commands
    // TODO: most functions below (exception dump() and cmdSetOne()) should be const, but they rely
    // on IVehicle.get(), which isn't...
    void cmdDump(int fd, const hidl_vec<hidl_string>& options);
    void cmdDumpOneProperty(int fd, int32_t prop, int32_t areaId);
    void cmdDumpOneProperty(int fd, int rowNumber, const VehiclePropConfig& config);

    bool cmdSetOneProperty(int fd, const hidl_vec<hidl_string>& options);

    static bool checkArgumentsSize(int fd, const hidl_vec<hidl_string>& options, size_t minSize);
    static bool checkCallerHasWritePermissions(int fd);
    template <typename T>
    static bool safelyParseInt(int fd, int index, const std::string& s, T* out);
    static bool safelyParseFloat(int fd, int index, const std::string& s, float* out);
    // Parses "s" as a hex string and populate "*bytes". The hex string must be in the format of
    // valid hex format, e.g. "0xABCD".
    static bool parseHexString(int fd, const std::string& s, std::vector<uint8_t>* bytes);
    void cmdHelp(int fd) const;
    void cmdListAllProperties(int fd) const;
    void cmdDumpAllProperties(int fd);
    void cmdDumpSpecificProperties(int fd, const hidl_vec<hidl_string>& options);

    static bool isSubscribable(const VehiclePropConfig& config,
                               SubscribeFlags flags);
    static bool isSampleRateFixed(VehiclePropertyChangeMode mode);
    static float checkSampleRate(const VehiclePropConfig& config,
                                 float sampleRate);
    static ClientId getClientId(const sp<IVehicleCallback>& callback);

    // Parses the cmdline options for "--set" command. "*prop" would be populated with the
    // the properties to be set. Returns true when the cmdline options are valid, false otherwise.
    static bool parseSetPropOptions(int fd, const hidl_vec<hidl_string>& options,
                                    VehiclePropValue* prop);
    // Parses the options and get the values for the current option specified by "*index". "*index"
    // would advance to the next option field (e.g., the next "-f"). Returns a list of values for
    // the current option.
    static std::vector<std::string> getOptionValues(const hidl_vec<hidl_string>& options,
                                                    size_t* index);

  private:
    VehicleHal* mHal;
    std::unique_ptr<VehiclePropConfigIndex> mConfigIndex;
    SubscriptionManager mSubscriptionManager;

    hidl_vec<VehiclePropValue> mHidlVecOfVehiclePropValuePool;

    ConcurrentQueue<VehiclePropValuePtr> mEventQueue;
    BatchingConsumer<VehiclePropValuePtr> mBatchingConsumer;
    VehiclePropValuePool mValueObjectPool;
};

}  // namespace impl

}  // namespace V2_0
}  // namespace vehicle
}  // namespace automotive
}  // namespace hardware
}  // namespace android

#endif  // android_hardware_automotive_vehicle_V2_0_impl_DefaultVehicleHal_H_
