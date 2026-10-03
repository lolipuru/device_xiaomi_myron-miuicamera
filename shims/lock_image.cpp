/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android_media_Utils.h>
#include <ui/GraphicBuffer.h>
#include <utils/Errors.h>

#include <cstddef>
#include <utility>

namespace android {

#if defined(__LP64__)
// Match the LockedImage layout used by the arm64 blob.
static_assert(offsetof(LockedImage, data) == 0);
static_assert(offsetof(LockedImage, width) == 8);
static_assert(offsetof(LockedImage, height) == 12);
static_assert(offsetof(LockedImage, format) == 16);
static_assert(offsetof(LockedImage, stride) == 20);
static_assert(offsetof(LockedImage, flexFormat) == 72);
static_assert(offsetof(LockedImage, dataCb) == 80);
static_assert(offsetof(LockedImage, dataCr) == 88);
static_assert(offsetof(LockedImage, chromaStride) == 96);
static_assert(offsetof(LockedImage, chromaStep) == 100);
#endif

// myron OS4.0.0.33 libmedia_jni_utils.so, 0x4e40:
// allowOpaque enables YCbCr locking for IMPLEMENTATION_DEFINED buffers.
status_t lockImageFromBuffer(sp<GraphicBuffer> buffer, uint32_t usage,
                            const Rect& rect, int fenceFd,
                            CpuConsumer::LockedBuffer* output, bool allowOpaque) {
    if (buffer == nullptr || output == nullptr) {
        return BAD_VALUE;
    }
    if (!allowOpaque || buffer->getPixelFormat() != HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED) {
        return lockImageFromBuffer(std::move(buffer), usage, rect, fenceFd, output);
    }

    android_ycbcr ycbcr{};
    buffer->lockAsyncYCbCr(usage, rect, &ycbcr, fenceFd);
    void* data = ycbcr.y;
    if (data == nullptr) {
        const status_t result = buffer->lockAsync(usage, rect, &data, fenceFd);
        if (result != OK) {
            return result;
        }
    }

    output->data = static_cast<uint8_t*>(data);
    output->width = buffer->getWidth();
    output->height = buffer->getHeight();
    output->format = buffer->getPixelFormat();
    output->flexFormat = HAL_PIXEL_FORMAT_YCbCr_420_888;
    output->stride = ycbcr.y != nullptr ? static_cast<uint32_t>(ycbcr.ystride)
                                      : buffer->getStride();
    output->dataCb = static_cast<uint8_t*>(ycbcr.cb);
    output->dataCr = static_cast<uint8_t*>(ycbcr.cr);
    output->chromaStride = static_cast<uint32_t>(ycbcr.cstride);
    output->chromaStep = static_cast<uint32_t>(ycbcr.chroma_step);
    // The caller fills crop, transform, timestamp and frame metadata.
    return OK;
}

}  // namespace android
