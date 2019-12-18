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
#include "common/EmulatorDetector.h"
#include "json/JSON.hpp"

// 对应的 Java 类
const char *JNI_CLASS_PATH = "com/github/gzuliyujiang/jni/JNISecurity";
// 需要被验证应用的包名
const char *APP_PACKAGE_NAME = "com.github.gzuliyujiang.jni.demo";
// 应用签名，注意开发版和发布版的区别，发布版需要使用正式签名打包后获取
const char *SIGNATURE_KEY = "308202e2308201ca020101300d06092a864886f70d010105050030373116301406035504030c0d416e64726f69642044656275673110300e060355040a0c07416e64726f6964310b3009060355040613025553301e170d3139303932353032333330325a170d3439303931373032333330325a30373116301406035504030c0d416e64726f69642044656275673110300e060355040a0c07416e64726f6964310b300906035504061302555330820122300d06092a864886f70d01010105000382010f003082010a0282010100a2f642a5f8363b805c5b71718c7d3aa67a7c4f5445094198a4c3c84c0460773b6451c2103a164a344c77fced2c96c0c086c975d46629818df667d585dc80fbf97d32d83ef12f2436e64b6ec1535dc08cd0d508f0a91ad0f45e012fbca56aff0ab7c03821295678fd9833de7fd300ea6a28db4754c89abd30aaa3e2822c6ec38f887dfb90672b087d71ce3ec5f97268187c78cfe8ceebc21a6c1fd99ffa96e4ea153e95edd6cac6bd0d82379c3ca20a2b808e03e93289406cc7d5caf5d5fb1d13c690b809fee52a20783e6e25ed922cdb620df1be9a37da27cce96ed5ec160bbba2384a9e881cbe857021b56c568cdd14021db6459ec5bcca20e8461f343069ab0203010001300d06092a864886f70d010105050003820101007339d75da7db27e85ba6939485bcfd96f3db654e57de3064b264ef502b60434df8b4669ef1a2ab0d96e37a9a228090c63a81b25a9f007e8ddc59b308ca66e38a5a876dafb71b4699bb9330126f99575f15ec9f17c481bde9dd7250f8f53095c02d782cbc18509c5fa6742025af2fb43b021bc90f0875bb72179dc68880ebdc2e6cc8adb2b193cb17c1fd0587689e9a9cf5c49f79c2998adbb18eaf59f06bfd34e2503ceaa9bddeb77a9e9d41e78fa6f09bedf7df2e1274fa5aa8b73ba3daea626512ee924df3a369c8d2db8da932feb9be82bd2e84e292a7ed5f0f3b15bd4b9e82b8f5103363b6570aad4808192dd9035dde3dff072063ec1a3f7869d3fddabe";
// 需要被保护的密钥，请修改成你自己的密钥
const char *DECRYPT_KEY = "successful return key!";
// MD5加盐
const char *MD5_KEY = "2019/12/18 11:35";

// 验证是否通过
static jboolean auth = JNI_FALSE;

/**
 * 获取系统的基础信息
 * @param env  jvm环境
 * @param key
 * @param def  defaultvalue
 * @return
 */
