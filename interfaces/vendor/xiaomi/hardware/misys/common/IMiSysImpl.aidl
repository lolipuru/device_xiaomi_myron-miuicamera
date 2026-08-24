package vendor.xiaomi.hardware.misys.common;

import vendor.xiaomi.hardware.misys.common.Ashmem;
import vendor.xiaomi.hardware.misys.common.FileInfo;
import vendor.xiaomi.hardware.misys.common.IVCameraCallback;

@VintfStability
interface IMiSysImpl {
    FileInfo[] DirListFiles(String path);
    void EraseFileOrDirectory(String path, String file_name);
    String MiSysReadFile(String path, String file_name);
    void MiSysWriteFile(String path, String file_name, String writebuf, int sbuf_len, byte append_data);
    boolean IsExists(String path, String file_name);
    void MiSysCreateFolder(String path, String folder_name);
    byte[] MiSysReadBuffer(String path, String file_name);
    void MiSysWriteBuffer(String path, String file_name, in byte[] writebuf, long buf_len);
    boolean SetVirtualCameraConfig(int width, int height, double frameRate);
    boolean ConnectVirtualCamera();
    boolean DisconnectVirtualCamera();
    boolean OnFrameData(in byte[] data, int len);
    void RegisterVCameraCallback(IVCameraCallback callback);
    void UnregisterVCameraCallback();
    long GetFileSize(String path, String fileName);
    void MiSysSetProp(String key, String value);
    void ReadFromFile(in Ashmem block, String path, String fileName);
    void WriteToFile(in Ashmem block, String path, String fileName, long size);
    long GetPartitionSize(String path, String partitionName);
    void MiSysCreateFolderMode(String path, String folder_name, int permission);
    void CopyFile(String inpath, String outpath);
}
