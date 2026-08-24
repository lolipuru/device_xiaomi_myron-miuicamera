package vendor.xiaomi.hardware.misys.common;

import android.os.ParcelFileDescriptor;

@VintfStability
parcelable Ashmem {
    ParcelFileDescriptor fd;
    long size;
}