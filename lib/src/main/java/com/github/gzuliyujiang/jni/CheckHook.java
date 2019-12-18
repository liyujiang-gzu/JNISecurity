package com.github.gzuliyujiang.jni;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.util.Log;

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * Adapted from https://github.com/quert999/DeviceObserver
 */
class CheckHook {

    static boolean isHook(Context context) {
        return isHookByPackageName(context) || isHookByStack() || isHookByJar();
    }

    private static boolean isHookByPackageName(Context context) {
        boolean isHook = false;
        PackageManager packageManager = context.getPackageManager();
        List<ApplicationInfo> applicationInfoList = packageManager.getInstalledApplications(PackageManager.GET_META_DATA);
        if (null == applicationInfoList) {
            return isHook;
        }
        for (ApplicationInfo applicationInfo : applicationInfoList) {
            if (applicationInfo.packageName.equals("de.robv.android.xposed.installer")) {
                Log.wtf("HookDetection", "Xposed found on the system.");
                isHook = true;
            }
            if (applicationInfo.packageName.equals("com.saurik.substrate")) {
                isHook = true;
                Log.wtf("HookDetection", "Substrate found on the system.");
            }
        }
        return isHook;
    }

    /**
     * 1. 如果存在Xposed框架hook  
     * （1）在dalvik.system.NativeStart.main方法后出现de.robv.android.xposed.XposedBridge.main调用  
     * （2）如果Xposed hook了调用栈里的一个方法，  
     * 还会有de.robv.android.xposed.XposedBridge.handleHookedMethod  
     * 和de.robv.android.xposed.XposedBridge.invokeOriginalMethodNative调用    
     * 2. 如果存在Substrate框架hook  
     * （1）dalvik.system.NativeStart.main调用后会出现2次com.android.internal.os.ZygoteInit.main，而不是一次。  
     * （2）如果Substrate hook了调用栈里的一个方法，  
     * 还会出现com.saurik.substrate.MS$2.invoked，com.saurik.substrate.MS$MethodPointer.invoke还有跟Substrate扩展相关的方法\
     */
    private static boolean isHookByStack() {
        boolean isHook = false;
        try {
            throw new Exception("blah");
        } catch (Exception e) {
            int zygoteInitCallCount = 0;
            for (StackTraceElement stackTraceElement : e.getStackTrace()) {
                if (stackTraceElement.getClassName().equals("com.android.internal.os.ZygoteInit")) {
                    zygoteInitCallCount++;
                    if (zygoteInitCallCount == 2) {
                        Log.wtf("HookDetection", "Substrate is active on the device.");
                        isHook = true;
                    }
                }
                if (stackTraceElement.getClassName().equals("com.saurik.substrate.MS$2") && stackTraceElement.getMethodName().equals("invoked")) {
                    Log.wtf("HookDetection", "A method on the stack trace has been hooked using Substrate.");
                    isHook = true;
                }
                if (stackTraceElement.getClassName().equals("de.robv.android.xposed.XposedBridge") && stackTraceElement.getMethodName().equals("main")) {
                    Log.wtf("HookDetection", "Xposed is active on the device.");
                    isHook = true;
                }
                if (stackTraceElement.getClassName().equals("de.robv.android.xposed.XposedBridge") && stackTraceElement.getMethodName().equals("handleHookedMethod")) {
                    Log.wtf("HookDetection", "A method on the stack trace has been hooked using Xposed.");
                    isHook = true;
                }
            }
        }
        return isHook;
    }

    private static boolean isHookByJar() {
        boolean isHook = false;
        try {
            Set<String> libraries = new HashSet();
            String mapsFilename = "/proc/" + android.os.Process.myPid() + "/maps";
            BufferedReader reader = new BufferedReader(new FileReader(mapsFilename));
            String line;
            while ((line = reader.readLine()) != null) {
                if (line.endsWith(".so") || line.endsWith(".jar")) {
                    int n = line.lastIndexOf(" ");
                    libraries.add(line.substring(n + 1));
                }
            }
            for (String library : libraries) {
                if (library.contains("com.saurik.substrate")) {
                    Log.wtf("HookDetection", "Substrate shared object found: " + library);
                    isHook = true;
                }
                if (library.contains("XposedBridge.jar")) {
                    Log.wtf("HookDetection", "Xposed JAR found: " + library);
                    isHook = true;
                }
            }
            reader.close();
        } catch (Exception e) {
            Log.wtf("HookDetection", e.toString());
        }
        return isHook;
    }

}
