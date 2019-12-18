//
// Created by liyujiang on 2019/12/18.
// Adapted from https://github.com/quert999/DeviceObserver
//

#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "../common/log.h"
#include "../common/config.h"
#include "../common/CommonUtils.h"
#include "EmulatorDetector.h"

/**
 * 模拟器检测
 */
std::string detectEmulator(JNIEnv *env, std::string result) {
    result = "";
    if (isSpecialEmulator()) {
        result = result + "-SpecialDevice";
    }
    if (!checkContainBaseBand(env)) {
        result = result + "-NoBaseBand";
    }
    if (!checkContainCpuTemperature()) {
        result = result + "-NoTemperature";
    }
    if (checkError(env) < 0) {
        result = "-Error";
    }
    return result;
}

/**
 * 是否包含特殊标记的模拟器
 */
bool isSpecialEmulator() {
    const char *filePath[] = {
        "/system/bin/nemuVM-prop", //网易.MuMu模拟器
        "/system/bin/androVM-prop", //Genymotion、腾讯手游助手
        "/system/bin/ttVM-prop", //天天模拟器
        "/system/bin/3btrans", //51模拟器、蓝叠模拟器
        "/system/bin/droid4x-prop", //海马模拟器
        "/ueventd.nox.rc", //夜神模拟器(Windows)
        "/init.nox.rc", //夜神模拟器(MAC)
    };
    int len = sizeof(filePath) / sizeof(filePath[0]);
    for (int i = 0; i < len; ++i) {
        if (checkFileExists(filePath[i])) {
            return true;
        }
    }
    return false;
}

/**
 * 基带检测
 * 基带是手机上的一块电路板，因为模拟器没有真实的电路板（基带电路）
 * 部分真机在刷机失败的时候也会出现丢失基带的情况
 */
bool checkContainBaseBand(JNIEnv *env) {
    jclass systemClass = env->FindClass("android/os/SystemProperties");
    jmethodID getMethodID = env->GetStaticMethodID(systemClass, "get",
                                                   "(Ljava/lang/String;)Ljava/lang/String;");
    jstring key = env->NewStringUTF("gsm.version.baseband");
    jobject baseBand = (env->CallStaticObjectMethod(systemClass, getMethodID, key));
    if (baseBand == nullptr || env->GetStringLength((jstring) baseBand) == 0) {
        return false;
    } else {
        const char *dname = env->GetStringUTFChars((jstring) baseBand, 0);
        DEBUG("BASEBAND NAME  =  %s", dname);
        return true;
    }
}

/**
 * CPU温度文件检测
 *真机 thermal_zoneX
 *模拟器 cooling_deviceX  模拟器上没有thermal_zoneX目录
 */
bool checkContainCpuTemperature() {
    const char *path = "/sys/class/thermal/";
    DIR *dirptr = nullptr;
    struct dirent *entry;
    if ((dirptr = opendir(path)) != nullptr) {
        while ((entry = readdir(dirptr))) {
            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
                continue;
            }
            char *tmp = entry->d_name;
            if (strstr(tmp, "thermal_zone") != nullptr) {
                return true;
            }
        }
        closedir(dirptr);
        return false;
    } else {
        //没有文件列表-》权限问题
        //模拟器和真机存在权限问题,则返回true的情况
        DEBUG("Open thermal Fail");
        return true;
    }
}

//文件是否存在
bool checkFileExists(const char *filename) {
    struct stat statBuf{};
    //执行成功则返回0，失败返回-1
    int result = stat(filename, &statBuf) == 0 ? 1 : 0;
    if (result) {
        DEBUG("%s exists", filename);
        return true;
    }
    return false;
}

