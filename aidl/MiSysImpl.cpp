#include "MiSysImpl.h"
#include <android-base/logging.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

namespace aidl::vendor::xiaomi::hardware::misys::common {

using ndk::ScopedAStatus;

ScopedAStatus MiSysImpl::IsExists(const std::string& path, const std::string& file, bool* _aidl_return) {
    std::string fullPath = path + "/" + file;
    *_aidl_return = (access(fullPath.c_str(), F_OK) == 0);
    return ScopedAStatus::ok();
}

ScopedAStatus MiSysImpl::MiSysCreateFolder(const std::string& path, const std::string& folder) {
    return MiSysCreateFolderMode(path, folder, 0775);
}

ScopedAStatus MiSysImpl::MiSysCreateFolderMode(const std::string& path, const std::string& folder, int32_t perm) {
    std::string fullPath = path + "/" + folder;
    mkdir(fullPath.c_str(), perm);
    return ScopedAStatus::ok();
}

ScopedAStatus MiSysImpl::MiSysWriteBuffer(const std::string& path, const std::string& file, const std::vector<uint8_t>& buf, int64_t len) {
    std::string fullPath = path + "/" + file;
    int fd = open(fullPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if (fd >= 0) {
        write(fd, buf.data(), len);
        close(fd);
    }
    return ScopedAStatus::ok();
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
    std::string fullPath = path + "/" + file;
    int srcFd = block.fd.get();
    int dstFd = open(fullPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if (srcFd >= 0 && dstFd >= 0) {
        char buffer[8192];
        int64_t remaining = size;
        ssize_t bytesRead;
        while (remaining > 0 && (bytesRead = read(srcFd, buffer, std::min<int64_t>(sizeof(buffer), remaining))) > 0) {
            write(dstFd, buffer, bytesRead);
            remaining -= bytesRead;
        }
        close(dstFd);
    }
    return ScopedAStatus::ok();
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
    int src = open(inpath.c_str(), O_RDONLY);
    int dst = open(outpath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if (src >= 0 && dst >= 0) {
        char buf[8192];
        ssize_t n;
        while ((n = read(src, buf, sizeof(buf))) > 0) {
            write(dst, buf, n);
        }
        close(src);
        close(dst);
    }
    return ScopedAStatus::ok();
}

} // namespace aidl::vendor::xiaomi::hardware::misys::common