jstring getSysBaseInfo(JNIEnv *env, const char *key, const char *def)
{
    try
    {
        jclass properClass = env->FindClass("android/os/SystemProperties");
        if (checkError(env) < 0)
        {
            return env->NewStringUTF("JError");
        }
        jmethodID methodId = env->GetStaticMethodID(properClass, "native_get",
                                                    "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
        if (checkError(env) < 0)
        {
            return env->NewStringUTF("JError2");
        }
        jstring jsKey = env->NewStringUTF(key);
        jstring jsDef = env->NewStringUTF(def);
        jobject result = env->CallStaticObjectMethod(properClass, methodId, jsKey, jsDef);
        if (checkError(env) < 0)
        {
            return env->NewStringUTF("JError3");
        }
        env->DeleteLocalRef(properClass);
        env->DeleteLocalRef(jsDef);
        env->DeleteLocalRef(jsKey);
        return (jstring)result;
    }
    catch (...)
    {
        if (checkError(env) < 0)
        {
            return env->NewStringUTF("JError3");
        }
        return env->NewStringUTF("NativeError");
    }
}

/**
 * 获取包名
 * @param env
 * @param context
 * @return
 */
jstring getPackageName(JNIEnv *env, jobject context)
{

    try
    {
        jclass contextClass = env->FindClass("android/content/Context");
        jmethodID getPackageNameId = env->GetMethodID(contextClass, "getPackageName",
                                                      "()Ljava/lang/String;");
        jobject packNameString = env->CallObjectMethod(context, getPackageNameId);
        env->DeleteLocalRef(contextClass);
        return (jstring)packNameString;
    }
    catch (...)
    {
        return env->NewStringUTF("NativeError");
    }
}

/**
 * 获取android id
 * @param env
 * @param mContext
 * @return
 */
jstring get_android_id(JNIEnv *env, jobject mContext)
{
    if (mContext == nullptr)
    {
        return env->NewStringUTF("");
    }
    try
    {
        jclass contextClass = env->FindClass("android/content/Context");
        jmethodID getMethod = env->GetMethodID(contextClass, "getContentResolver",
                                               "()Landroid/content/ContentResolver;");
        jobject resolver = env->CallObjectMethod(mContext, getMethod);
        if (resolver == nullptr)
        {
            return env->NewStringUTF("ContentResolver is NULL");
        }
        jclass secClass = env->FindClass("android/provider/Settings$Secure");
        if (secClass == nullptr)
        {
            return env->NewStringUTF("secureClass is NULL");
        }
        jmethodID getStringMethod = env->GetStaticMethodID(secClass, "getString",
                                                           "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;");
        if (getStringMethod == nullptr)
        {
            return env->NewStringUTF("getStringMethod is NULL");
        }
        jfieldID ANDROID_ID = env->GetStaticFieldID(secClass,
                                                    "ANDROID_ID", "Ljava/lang/String;");
        jstring str = (jstring)(env->GetStaticObjectField(secClass, ANDROID_ID));
        jstring jId = (jstring)(env->CallStaticObjectMethod(secClass, getStringMethod, resolver,
                                                            str));
        return jId;
    }
    catch (...)
    {
        return env->NewStringUTF("NativeError");
    }
}

/**
 * 获取包名的版本
 * @param env
 * @param context
 * @return
 */
neb::JSON getPackageVersionInfo(JNIEnv *env, jobject context, neb::JSON rootInfo)
{
    try
    {
        // 获取pm对象
        jclass contextClass = env->FindClass("android/content/Context");
        jmethodID getPMId = env->GetMethodID(contextClass, "getPackageManager",
                                             "()Landroid/content/pm/PackageManager;");
        jobject pm = env->CallObjectMethod(context, getPMId);
        // 获取pi对象
        jclass pmClass = env->FindClass("android/content/pm/PackageManager");
        jmethodID getPIId = env->GetMethodID(pmClass, "getPackageInfo",
                                             "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
        jstring packageJS = getPackageName(env, context);

        jobject pi = env->CallObjectMethod(pm, getPIId, packageJS, 0);
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
        std::string verName = jstring2str(env, (jstring)versionName);
        rootInfo.Add("appVersionName", verName);
        env->DeleteLocalRef(contextClass);
        env->DeleteLocalRef(pm);
        env->DeleteLocalRef(pmClass);
        env->DeleteLocalRef(packageJS);
        env->DeleteLocalRef(pi);
        env->DeleteLocalRef(piClass);

        return rootInfo;
    }
    catch (...)
    {
        return rootInfo;
    }
}

/**
 * 获取包名的版本
 * @param env
 * @param context
 * @return
 */
jstring getPackageVersionName(JNIEnv *env, jobject context)
{
    try
    {
        // 获取pm对象
        jclass contextClass = env->FindClass("android/content/Context");
        jmethodID getPMId = env->GetMethodID(contextClass, "getPackageManager",
                                             "()Landroid/content/pm/PackageManager;");
        jobject pm = env->CallObjectMethod(context, getPMId);
        // 获取pi对象
        jclass pmClass = env->FindClass("android/content/pm/PackageManager");
        jmethodID getPIId = env->GetMethodID(pmClass, "getPackageInfo",
                                             "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
        jstring packageJS = getPackageName(env, context);
        jobject pi = env->CallObjectMethod(pm, getPIId, packageJS, 0);
        //获取版本信息
        jclass piClass = env->FindClass("android/content/pm/PackageInfo");

        jfieldID JidVN = env->GetFieldID(piClass, "versionName", "Ljava/lang/String;");
        jobject versionName = env->GetObjectField(pi, JidVN);
        env->DeleteLocalRef(contextClass);
        env->DeleteLocalRef(pm);
        env->DeleteLocalRef(pmClass);
        env->DeleteLocalRef(packageJS);
        env->DeleteLocalRef(pi);
        env->DeleteLocalRef(piClass);
        return (jstring)versionName;
    }
    catch (...)
    {
        return env->NewStringUTF("NativeError");
    }
}

/**
 * 获取app签名信息
 * @param env
 * @param mContext
 * @return
 */
jstring
get_sign_id(JNIEnv *env, jobject mContext)
{

    try
    {

        jclass context_class = env->GetObjectClass(mContext);
        jmethodID methodId = env->GetMethodID(context_class, "getPackageManager",
                                              "()Landroid/content/pm/PackageManager;");
        jobject package_manager_object = env->CallObjectMethod(mContext, methodId);

        if (package_manager_object == NULL)
        {
            DEBUG("getPackageManager() Failed!");
            return NULL;
        }

        //context.getPackageName()
        methodId = env->GetMethodID(context_class, "getPackageName", "()Ljava/lang/String;");
        jstring package_name_string = (jstring)env->CallObjectMethod(mContext, methodId);
        if (package_name_string == NULL)
        {
            DEBUG("getPackageName() Failed!");
            return NULL;
        }

        env->DeleteLocalRef(context_class);

        jclass pack_manager_class = env->GetObjectClass(package_manager_object);
        methodId = env->GetMethodID(pack_manager_class, "getPackageInfo",
                                    "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
        env->DeleteLocalRef(pack_manager_class);
        jobject package_info_object = env->CallObjectMethod(package_manager_object, methodId,
                                                            package_name_string, 64);
        if (package_info_object == NULL)
        {
            return NULL;
        }

        env->DeleteLocalRef(package_manager_object);

        //PackageInfo.signatures[0]
        jclass package_info_class = env->GetObjectClass(package_info_object);
        jfieldID fieldId = env->GetFieldID(package_info_class, "signatures",
                                           "[Landroid/content/pm/Signature;");
        env->DeleteLocalRef(package_info_class);
        jobjectArray signature_object_array = (jobjectArray)env->GetObjectField(
            package_info_object, fieldId);
        if (signature_object_array == NULL)
        {
            return NULL;
        }
        jobject signature_object = env->GetObjectArrayElement(signature_object_array, 0);

        env->DeleteLocalRef(package_info_object);

        jclass signature_class = env->GetObjectClass(signature_object);
        methodId = env->GetMethodID(signature_class, "toCharsString", "()Ljava/lang/String;");
        env->DeleteLocalRef(signature_class);
        jstring signature_string = (jstring)env->CallObjectMethod(signature_object, methodId);
        return signature_string;
    }
    catch (...)
    {
        return env->NewStringUTF("signError");
    }
}

jstring getImei(JNIEnv *env, jobject mContext)
{
    if (mContext == nullptr)
    {
        return env->NewStringUTF("Context is NULL");
    }
    try
    {
        jclass cls_context = env->FindClass("android/content/Context");
        if (cls_context == nullptr)
        {
            return env->NewStringUTF("FindClass <android/content/Context> Error");
        }
        jmethodID getSystemService = env->GetMethodID(cls_context, "getSystemService",
                                                      "(Ljava/lang/String;)Ljava/lang/Object;");
        if (getSystemService == nullptr)
        {
            return env->NewStringUTF("getSystemService is NULL");
        }
        jfieldID TELEPHONY_SERVICE = env->GetStaticFieldID(cls_context, "TELEPHONY_SERVICE",
                                                           "Ljava/lang/String;");
        if (TELEPHONY_SERVICE == nullptr)
        {
            return env->NewStringUTF("TELEPHONY_SERVICE is NULL");
        }
        jobject jsTelService = env->GetStaticObjectField(cls_context, TELEPHONY_SERVICE);
        jobject telephonyManager = (env->CallObjectMethod(mContext, getSystemService,
                                                          jsTelService));
        if (checkError(env) < 0)
        {
            return env->NewStringUTF("Error1");
        }
        if (telephonyManager == nullptr)
        {
            return env->NewStringUTF("telephonyManager is NULL");
        }
        jclass cls_TelephoneManager = env->FindClass("android/telephony/TelephonyManager");
        if (checkError(env) < 0)
        {
            return env->NewStringUTF("Error2");
        }
        if (cls_TelephoneManager == nullptr)
        {
            return env->NewStringUTF("cls_TelephoneManager is NULL");
        }
        jmethodID getDeviceId = (env->GetMethodID(cls_TelephoneManager, "getDeviceId",
                                                  "()Ljava/lang/String;"));
        if (checkError(env) < 0)
        {
            return env->NewStringUTF("Error3");
        }
        jobject DeviceID = env->CallObjectMethod(telephonyManager, getDeviceId);
        if (checkError(env) < 0)
        {
            return env->NewStringUTF("Error4");
        }
        return (jstring)DeviceID;
    }
    catch (...)
    {
        return env->NewStringUTF("ErrorCatch");
    }
}

/**
 * 判断当前设备是否root
 * @return 是否root
 */
int isRooted()
{
    try
    {
        const int expectResultSize = 256;
        char result[expectResultSize] = {0};
        // 查看su文件存在位置
        executeCMD("which su", result, expectResultSize);
        DEBUG("execute `which su` result  %s", result);
        int resultSize = strlen(result);
        if (resultSize > 1)
        {
            return 1;
        }
        return -1;
    }
    catch (...)
    {
        return -2;
    }
}

neb::JSON getHookInfo(JNIEnv *env, jobject appContext, neb::JSON infoObj)
{
    try
    {
        jclass clsCheckHook = env->FindClass(JNI_CLASS_PATH);
        jmethodID midIsHook = env->GetStaticMethodID(clsCheckHook, "isHook", "()Z");
        jboolean isHook = env->CallStaticBooleanMethod(clsCheckHook, midIsHook, appContext);
        if (checkError(env) < 0)
        {
            return infoObj;
        }
        infoObj.Add("hook", isHook);
        return infoObj;
    }
    catch (...)
    {
        return infoObj;
    }
}

/**
 * 获取终端设备不可变的原始信息，可用来生成设备唯一标识。
 */
std::string getDeviceInfo(JNIEnv *env)
{
    neb::JSON infoObj;

    jobject appContextObj = getAppContext(env);
    jclass contextClass = env->GetObjectClass(appContextObj);
    jmethodID getAssets = env->GetMethodID(contextClass, "getAssets",
                                           "()Landroid/content/res/AssetManager;");
    AAssetManager *mgr = AAssetManager_fromJava(env,
                                                env->CallObjectMethod(appContextObj, getAssets));
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
    // 应用名称
    std::string strAppName = getAppName(env);
    infoObj.Add("appName", strAppName);
    // 应用签名
    jstring jsSignId = get_sign_id(env, appContextObj);
    //std::string strSignId = jstring2str(env, jsSignId);
    jstring jsMd5 = md5(env, jsSignId);
    std::string strMd5 = jstring2str(env, jsMd5);
    infoObj.Add("appSignature", strMd5);
    // 应用包名
    jstring jsPackageName = getPackageName(env, appContextObj);
    std::string strPackageName = jstring2str(env, jsPackageName);
    infoObj.Add("appBundleId", strPackageName);
    infoObj = getPackageVersionInfo(env, appContextObj, infoObj);
    // 国际移动设备识别码（需要READ_PHONE_STATE权限，可能获取不到）
    jstring jsImei = getImei(env, appContextObj);
    std::string strImei = jstring2str(env, jsImei);
    infoObj.Add("imei", strImei);
    // 获取安卓ID（极个别设备获取不到数据或得到错误数据）
    jstring androidId = get_android_id(env, appContextObj);
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
    infoObj = getHookInfo(env, appContextObj, infoObj);

    std::string strResult = infoObj.ToString();
    if (checkError(env) < 0)
    {
        strResult = "";
    }
    return strResult;
}

/**
 * 对字符串进行加密
 * @param c
 * @param a
 */
void encryption(std::string &c, int a[])
{
    for (int i = 0, j = 0; c[j]; j++, i = (i + 1) % 7)
    {
        c[j] ^= a[i];
    }
}

/**
 * 对字符串进行解密
 * @param c
 * @param a
 */
void decryption(std::string &c, int *a)
{
    for (int i = 0, j = 0; c[j]; j++, i = (i + 1) % 7)
    {
        c[j] ^= a[i];
    }
}

/*
 * 获取全局 Application
 */
jobject getApplicationContext(JNIEnv *env)
{
    jclass activityThread = env->FindClass("android/app/ActivityThread");
    jmethodID currentActivityThread = env->GetStaticMethodID(activityThread,
                                                             "currentActivityThread",
                                                             "()Landroid/app/ActivityThread;");
    jobject at = env->CallStaticObjectMethod(activityThread, currentActivityThread);
    jmethodID getApplication = env->GetMethodID(activityThread, "getApplication",
                                                "()Landroid/app/Application;");
    return env->CallObjectMethod(at, getApplication);
}

/*
 * 初始化并判断当前 APP 是否为合法应用，只需调用一次
 */
JNICALL jboolean verify(JNIEnv *env, jclass)
{

    jclass binderClass = env->FindClass("android/os/Binder");
    jclass contextClass = env->FindClass("android/content/Context");
    jclass signatureClass = env->FindClass("android/content/pm/Signature");
    jclass packageNameClass = env->FindClass("android/content/pm/PackageManager");
    jclass packageInfoClass = env->FindClass("android/content/pm/PackageInfo");

    jmethodID packageManager = env->GetMethodID(contextClass, "getPackageManager",
                                                "()Landroid/content/pm/PackageManager;");
    jmethodID packageName = env->GetMethodID(contextClass, "getPackageName",
                                             "()Ljava/lang/String;");
    jmethodID toCharsString = env->GetMethodID(signatureClass, "toCharsString",
                                               "()Ljava/lang/String;");
    jmethodID packageInfo = env->GetMethodID(packageNameClass, "getPackageInfo",
                                             "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
    jmethodID nameForUid = env->GetMethodID(packageNameClass, "getNameForUid",
                                            "(I)Ljava/lang/String;");
    jmethodID callingUid = env->GetStaticMethodID(binderClass, "getCallingUid", "()I");

    jint uid = env->CallStaticIntMethod(binderClass, callingUid);

    // 获取全局 Application
    jobject context = getApplicationContext(env);

    jobject packageManagerObject = env->CallObjectMethod(context, packageManager);
    jstring packNameString = (jstring)env->CallObjectMethod(context, packageName);
    jobject packageInfoObject = env->CallObjectMethod(packageManagerObject, packageInfo,
                                                      packNameString, 64);
    jfieldID signaturefieldID = env->GetFieldID(packageInfoClass, "signatures",
                                                "[Landroid/content/pm/Signature;");
    jobjectArray signatureArray = (jobjectArray)env->GetObjectField(packageInfoObject,
                                                                    signaturefieldID);
    jobject signatureObject = env->GetObjectArrayElement(signatureArray, 0);
    jstring runningPackageName = (jstring)env->CallObjectMethod(packageManagerObject, nameForUid,
                                                                uid);

    if (runningPackageName)
    { // 正在运行应用的包名
        const char *charPackageName = env->GetStringUTFChars(runningPackageName, JNI_FALSE);
        if (strcmp(charPackageName, APP_PACKAGE_NAME) != 0)
        {
            return JNI_FALSE;
        }
        env->ReleaseStringUTFChars(runningPackageName, charPackageName);
    }
    else
    {
        return JNI_FALSE;
    }

    jstring signatureStr = (jstring)env->CallObjectMethod(signatureObject, toCharsString);
    const char *signature = env->GetStringUTFChars(
        (jstring)env->CallObjectMethod(signatureObject, toCharsString), NULL);

    env->DeleteLocalRef(binderClass);
    env->DeleteLocalRef(contextClass);
    env->DeleteLocalRef(signatureClass);
    env->DeleteLocalRef(packageNameClass);
    env->DeleteLocalRef(packageInfoClass);

    DEBUG("current apk signature %s", signature);
    if (strcmp(signature, SIGNATURE_KEY) == 0)
    {
        DEBUG("verification passed");
        env->ReleaseStringUTFChars(signatureStr, signature);
        auth = JNI_TRUE;
        return JNI_TRUE;
    }
    else
    {
        DEBUG("verification failed");
        auth = JNI_FALSE;
        return JNI_FALSE;
    }
}

/*
 * 获取 Key
 */
JNIEXPORT jstring JNICALL getKey(JNIEnv *env, jclass)
{
    if (auth)
    {
        return env->NewStringUTF(DECRYPT_KEY);
    }
    else
    { // 你没有权限，验证没有通过。
        return env->NewStringUTF("You don't have permission, the verification didn't pass.");
    }
}

int encodeFileData(int b)
{
    return b + 20191104;
}

int decodeFileData(int b)
{
    return b - 20191104;
}

/**
 * 对传入的文件进行加密，保存
 */
extern "C" JNIEXPORT jint JNICALL
fileEncrypt(JNIEnv *env, jclass, jstring sourcePath, jstring filePath)
{

    //将jstring转化为普通的字符串
    const char *SPath = env->GetStringUTFChars(sourcePath, JNI_FALSE);
    const char *FPath = env->GetStringUTFChars(filePath, JNI_FALSE);

    //获取到两个文件，没有则创建
    FILE *SOURCE_FILE = fopen(SPath, "rb");
    FILE *ENCODE_FILE = fopen(FPath, "wb+");

    if (SOURCE_FILE == NULL)
    {
        DEBUG("文件不存在");
        return EOF;
    }

    int b;
    while ((b = fgetc(SOURCE_FILE)) != EOF)
    {
        fputc(encodeFileData(b), ENCODE_FILE);
    }

    fclose(SOURCE_FILE);
    fclose(ENCODE_FILE);

    env->ReleaseStringUTFChars(sourcePath, SPath);
    env->ReleaseStringUTFChars(filePath, FPath);

    DEBUG("文件加密完成");
    return 1;
}

/**
 * 文件解密
 * @param env
 * @param sourseFile
 * @param file
 * @return
 */
JNIEXPORT jint JNICALL
fileDecrypt(JNIEnv *env, jclass, jstring sourceFile, jstring file)
{

    const char *SFILE = env->GetStringUTFChars(sourceFile, JNI_FALSE);
    const char *PFILE = env->GetStringUTFChars(file, JNI_FALSE);

    FILE *SOURCE_FILE = fopen(SFILE, "rb");
    FILE *DECODE_FILE = fopen(PFILE, "wb+");

    if (SOURCE_FILE == NULL)
    {
        DEBUG("没有文件");
        return EOF;
    }

    int b;

    while ((b = fgetc(SOURCE_FILE)) != EOF)
    {
        fputc(decodeFileData(b), DECODE_FILE);
    }

    fclose(SOURCE_FILE);
    fclose(DECODE_FILE);

    env->ReleaseStringUTFChars(sourceFile, SFILE);
    env->ReleaseStringUTFChars(file, PFILE);
    DEBUG("解码成功");
    return 1;
}

/**
 * 将文件分为多个
 * @param env
 * @param filePath
 * @param fileName
 * @param count
 * @return
 */
JNIEXPORT jint JNICALL
fileDivision(JNIEnv *env, jclass, jstring filePath, jstring path, jint count)
{

    //获取到地址
    const char *FILE_PATH = env->GetStringUTFChars(filePath, JNI_FALSE);
    const char *FILE_NAME = env->GetStringUTFChars(path, JNI_FALSE);

    //打开原文件
    FILE *SOURCE_FILE = fopen(FILE_PATH, "rb");

    if (SOURCE_FILE == NULL)
    {
        DEBUG("没有文件");
        return EOF;
    }

    //分解文件的列表
    FILE *file_array[count];

    //创建分解文件
    for (int i = 0; i < count; i++)
    {
        char buf[1024];
        strcpy(buf, FILE_NAME);
        sprintf(buf, "%s%d", buf, i);
        const char *mp = (const char *)buf;
        FILE *D_FILE = fopen(mp, "wb+");
        file_array[i] = D_FILE;
    }

    //将数据依次写到各个文件
    int b;
    int i = 0;
    while ((b = fgetc(SOURCE_FILE)) != EOF)
    {
        fputc(encodeFileData(b), file_array[i]);
        i++;
        if (i >= count)
        {
            i = 0;
        }
    }

    //关闭各个文件
    for (int a = 0; a < count; a++)
    {
        fclose(file_array[a]);
    }

    //释放内存
    env->ReleaseStringUTFChars(filePath, FILE_PATH);
    env->ReleaseStringUTFChars(path, FILE_NAME);

    return 1;
}

/**
 * 文件合并
 * @param env
 * @param sourcePath
 * @param paths
 * @return
 */
JNIEXPORT jint JNICALL
fileMerge(JNIEnv *env, jclass, jstring sourcePath, jobjectArray paths)
{

    const char *SOURCE_PATH = env->GetStringUTFChars(sourcePath, JNI_FALSE);

    int fileCount = env->GetArrayLength(paths);

    FILE *source_file = fopen(SOURCE_PATH, "wb+");

    FILE *files[fileCount];

    for (int i = 0; i < fileCount; i++)
    {
        jstring jb = static_cast<jstring>(env->GetObjectArrayElement(paths, i));
        const char *file = env->GetStringUTFChars(jb, JNI_FALSE);

        FILE *f = fopen(file, "rb");

        if (f == NULL)
        {
            DEBUG("%s没有找到", file);
            return 0;
        }
        files[i] = f;

        env->ReleaseStringUTFChars(jb, file);
    }

    int i = 0;
    int byte;

    while ((byte = fgetc(files[i])) != EOF)
    {
        fputc(decodeFileData(byte), source_file);
        i++;
        if (i >= fileCount)
        {
            i = 0;
        }
    }

    for (int a = 0; a < fileCount; a++)
    {
        fclose(files[a]);
    }
    fclose(source_file);
    env->ReleaseStringUTFChars(sourcePath, SOURCE_PATH);
    return 1;
}

JNICALL jboolean saveDeviceStoredId(JNIEnv *env, jclass, jstring rootPath,
                                    jobject assetManager, jstring devId)
{

    std::string sId = jstring2str(env, devId);
    DEBUG("30000000 %s", sId.c_str());
    // 常量偏移值
    const int rand = 8;
    //首先是拼接目标位置路径
    std::string fromPath = env->GetStringUTFChars(rootPath, 0);
    std::string AssumeFilePath = fromPath + "/Runtime.so";
    std::string srcFileName = "demo_logo.png";
    std::string idStr = env->GetStringUTFChars(devId, 0);
    //检查拼接完成的文件位置是否可写入
    FILE *fp_to;
    if ((fp_to = fopen(AssumeFilePath.c_str(), "w")) == nullptr)
    {
        return static_cast<jboolean>(false);
    }
    // 该地址可以写入，则从asset目录中读取原始文件，混合id后写入新文件
    AAssetManager *mgr = AAssetManager_fromJava(env, assetManager);
    if (mgr == nullptr)
    {
        // asset目录获取异常
        return static_cast<jboolean>(false);
    }
    AAsset *asset = AAssetManager_open(mgr, srcFileName.c_str(), AASSET_MODE_UNKNOWN);
    if (asset == nullptr)
    {
        return static_cast<jboolean>(false);
    }
    //单次写入buffer
    const int perBuffer = 256;
    //已写入大小
    int writeSize = 0;
    //字符缓存数组
    char *buffer = (char *)malloc(perBuffer);
    int numBytesRead = -1;
    do
    {
        memset(buffer, 0, perBuffer);
        numBytesRead = AAsset_read(asset, buffer, perBuffer);
        fwrite(buffer, 1, numBytesRead, fp_to);
        writeSize += numBytesRead;
    } while (numBytesRead > 0);
    // 获取id字符串
    unsigned int iSize = idStr.size();
    unsigned int i = 0;
    for (i = 0; i < iSize; i++)
    {
        idStr[i] -= rand;
    }
    std::string blockStr = "gIFt" + idStr;
    fwrite(idStr.c_str(), 1, idStr.length(), fp_to);
    fwrite("\0", 1, 1, fp_to);
    // 扫尾
    fflush(fp_to);
    free(buffer);
    // 关闭文件
    fclose(fp_to);
    AAsset_close(asset);
    // 返回加密结果标记
    std::string success = std::to_string(writeSize) + "-" + std::to_string(idStr.length()) + "-" +
                          std::to_string(rand);
    DEBUG("success: %s", success.c_str());
    return static_cast<jboolean>(true);
}

JNICALL jstring getEncodeInfo(JNIEnv *env, jclass)
{
    std::string strResult = getDeviceInfo(env);
    DEBUG("success: %s", strResult.c_str());
    BASE64 cBase64Coder;
    std::string encodedResult = cBase64Coder.encode(strResult);
    int a[] = {4, 9, 6, 2, 8, 7, 3};
    encryption(encodedResult, a);
    std::string sFlag = "a8;";
    std::string dFlag = "d0az";
    strResult = sFlag + encodedResult + dFlag;
    const char *cStr = strResult.c_str();
    DEBUG("success encrypt: %s", cStr);
    return env->NewStringUTF(cStr);
}

/*
 * 动态注册 native 方法数组，可以不受方法名称的限制，与 Java native 方法一一对应
 *
 * 私密数据存储部分：https://github.com/RockyQu/JNIKeyProtection
 * 文件加密解密部分：https://github.com/earthWo/FileEncryption
 * 设备唯一标识部分：https://github.com/quert999/DeviceObservern
*/
int registerNatives(JNIEnv *env)
{
    JNINativeMethod registerMethods[] = {
        {"verify", "()Z", (jboolean *)verify},
        {"getKey", "()Ljava/lang/String;", (jstring *)getKey},
        {"fileEncrypt", "(Ljava/lang/String;Ljava/lang/String;)I", (void *)fileEncrypt},
        {"fileDecrypt", "(Ljava/lang/String;Ljava/lang/String;)I", (void *)fileDecrypt},
        {"fileDivision", "(Ljava/lang/String;Ljava/lang/String;I)I", (void *)fileDivision},
        {"fileMerge", "(Ljava/lang/String;[Ljava/lang/String;)I", (void *)fileMerge},
        {"getEncodeDeviceInfo", "()Ljava/lang/String;", (void *)getEncodeInfo},
        {"saveStoredId", "(Ljava/lang/String;Landroid/content/res/AssetManager;Ljava/lang/String;)Z", (void *)saveDeviceStoredId},
    };
    int methodsNum = sizeof(registerMethods) / sizeof(registerMethods[0]);
    return registerNativeMethods(env, JNI_CLASS_PATH, registerMethods, methodsNum);
}

/*
 * 默认执行的初始化方法
 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *)
{
    DEBUG("Dynamic library was load");
    JNIEnv *env = nullptr;
    if (vm->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK)
    {
        return JNI_ERR;
    }
    DEBUG("register native methods");
    assert(env != nullptr);
    if (!registerNatives(env))
    {
        DEBUG("register native methods failed");
        return JNI_ERR;
    }
    DEBUG("register native methods success");
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *, void *)
{
    DEBUG("Dynamic library was unload");
}
