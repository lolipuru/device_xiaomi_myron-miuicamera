#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include "MiSysImpl.h"
#include "HypSysIntl.h"

using aidl::vendor::xiaomi::hardware::misys::common::MiSysImpl;
using aidl::vendor::xiaomi::hardware::misys::common::HypSysIntlImpl;

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(4);
    ABinderProcess_startThreadPool();

    // IMiSysImpl
    auto misysService = ndk::SharedRefBase::make<MiSysImpl>();
    const std::string misysName = std::string(MiSysImpl::descriptor) + "/default";
    binder_status_t status1 = AServiceManager_addService(misysService->asBinder().get(), misysName.c_str());
    CHECK_EQ(status1, STATUS_OK) << "Failed to register " << misysName;

    // IHypSysIntl
    auto hypSysService = ndk::SharedRefBase::make<HypSysIntlImpl>();
    const std::string hypSysName = std::string(HypSysIntlImpl::descriptor) + "/default";
    binder_status_t status2 = AServiceManager_addService(hypSysService->asBinder().get(), hypSysName.c_str());
    CHECK_EQ(status2, STATUS_OK) << "Failed to register " << hypSysName;

    LOG(INFO) << "Registered IMiSysImpl and IHypSysIntl successfully";

    ABinderProcess_joinThreadPool();
    return 0;
}
