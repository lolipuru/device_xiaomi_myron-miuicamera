#include "MiSysImpl.h"
#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/unique_fd.h>
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <memory>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

namespace aidl::vendor::xiaomi::hardware::misys::common {

using ndk::ScopedAStatus;
using android::base::unique_fd;

namespace {

constexpr char kWatermarkDirectory[] = "/data/vendor/camera/watermarks";

std::string filePath(const std::string& path, const std::string& file = "") {
    return std::filesystem::path(file.empty() ? path : path + "/" + file)
            .lexically_normal().string();
}

bool isWatermarkPath(const std::string& path) {
    return path == kWatermarkDirectory ||
           path.compare(0, sizeof(kWatermarkDirectory),
                        std::string(kWatermarkDirectory) + "/") == 0;
}

ScopedAStatus ioError(const std::string& operation, const std::string& path, int error = errno) {
    if (error == 0) error = EIO;
    const std::string message = operation + " " + path + ": " + strerror(error);
    LOG(ERROR) << message;
    return ScopedAStatus::fromServiceSpecificErrorWithMessage(error, message.c_str());
}

bool setWatermarkMode(int fd, bool directory) {
    // Init uses umask 077; the camera HAL needs access through group camera.
    return fchmod(fd, directory ? 0770 : 0660) == 0;
}

bool repairWatermarkPath(const std::string& path) {
    if (!isWatermarkPath(path)) return true;
    unique_fd fd(open(path.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW));
    if (!fd.ok()) return false;
    struct stat st;
    return fstat(fd, &st) == 0 && setWatermarkMode(fd, S_ISDIR(st.st_mode));
}

void repairWatermarkDirectory(int parentFd, const char* name) {
    unique_fd fd(openat(parentFd, name, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW));
    if (!fd.ok()) {
        if (errno != ENOENT) PLOG(ERROR) << "Open watermark directory " << name;
        return;
    }
    if (!setWatermarkMode(fd, true)) {
        PLOG(ERROR) << "Set watermark directory permissions " << name;
        return;
    }
    const int directoryFd = fd.release();
    DIR* rawDir = fdopendir(directoryFd);
    if (rawDir == nullptr) {
        PLOG(ERROR) << "Read watermark directory " << name;
        close(directoryFd);
        return;
    }
    std::unique_ptr<DIR, decltype(&closedir)> dir(rawDir, closedir);
    while (dirent* entry = readdir(dir.get())) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
        struct stat st;
        if (fstatat(dirfd(dir.get()), entry->d_name, &st, AT_SYMLINK_NOFOLLOW) != 0) {
            PLOG(ERROR) << "Stat watermark file " << entry->d_name;
            continue;
        }
        if (S_ISDIR(st.st_mode)) {
            repairWatermarkDirectory(dirfd(dir.get()), entry->d_name);
        } else if (S_ISREG(st.st_mode)) {
            unique_fd file(openat(dirfd(dir.get()), entry->d_name,
                                  O_RDONLY | O_CLOEXEC | O_NOFOLLOW));
            if (!file.ok() || !setWatermarkMode(file, false)) {
                PLOG(ERROR) << "Set watermark file permissions " << entry->d_name;
            }
        }
    }
}

bool copyBytes(int src, int dst, int64_t size) {
    char buffer[8192];
    for (int64_t offset = 0; offset < size;) {
        const size_t count = std::min<int64_t>(sizeof(buffer), size - offset);
        if (!android::base::ReadFullyAtOffset(src, buffer, count, offset) ||
            !android::base::WriteFully(dst, buffer, count)) {
            if (errno == 0) errno = EIO;
            return false;
        }
        offset += count;
    }
    return true;
}

template <typename Writer>
ScopedAStatus writeFile(const std::string& path, Writer writer) {
    const bool watermark = isWatermarkPath(path);
    std::string temporaryPath = path + ".tmp.XXXXXX";
    unique_fd fd(watermark ? mkostemp(temporaryPath.data(), O_CLOEXEC)
                          : open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0664));
    if (!fd.ok()) return ioError("Open output", path);
    if ((watermark && !setWatermarkMode(fd, false)) || !writer(fd)) {
        const int error = errno;
        if (watermark) unlink(temporaryPath.c_str());
        return ioError("Write output", path, error);
    }
    if (watermark && rename(temporaryPath.c_str(), path.c_str()) != 0) {
        const int error = errno;
        unlink(temporaryPath.c_str());
        return ioError("Replace output", path, error);
    }
    return ScopedAStatus::ok();
}

} // namespace

MiSysImpl::MiSysImpl() {
    repairWatermarkDirectory(AT_FDCWD, kWatermarkDirectory);
}

