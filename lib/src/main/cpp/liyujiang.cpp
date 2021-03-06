// Created by liyujiang on 2019/12/18.
// Author 大定府羡民
//
#include <string>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/configuration.h>
#include "common/config.h"
#include "common/log.h"
#include "common/CommonUtils.h"
#include "common/BASE64.hpp"
#include "common/MD5.hpp"
#include "common/FileOper.h"
#include "json/JSON.hpp"
#include "emulator/EmulatorDetector.h"

// 对应的 Java 类
const char *JNI_CLASS_PATH = "com/github/gzuliyujiang/jni/JNISecurity";
// 需要被验证应用的包名
const char *APP_PACKAGE_NAME = "com.github.gzuliyujiang.jni.demo";
// 应用签名，注意开发版和发布版的区别，发布版需要使用正式签名打包后获取
const char *SIGNATURE_KEY = "ca82129355781c39d13dd243b0466db8";
// 服务端接口数据解密秘钥
const char *API_DECRYPT_KEY = "2019/12/19 02:00";
// MD5加盐
const char *MD5_KEY = "2019/12/18 11:35";
// Java获取签名信息的常量
const int GET_SIGNATURES = 0x00000040;
const int GET_SIGNING_CERTIFICATES = 0x08000000;

// 验证是否通过
static jboolean authPass = JNI_FALSE;

