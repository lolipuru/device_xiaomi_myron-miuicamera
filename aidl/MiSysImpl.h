#pragma once

#include <aidl/vendor/xiaomi/hardware/misys/common/BnMiSysImpl.h>

namespace aidl::vendor::xiaomi::hardware::misys::common {

class MiSysImpl : public BnMiSysImpl {
public:
    ndk::ScopedAStatus IsExists(const std::string& path, const std::string& file, bool* _aidl_return) override;
    ndk::ScopedAStatus MiSysCreateFolder(const std::string& path, const std::string& folder) override;
    ndk::ScopedAStatus MiSysCreateFolderMode(const std::string& path, const std::string& folder, int32_t perm) override;
    ndk::ScopedAStatus MiSysWriteBuffer(const std::string& path, const std::string& file, const std::vector<uint8_t>& buf, int64_t len) override;
    ndk::ScopedAStatus MiSysReadBuffer(const std::string& path, const std::string& file, std::vector<uint8_t>* _aidl_return) override;
    ndk::ScopedAStatus WriteToFile(const Ashmem& block, const std::string& path, const std::string& file, int64_t size) override;
    ndk::ScopedAStatus ReadFromFile(const Ashmem& block, const std::string& path, const std::string& file) override;
    ndk::ScopedAStatus EraseFileOrDirectory(const std::string& path, const std::string& file) override;
    ndk::ScopedAStatus GetFileSize(const std::string& path, const std::string& file, int64_t* _aidl_return) override;
    ndk::ScopedAStatus DirListFiles(const std::string& path, std::vector<FileInfo>* _aidl_return) override;
    ndk::ScopedAStatus CopyFile(const std::string& inpath, const std::string& outpath) override;

    // No-op / Virtual camera stubs:
    ndk::ScopedAStatus ConnectVirtualCamera(bool* _aidl_return) override { *_aidl_return = false; return ndk::ScopedAStatus::ok(); }
    ndk::ScopedAStatus DisconnectVirtualCamera(bool* _aidl_return) override { *_aidl_return = false; return ndk::ScopedAStatus::ok(); }
    ndk::ScopedAStatus OnFrameData(const std::vector<uint8_t>&, int32_t, bool* _aidl_return) override { *_aidl_return = false; return ndk::ScopedAStatus::ok(); }
    ndk::ScopedAStatus RegisterVCameraCallback(const std::shared_ptr<IVCameraCallback>&) override { return ndk::ScopedAStatus::ok(); }
    ndk::ScopedAStatus UnregisterVCameraCallback() override { return ndk::ScopedAStatus::ok(); }
    ndk::ScopedAStatus SetVirtualCameraConfig(int32_t, int32_t, double, bool* _aidl_return) override { *_aidl_return = false; return ndk::ScopedAStatus::ok(); }
    ndk::ScopedAStatus GetPartitionSize(const std::string&, const std::string&, int64_t* _aidl_return) override { *_aidl_return = 0; return ndk::ScopedAStatus::ok(); }
    ndk::ScopedAStatus MiSysReadFile(const std::string&, const std::string&, std::string* _aidl_return) override { *_aidl_return = ""; return ndk::ScopedAStatus::ok(); }
    ndk::ScopedAStatus MiSysWriteFile(const std::string&, const std::string&, const std::string&, int32_t, int8_t) override { return ndk::ScopedAStatus::ok(); }
    ndk::ScopedAStatus MiSysSetProp(const std::string&, const std::string&) override { return ndk::ScopedAStatus::ok(); }
};

} // namespace aidl::vendor::xiaomi::hardware::misys::common
