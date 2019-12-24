package com.github.gzuliyujiang.jni;

import android.annotation.SuppressLint;
import android.app.Application;
import android.content.Context;
import android.os.Build;

/**
 * 私密数据存储部分：用来获取解密秘钥，内含签名防盗机制
 * 文件加密解密部分：用来加密解密、切割合并文件
 * 设备唯一标识部分：用来标记唯一的用户终端设备
 */
@SuppressWarnings("unused")
public class JNISecurity {

    static {
        System.loadLibrary("liyujiang");
    }

    private JNISecurity() {
        super();
    }

    /**
     * 初始化并判断当前 APP 是否为合法应用，只需调用一次
     *
     * @return 返回 true 则初始化成功并当前 APP 为合法应用
     */
    public static native boolean verify();

    /**
     * 已加密的终端设备信息
     */
    public static native String getDeviceInfo();

    /**
     * 服务端接口数据解密秘钥
     */
    public static native String getApiKey();

    /**
     * 加密文件
     */
    public static native boolean encryptFile(String sourcePath, String filePath);


    /**
     * 解密文件
     */
    public static native boolean decryptFile(String sourcePath, String filePath);

    /**
     * 分割文件
     */
    public static native boolean divideFile(String sourcePath, String name, int count);

    /**
     * 合并文件
     */
    public static native boolean mergeFile(String mergePath, String[] filePaths);

    //********************  以下方法供C/C++调用，不可混淆或移除 ****************************

    public static boolean isNotEmpty(String str) {
        return str != null && str.length() >= 1;
    }

    public static boolean isHook(Context context) {
        return CheckHook.isHook(context);
    }

    @SuppressLint("HardwareIds")
    public synchronized static String getSerial() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
            //noinspection deprecation
            return Build.SERIAL;
        }
        try {
            return Build.getSerial();
        } catch (SecurityException e) {
            return "SecurityException";
        }
    }

}