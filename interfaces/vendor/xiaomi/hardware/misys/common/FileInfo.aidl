package vendor.xiaomi.hardware.misys.common;

@VintfStability
parcelable FileInfo {
    String name;
    long mtime = 0;
    long fileSize = 0;
}