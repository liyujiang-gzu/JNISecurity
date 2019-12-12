package com.github.gzuliyujiang.jni;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;

/**
 * 私密数据存储部分：用来获取解密秘钥，内含签名防盗机制
 * 文件加密解密部分：用来加密解密、切割合并文件
 */
public class JNISecurity {

    static {
        System.loadLibrary("liyujiang");
    }

    /**
     * 初始化并判断当前 APP 是否为合法应用，只需调用一次
     *
     * @return 返回 true 则初始化成功并当前 APP 为合法应用
     */
    public static native boolean init();

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