ScopedAStatus MiSysImpl::IsExists(const std::string& path, const std::string& file, bool* _aidl_return) {
    const std::string fullPath = filePath(path, file);
    *_aidl_return = (access(fullPath.c_str(), F_OK) == 0);
    if (*_aidl_return && !repairWatermarkPath(fullPath)) {
        return ioError("Set permissions", fullPath);
    }
    return ScopedAStatus::ok();
}

ScopedAStatus MiSysImpl::MiSysCreateFolder(const std::string& path, const std::string& folder) {
    return MiSysCreateFolderMode(path, folder, 0775);
}

ScopedAStatus MiSysImpl::MiSysCreateFolderMode(const std::string& path, const std::string& folder, int32_t perm) {
    const std::string fullPath = filePath(path, folder);
    if (mkdir(fullPath.c_str(), perm) != 0 && errno != EEXIST) {
        return ioError("Create directory", fullPath);
    }
    if (!repairWatermarkPath(fullPath)) return ioError("Set permissions", fullPath);
    return ScopedAStatus::ok();
}

ScopedAStatus MiSysImpl::MiSysWriteBuffer(const std::string& path, const std::string& file, const std::vector<uint8_t>& buf, int64_t len) {
    const std::string fullPath = filePath(path, file);
    if (len < 0 || static_cast<uint64_t>(len) > buf.size()) {
        return ioError("Invalid buffer length", fullPath, EINVAL);
    }
    return writeFile(fullPath, [&](int fd) { return android::base::WriteFully(fd, buf.data(), len); });
}

ScopedAStatus MiSysImpl::MiSysReadBuffer(const std::string& path, const std::string& file, std::vector<uint8_t>* _aidl_return) {
    std::string fullPath = path + "/" + file;
    int fd = open(fullPath.c_str(), O_RDONLY);
    if (fd >= 0) {
        struct stat st;
        if (fstat(fd, &st) == 0 && st.st_size > 0) {
            _aidl_return->resize(st.st_size);
            read(fd, _aidl_return->data(), st.st_size);
        }
        close(fd);
    }
    return ScopedAStatus::ok();
}

ScopedAStatus MiSysImpl::WriteToFile(const Ashmem& block, const std::string& path, const std::string& file, int64_t size) {
    const std::string fullPath = filePath(path, file);
    if (block.fd.get() < 0 || size < 0 || size > block.size) {
        return ioError("Invalid shared memory", fullPath, EINVAL);
    }
    return writeFile(fullPath, [&](int fd) { return copyBytes(block.fd.get(), fd, size); });
}

ScopedAStatus MiSysImpl::ReadFromFile(const Ashmem& block, const std::string& path, const std::string& file) {
    std::string fullPath = path + "/" + file;
    int srcFd = open(fullPath.c_str(), O_RDONLY);
    int dstFd = block.fd.get();
    if (srcFd >= 0 && dstFd >= 0) {
        char buffer[8192];
        ssize_t bytesRead;
        while ((bytesRead = read(srcFd, buffer, sizeof(buffer))) > 0) {
            write(dstFd, buffer, bytesRead);
        }
        close(srcFd);
    }
    return ScopedAStatus::ok();
}

ScopedAStatus MiSysImpl::EraseFileOrDirectory(const std::string& path, const std::string& file) {
    std::string fullPath = path + "/" + file;
    unlink(fullPath.c_str());
    rmdir(fullPath.c_str());
    return ScopedAStatus::ok();
}

ScopedAStatus MiSysImpl::GetFileSize(const std::string& path, const std::string& file, int64_t* _aidl_return) {
    std::string fullPath = path + "/" + file;
    struct stat st;
    *_aidl_return = (stat(fullPath.c_str(), &st) == 0) ? st.st_size : 0;
    return ScopedAStatus::ok();
}

ScopedAStatus MiSysImpl::DirListFiles(const std::string& path, std::vector<FileInfo>* _aidl_return) {
    DIR* dir = opendir(path.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_name[0] != '.') {
                FileInfo info;
                info.name = entry->d_name;
                _aidl_return->push_back(info);
            }
        }
        closedir(dir);
    }
    return ScopedAStatus::ok();
}

ScopedAStatus MiSysImpl::CopyFile(const std::string& inpath, const std::string& outpath) {
    unique_fd src(open(inpath.c_str(), O_RDONLY | O_CLOEXEC));
    if (!src.ok()) return ioError("Open input", inpath);
    struct stat st;
    if (fstat(src, &st) != 0) return ioError("Stat input", inpath);
    return writeFile(filePath(outpath), [&](int dst) { return copyBytes(src, dst, st.st_size); });
}

} // namespace aidl::vendor::xiaomi::hardware::misys::common
