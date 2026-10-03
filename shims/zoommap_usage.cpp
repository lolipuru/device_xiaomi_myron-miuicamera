/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "MiuiCameraZoomMap"

#include <cinttypes>
#include <hardware/gralloc.h>
#include <jni.h>
#include <log/log.h>
#include <surfacetexture/LegacySurfaceTexture.h>
#include <utils/Errors.h>
#include <utils/StrongPointer.h>

namespace android {
sp<LegacySurfaceTexture> SurfaceTexture_getSurfaceTexture(JNIEnv* env, jobject thiz);
}

namespace {

constexpr uint64_t kZoomMapUsage =
        GRALLOC_USAGE_HW_CAMERA_WRITE | GRALLOC_USAGE_RENDERSCRIPT;

}

extern "C" JNIEXPORT jint JNICALL
Java_com_android_camera_compat_ZoomMapUsage_setCameraOutputUsage(
        JNIEnv* env, jclass, jobject javaSurfaceTexture) {
    if (javaSurfaceTexture == nullptr) {
        return android::BAD_VALUE;
    }

    const auto surfaceTexture =
            android::SurfaceTexture_getSurfaceTexture(env, javaSurfaceTexture);
    if (surfaceTexture == nullptr) {
        return android::NO_INIT;
    }

    const android::status_t status =
            surfaceTexture->setConsumerUsageBits(kZoomMapUsage);
    if (status == android::OK) {
        ALOGI("ZoomMap usage set to 0x%" PRIx64, kZoomMapUsage);
    } else {
        ALOGE("Unable to set ZoomMap usage: %d", status);
    }
    return status;
}
