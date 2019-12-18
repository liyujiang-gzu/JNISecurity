package com.github.gzuliyujiang.jni;

import android.annotation.SuppressLint;
import android.app.Application;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;
import android.content.res.AssetManager;
import android.os.Build;
import android.os.Environment;

import java.io.File;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

/**
 * 私密数据存储部分：用来获取解密秘钥，内含签名防盗机制
 * 文件加密解密部分：用来加密解密、切割合并文件
 */
public class JNISecurity {
    private static boolean hasInit = false;
    private static Application appContext = null;

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

    public static void initInApplication(Application app) {
        if (hasInit) {
            // 避免重复初始化
            return;
        }
        appContext = app;
        hasInit = true;
    }

    public static native String getEncodeDeviceInfo();


    //********************  以下方法供C/C++调用，不可混淆或移除 **************************************

    public static Application getAppContext() {
        return appContext;
    }

    public static boolean isNonEmpty(String str) {
        return str != null && str.length() >= 1;
    }

    public static boolean isHook() {
        return CheckHook.isHook(appContext);
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

    public static String getAppName(Context context) {
        try {
            PackageManager packageManager = context.getPackageManager();
            PackageInfo packageInfo = packageManager.getPackageInfo(
                    context.getPackageName(), 0);
            int labelRes = packageInfo.applicationInfo.labelRes;
            return context.getResources().getString(labelRes);
        } catch (Exception e) {
            return "Exception";
        }
    }

    public static String md5(String str) {
        try {
            MessageDigest instance = MessageDigest.getInstance("MD5");
            instance.update(str.getBytes());
            byte[] digest = instance.digest();
            StringBuilder stringBuilder = new StringBuilder();
            for (byte b : digest) {
                stringBuilder.append(Integer.toHexString(b >> 4 & 15));
                stringBuilder.append(Integer.toHexString(b & 15));
            }
            return stringBuilder.toString();
        } catch (NoSuchAlgorithmException e) {
            return "";
        }
    }

    private static native boolean saveStoredId(String rootPath, AssetManager assetManager, String id);

    public static boolean saveKey(String id) {
        File rootFile = Environment.getExternalStorageDirectory();
        return saveStoredId(rootFile.getPath(), appContext.getAssets(), id);
    }

    /**
     * 获取 Key
     *
     * @return return key
     */
    public static native String getKey();

    /**
     * 获取应用签名，获取签名后替换 cpp 的 SIGNATURE_KEY 常量
     */
    public static String getSign(Context context) {
        String pkgName = context.getApplicationContext().getPackageName();
        try {
            @SuppressLint("PackageManagerGetSignatures")
            PackageInfo packageInfo = context.getPackageManager().getPackageInfo(pkgName, PackageManager.GET_SIGNATURES);
            Signature[] signatures = packageInfo.signatures;
            return signatures[0].toCharsString();
        } catch (Exception ignore) {
        }
        return "";
    }

    /**
     * 文件加密
     */
    public static native int fileEncrypt(String sourcePath, String filePath);


    /**
     * 文件解密
     */
    public static native int fileDecrypt(String sourcePath, String filePath);


    /**
     * 文件分割
     */
    public static native int fileDivision(String sourcePath, String name, int count);

    /**
     * 文件合并
     */
    public static native int fileMerge(String mergePath, String[] filePaths);

}