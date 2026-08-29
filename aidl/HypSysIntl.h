#pragma once

#include <aidl/vendor/xiaomi/hardware/misys/common/BnHypSysIntl.h>

namespace aidl::vendor::xiaomi::hardware::misys::common {

class HypSysIntlImpl : public BnHypSysIntl {
public:
    ndk::ScopedAStatus transferMessage(const std::string& /*action*/, const std::string& /*jsonString*/) override {
        return ndk::ScopedAStatus::ok();
    }
};

} // namespace aidl::vendor::xiaomi::hardware::misys::common