jstring getSysBaseInfo(JNIEnv *env, const char *key, const char *def) {
    try {
        jclass properClass = env->FindClass("android/os/SystemProperties");
        if (checkError(env) < 0) {
            return env->NewStringUTF("JError");
        }
        jmethodID methodId = env->GetStaticMethodID(properClass, "native_get",
                                                    "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
        if (checkError(env) < 0) {
            return env->NewStringUTF("JError2");
        }
        jstring jsKey = env->NewStringUTF(key);
        jstring jsDef = env->NewStringUTF(def);
        jobject result = env->CallStaticObjectMethod(properClass, methodId, jsKey, jsDef);
        if (checkError(env) < 0) {
            return env->NewStringUTF("JError3");
        }
        env->DeleteLocalRef(properClass);
        env->DeleteLocalRef(jsDef);
        env->DeleteLocalRef(jsKey);
        return (jstring) result;
    }
    catch (...) {
        if (checkError(env) < 0) {
            return env->NewStringUTF("JError3");
        }
        return env->NewStringUTF("NativeError");
    }
}

jstring getAndroidId(JNIEnv *env, jobject mContext) {
    if (mContext == nullptr) {
        return env->NewStringUTF("");
    }
    try {
        jclass contextClass = env->FindClass("android/content/Context");
        jmethodID getMethod = env->GetMethodID(contextClass, "getContentResolver",
                                               "()Landroid/content/ContentResolver;");
        jobject resolver = env->CallObjectMethod(mContext, getMethod);
        if (resolver == nullptr) {
            return env->NewStringUTF("ContentResolver is NULL");
        }
        jclass secClass = env->FindClass("android/provider/Settings$Secure");
        if (secClass == nullptr) {
            return env->NewStringUTF("secureClass is NULL");
        }
        jmethodID getStringMethod = env->GetStaticMethodID(secClass, "getString",
                                                           "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;");
        if (getStringMethod == nullptr) {
            return env->NewStringUTF("getStringMethod is NULL");
        }
        jfieldID ANDROID_ID = env->GetStaticFieldID(secClass, "ANDROID_ID", "Ljava/lang/String;");
        jobject str = (env->GetStaticObjectField(secClass, ANDROID_ID));
        jobject jId = (env->CallStaticObjectMethod(secClass, getStringMethod, resolver, str));
        return (jstring) jId;
    }
    catch (...) {
        return env->NewStringUTF("NativeError");
    }
}

jstring getImei(JNIEnv *env, jobject mContext) {
    if (mContext == nullptr) {
        return env->NewStringUTF("Context is NULL");
    }
    try {
        jclass cls_context = env->FindClass("android/content/Context");
        if (cls_context == nullptr) {
            return env->NewStringUTF("FindClass <android/content/Context> Error");
        }
        jmethodID getSystemService = env->GetMethodID(cls_context, "getSystemService",
                                                      "(Ljava/lang/String;)Ljava/lang/Object;");
        if (getSystemService == nullptr) {
            return env->NewStringUTF("getSystemService is NULL");
        }
        jfieldID TELEPHONY_SERVICE = env->GetStaticFieldID(cls_context, "TELEPHONY_SERVICE",
                                                           "Ljava/lang/String;");
        if (TELEPHONY_SERVICE == nullptr) {
            return env->NewStringUTF("TELEPHONY_SERVICE is NULL");
        }
        jobject jsTelService = env->GetStaticObjectField(cls_context, TELEPHONY_SERVICE);
        jobject telephonyManager = (env->CallObjectMethod(mContext, getSystemService,
                                                          jsTelService));
        if (checkError(env) < 0) {
            return env->NewStringUTF("Error1");
        }
        if (telephonyManager == nullptr) {
            return env->NewStringUTF("telephonyManager is NULL");
        }
        jclass cls_TelephoneManager = env->FindClass("android/telephony/TelephonyManager");
        if (checkError(env) < 0) {
            return env->NewStringUTF("Error2");
        }
        if (cls_TelephoneManager == nullptr) {
            return env->NewStringUTF("cls_TelephoneManager is NULL");
        }
        jmethodID getDeviceId = (env->GetMethodID(cls_TelephoneManager, "getDeviceId",
                                                  "()Ljava/lang/String;"));
        if (checkError(env) < 0) {
            return env->NewStringUTF("Error3");
        }
        jobject DeviceID = env->CallObjectMethod(telephonyManager, getDeviceId);
        if (checkError(env) < 0) {
            return env->NewStringUTF("Error4");
        }
        return (jstring) DeviceID;
    }
    catch (...) {
        return env->NewStringUTF("ErrorCatch");
    }
}

int isRooted() {
    try {
        const int expectResultSize = 256;
        char result[expectResultSize] = {0};
        // 查看su文件存在位置
        executeCMD("which su", result, expectResultSize);
        DEBUG("execute `which su` result  %s", result);
        int resultSize = strlen(result);
        if (resultSize > 1) {
            return 1;
        }
        return -1;
    }
    catch (...) {
        return -2;
    }
}

neb::JSON getHookInfo(JNIEnv *env, neb::JSON infoObj) {
    try {
        jobject context = getApplication(env);
        jclass clsCheckHook = env->FindClass(JNI_CLASS_PATH);
        jmethodID midIsHook = env->GetStaticMethodID(clsCheckHook, "isHook",
                                                     "(Landroid/content/Context;)Z");
        jboolean isHook = env->CallStaticBooleanMethod(clsCheckHook, midIsHook, context);
        if (checkError(env) < 0) {
            return infoObj;
        }
        infoObj.Add("hook", isHook);
        return infoObj;
    }
    catch (...) {
        return infoObj;
    }
}

neb::JSON getAppVersionInfo(JNIEnv *env, neb::JSON rootInfo) {
    try {
        jobject context = getApplication(env);
        jclass contextClass = env->FindClass("android/content/Context");
        jmethodID getPMId = env->GetMethodID(contextClass, "getPackageManager",
                                             "()Landroid/content/pm/PackageManager;");
        jobject pm = env->CallObjectMethod(context, getPMId);
        // 获取pi对象
        jclass pmClass = env->FindClass("android/content/pm/PackageManager");
        jmethodID getPIId = env->GetMethodID(pmClass, "getPackageInfo",
                                             "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
        std::string packageJS = getAppPackageName(env);
        jobject pi = env->CallObjectMethod(pm, getPIId, str2jstring(env, packageJS.c_str()), 0);
        //获取版本信息
        jclass piClass = env->FindClass("android/content/pm/PackageInfo");
        jfieldID jfdVerCode = env->GetFieldID(piClass, "versionCode", "I");
        int versionCodeI = env->GetIntField(pi, jfdVerCode);
        char strVersionCode[64];
        sprintf(strVersionCode, "%d", versionCodeI);
        jstring versionCodeJS = env->NewStringUTF(strVersionCode);
        std::string strVersion = jstring2str(env, versionCodeJS);
        rootInfo.Add("appVersionCode", strVersion);
        jfieldID JidVN = env->GetFieldID(piClass, "versionName", "Ljava/lang/String;");
        jobject versionName = env->GetObjectField(pi, JidVN);
        std::string verName = jstring2str(env, (jstring) versionName);
        rootInfo.Add("appVersionName", verName);
        env->DeleteLocalRef(contextClass);
        env->DeleteLocalRef(pm);
        env->DeleteLocalRef(pmClass);
        env->DeleteLocalRef(pi);
        env->DeleteLocalRef(piClass);

        return rootInfo;
    }
    catch (...) {
        return rootInfo;
    }
}

std::string getDeviceJson(JNIEnv *env) {
    neb::JSON infoObj;

    jobject context = getApplication(env);
    jclass contextClass = env->GetObjectClass(context);
    jmethodID getAssets = env->GetMethodID(contextClass, "getAssets",
                                           "()Landroid/content/res/AssetManager;");
    AAssetManager *mgr = AAssetManager_fromJava(env, env->CallObjectMethod(context, getAssets));
    struct AConfiguration *initConfig = AConfiguration_new();
    AConfiguration_fromAssetManager(initConfig, mgr);

    //系统平台
    infoObj.Add("platform", "Android");
    //系统SDK版本
    int32_t sdkVersion = AConfiguration_getSdkVersion(initConfig);
    infoObj.Add("sdkVersion", sdkVersion);
    //系统语言
    char language[6];
    memset(language, 0, 6);
    AConfiguration_getLanguage(initConfig, language);
    infoObj.Add("language", language);
    //屏幕宽高
    int32_t screenHeight = AConfiguration_getScreenHeightDp(initConfig);
    infoObj.Add("screenHeight", screenHeight);
    int32_t screenWidth = AConfiguration_getScreenWidthDp(initConfig);
    infoObj.Add("screenWidth", screenWidth);
    //系统国家代码
    char countryCode[6];
    memset(countryCode, 0, 6);
    AConfiguration_getCountry(initConfig, countryCode);
    infoObj.Add("country", countryCode);
    //移动国家代码
    int32_t mcc = AConfiguration_getMcc(initConfig);
    infoObj.Add("mcc", mcc);
    //移动网络代码
    int32_t mnc = AConfiguration_getMnc(initConfig);
    infoObj.Add("mnc", mnc);
    AConfiguration_delete(initConfig);
    // 系统版本
    jstring sysVersion = getSysBaseInfo(env, "ro.build.version.release", "");
    std::string strOSVersion = jstring2str(env, sysVersion);
    infoObj.Add("osVersion", strOSVersion);
    // 产品名称
    jstring jsDeviceName = getSysBaseInfo(env, "ro.product.device", "");
    std::string strDeviceName = jstring2str(env, jsDeviceName);
    infoObj.Add("deviceName", strDeviceName);
    // 产品品牌
    jstring brand = getSysBaseInfo(env, "ro.product.brand", "");
    std::string strBrand = jstring2str(env, brand);
    infoObj.Add("deviceBrand", strBrand);
    // 产品型号
    jstring model = getSysBaseInfo(env, "ro.product.model", "");
    std::string strModel = jstring2str(env, model);
    infoObj.Add("deviceModel", strModel);
    // 支持的CPU架构
    jstring jsSupportedAbi = getSysBaseInfo(env, "ro.product.cpu.abilist", "");
    std::string strSupportedAbi = jstring2str(env, jsSupportedAbi);
    infoObj.Add("supportedAbi", strSupportedAbi);
    // 硬件名称
    jstring jsHardware = getSysBaseInfo(env, "ro.hardware", "");
    std::string strHardware = jstring2str(env, jsHardware);
    infoObj.Add("hardware", strHardware);
    // 是否模拟器
    jstring jsQemu = getSysBaseInfo(env, "ro.kernel.qemu", "0");
    std::string strQemu = jstring2str(env, jsQemu);
    infoObj.Add("qemu", strQemu);
    // A build ID string meant for displaying to the user
    jstring jsDisplay = getSysBaseInfo(env, "ro.build.display.id", "");
    std::string strDisplay = jstring2str(env, jsDisplay);
    infoObj.Add("display", strDisplay);
    // 产品制造商
    jstring jsManufacturer = getSysBaseInfo(env, "ro.product.manufacturer", "");
    std::string strManufacture = jstring2str(env, jsManufacturer);
    infoObj.Add("manufacturer", strManufacture);
    // 序列号（需要READ_PHONE_STATE权限，可能获取不到）
    std::string strSerial = getSerial(env);
    infoObj.Add("serial", strSerial);
    // 应用签名
    std::string strSignId = getAppSignature(env);
    std::string strMd5 = toMD5(strSignId);
    infoObj.Add("appSignature", strMd5);
    // 应用包名
    std::string strPackageName = getAppPackageName(env);
    infoObj.Add("appBundleId", strPackageName);
    infoObj = getAppVersionInfo(env, infoObj);
    // 国际移动设备识别码（需要READ_PHONE_STATE权限，可能获取不到）
    jstring jsImei = getImei(env, context);
    std::string strImei = jstring2str(env, jsImei);
    infoObj.Add("imei", strImei);
    // 获取安卓ID（极个别设备获取不到数据或得到错误数据）
    jstring androidId = getAndroidId(env, context);
    std::string strAID = jstring2str(env, androidId);
    infoObj.Add("androidId", strAID);
    // 是否ROOT
    int rootInfo = isRooted();
    infoObj.Add("rooted", rootInfo);
    // 是否模拟器
    std::string emulatorResult;
    emulatorResult = detectEmulator(env, emulatorResult);
    infoObj.Add("emulator", emulatorResult);
    // 监测破解情况
    infoObj = getHookInfo(env, infoObj);

    std::string strResult = infoObj.ToString();
    if (checkError(env) < 0) {
        strResult = "";
    }
    return strResult;
}

void encryption(std::string &c, int a[]) {
    for (int i = 0, j = 0; c[j]; j++, i = (i + 1) % 7) {
        c[j] ^= a[i];
    }
}

void decryption(std::string &c, int *a) {
    for (int i = 0, j = 0; c[j]; j++, i = (i + 1) % 7) {
        c[j] ^= a[i];
    }
}

jstring getDeviceInfoEncrypt(JNIEnv *env) {
    std::string strResult = getDeviceJson(env);
    DEBUG("success: %s", strResult.c_str());
    BASE64 base64;
    std::string encodedResult = base64.encode(strResult);
    int a[] = {4, 9, 6, 2, 8, 7, 3};
    encryption(encodedResult, a);
    std::string sFlag = "a8;";
    std::string dFlag = "d0az";
    strResult = sFlag + encodedResult + dFlag;
    const char *cStr = strResult.c_str();
    DEBUG("success encrypt: %s", cStr);
    return env->NewStringUTF(cStr);
}

JNICALL jboolean verify(JNIEnv *env, jclass) {
    // c_str()的坑，参阅 https://blog.csdn.net/u013383344/article/details/53379029
    std::string stdPackageName = getAppPackageName(env);
    const char *charPackageName = stdPackageName.c_str();
    if (strcmp(charPackageName, APP_PACKAGE_NAME) != 0) {
        DEBUG("package name %s not equals %s", charPackageName, APP_PACKAGE_NAME);
        authPass = JNI_FALSE;
        return JNI_FALSE;
    }
    std::string stdSignature = getAppSignature(env);
    stdSignature = toMD5(stdSignature);
    const char *signature = stdSignature.c_str();
    DEBUG("current signature %s", signature);
    //DEBUG("reserve signature %s", SIGNATURE_KEY);
    if (strcmp(signature, SIGNATURE_KEY) == 0) {
        DEBUG("signature verification passed");
        authPass = JNI_TRUE;
    } else {
        DEBUG("signature verification failed");
        authPass = JNI_FALSE;
    }
    return authPass;
}

jboolean checkAuthPass(JNIEnv *env) {
    if (authPass) {
        return JNI_TRUE;
    } else {
        env->NewStringUTF("You don't have permission, the verification didn't pass.");
        return JNI_FALSE;
    }
}

JNICALL jstring getDeviceInfo(JNIEnv *env, jclass) {
    //return getDeviceInfoEncrypt(env);
    std::string strResult = getDeviceJson(env);
    return str2jstring(env, strResult.c_str());
}

JNICALL jstring getApiKey(JNIEnv *env, jclass) {
    if (checkAuthPass(env)) {
        return env->NewStringUTF(API_DECRYPT_KEY);
    } else {
        return env->NewStringUTF("");
    }
}

JNICALL jboolean encryptFile(JNIEnv *env, jclass, jstring sourcePath, jstring filePath) {
    return static_cast<jboolean>(fileEncrypt(env, sourcePath, filePath) > 0);
}

JNICALL jboolean decryptFile(JNIEnv *env, jclass, jstring sourceFile, jstring file) {
    return static_cast<jboolean>(fileDecrypt(env, sourceFile, file) > 0);
}

JNICALL jboolean divideFile(JNIEnv *env, jclass, jstring filePath, jstring path, jint count) {
    return static_cast<jboolean>(fileDivision(env, filePath, path, count) > 0);
}

JNICALL jboolean mergeFile(JNIEnv *env, jclass, jstring sourcePath, jobjectArray paths) {
    return static_cast<jboolean>(fileMerge(env, sourcePath, paths) > 0);
}

int registerNatives(JNIEnv *env) {
    JNINativeMethod registerMethods[] = {
        {"verify",        "()Z",                                      (jboolean *) verify},
        {"getDeviceInfo", "()Ljava/lang/String;",                     (jstring *) getDeviceInfo},
        {"getApiKey",     "()Ljava/lang/String;",                     (jstring *) getApiKey},
        {"encryptFile",   "(Ljava/lang/String;Ljava/lang/String;)Z",  (jboolean *) encryptFile},
        {"decryptFile",   "(Ljava/lang/String;Ljava/lang/String;)Z",  (jboolean *) decryptFile},
        {"divideFile",    "(Ljava/lang/String;Ljava/lang/String;I)Z", (jboolean *) divideFile},
        {"mergeFile",     "(Ljava/lang/String;[Ljava/lang/String;)Z", (jboolean *) mergeFile},
    };
    int methodsNum = sizeof(registerMethods) / sizeof(registerMethods[0]);
    return registerNativeMethods(env, JNI_CLASS_PATH, registerMethods, methodsNum);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *) {
    DEBUG("Dynamic library was load");
    JNIEnv *env = nullptr;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    assert(env != nullptr);
    if (!registerNatives(env)) {
        DEBUG("register native methods failed");
        return JNI_ERR;
    }
    DEBUG("register native methods success");
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *, void *) {
    DEBUG("Dynamic library was unload");
}
