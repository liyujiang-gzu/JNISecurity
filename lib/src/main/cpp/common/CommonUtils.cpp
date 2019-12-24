///
/// Created by liyujiang on 2019/12/18.
/// Author 大定府羡民
///

#include <string>
#include <jni.h>
#include "log.h"
#include "config.h"
#include "CommonUtils.h"
#include "MD5.hpp"

int registerNativeMethods(JNIEnv *env, const char *className, JNINativeMethod *getMethods,
                          int methodsNum) {
    jclass clazz;
    //找到声明native方法的类
    clazz = env->FindClass(className);
    if (clazz == nullptr) {
        return JNI_FALSE;
    }
    //注册函数 参数：java类 所要注册的函数数组 注册函数的个数
    if (env->RegisterNatives(clazz, getMethods, methodsNum) < 0) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

int checkError(JNIEnv *env) {
    jthrowable err = env->ExceptionOccurred();
    if (err != nullptr) {
        DEBUG("手动清空异常信息，保证Java代码能够继续执行");
        env->ExceptionClear();
        return -1;
    }
    return 1;
}

int isEmptyStr(JNIEnv *env, jstring str) {
    jclass clsstring = env->FindClass(JNI_CLASS_PATH);
    if (clsstring == nullptr) {
        return -3;
    }
    jmethodID mid = env->GetStaticMethodID(clsstring, "isNotEmpty", "(Ljava/lang/String;)Z");
    if (mid == nullptr) {
        return -2;
    }
    jboolean result = env->CallStaticBooleanMethod(clsstring, mid, str);
    if (result) {
        return 1;
    } else {
        return -1;
    }
}

std::string jstring2str(JNIEnv *env, jstring jstr) {
    if (isEmptyStr(env, jstr) < 0) {
        return "";
    }
    char *rtn = nullptr;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("UTF-8");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jobject barr = env->CallObjectMethod(jstr, mid, strencode);
    jsize alen = env->GetArrayLength((jbyteArray) barr);
    jbyte *ba = env->GetByteArrayElements((jbyteArray) barr, JNI_FALSE);
    if (alen > 0) {
        rtn = (char *) malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements((jbyteArray) barr, ba, 0);
    std::string stemp(rtn);
    free(rtn);
    env->DeleteLocalRef(clsstring);
    env->DeleteLocalRef(strencode);
    return stemp;
}

jstring str2jstring(JNIEnv *env, const char *pat) {
    //定义java String类 strClass
    jclass strClass = (env)->FindClass("java/lang/String");
    //获取String(byte[],String)的构造器,用于将本地byte[]数组转换为一个新String
    jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    //建立byte数组
    jbyteArray bytes = (env)->NewByteArray(strlen(pat));
    //将char* 转换为byte数组
    (env)->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte *) pat);
    // 设置String, 保存语言类型,用于byte数组转换至String时的参数
    jstring encoding = (env)->NewStringUTF("UTF-8");
    //将byte数组转换为java String,并输出
    return (jstring) (env)->NewObject(strClass, ctorID, bytes, encoding);
}

jstring md5(JNIEnv *env, jstring str) {
    std::string cStr = jstring2str(env, str);
    const char *pat = toMD5(cStr).c_str();
    jstring md5ByC = str2jstring(env, pat);
    return md5ByC;
}

/**
 * 执行系统命令 并返回结果
 * @param cmd 命令
 * @param result  执行结果
 */
void executeCMD(const char *cmd, char *result, int expectResultSize) {

    const int bufSize = 32;
    char buf_ps[bufSize];
    // 命令行数组（不能大于128）
    char ps[128] = {0};

    FILE *ptr;
    strcpy(ps, cmd);
    if ((ptr = popen(ps, "r")) != nullptr) {
        while (fgets(buf_ps, bufSize, ptr) != nullptr) {

            int existLen = strlen(result);
            int bufLen = strlen(buf_ps);
            if (existLen + bufLen > expectResultSize) {
                break;
            }
            //可以通过这行来获取shell命令行中的每一行的输出
            strcat(result, buf_ps);
        }
        pclose(ptr);
    } else {
        DEBUG("popen %s error", ps);
    }
}

std::string getSerial(JNIEnv *env) {
    jclass jniClass = env->FindClass(JNI_CLASS_PATH);
    jmethodID jmiSerial = env->GetStaticMethodID(jniClass, "getSerial", "()Ljava/lang/String;");
    jobject serial = env->CallStaticObjectMethod(jniClass, jmiSerial);
    std::string strSerial = jstring2str(env, (jstring) serial);
    return strSerial;
}

jobject getAppContext(JNIEnv *env) {
    jclass activityThread = env->FindClass("android/app/ActivityThread");
    jmethodID currentActivityThread = env->GetStaticMethodID(activityThread,
                                                             "currentActivityThread",
                                                             "()Landroid/app/ActivityThread;");
    jobject at = env->CallStaticObjectMethod(activityThread, currentActivityThread);
    jmethodID getApplication = env->GetMethodID(activityThread, "getApplication",
                                                "()Landroid/app/Application;");
    return env->CallObjectMethod(at, getApplication);
}

std::string getAppPackageName(JNIEnv *env) {
    try {
        jobject context = getAppContext(env);
        jclass contextClass = env->FindClass("android/content/Context");
        jmethodID getPackageNameId = env->GetMethodID(contextClass, "getPackageName",
                                                      "()Ljava/lang/String;");
        jobject packNameString = env->CallObjectMethod(context, getPackageNameId);
        env->DeleteLocalRef(contextClass);
        return jstring2str(env, (jstring) packNameString);
    }
    catch (...) {
        return "";
    }
}

std::string getAppSignature(JNIEnv *env) {
    try {
        jobject context = getAppContext(env);
        jclass context_class = env->GetObjectClass(context);
        jmethodID methodId = env->GetMethodID(context_class, "getPackageManager",
                                              "()Landroid/content/pm/PackageManager;");
        jobject package_manager_object = env->CallObjectMethod(context, methodId);
        if (package_manager_object == nullptr) {
            DEBUG("getPackageManager() Failed!");
            return "";
        }
        methodId = env->GetMethodID(context_class, "getPackageName", "()Ljava/lang/String;");
        jobject package_name_string = env->CallObjectMethod(context, methodId);
        if (package_name_string == nullptr) {
            DEBUG("getPackageName() Failed!");
            return "";
        }
        env->DeleteLocalRef(context_class);
        jclass pack_manager_class = env->GetObjectClass(package_manager_object);
        methodId = env->GetMethodID(pack_manager_class, "getPackageInfo",
                                    "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
        env->DeleteLocalRef(pack_manager_class);
        jobject package_info_object = env->CallObjectMethod(package_manager_object, methodId,
                                                            package_name_string, GET_SIGNATURES);
        if (package_info_object == nullptr) {
            return "";
        }
        env->DeleteLocalRef(package_manager_object);
        jclass package_info_class = env->GetObjectClass(package_info_object);
        jfieldID fieldId = env->GetFieldID(package_info_class, "signatures",
                                           "[Landroid/content/pm/Signature;");
        env->DeleteLocalRef(package_info_class);
        jobject signature_object_array = env->GetObjectField(
            package_info_object, fieldId);
        if (signature_object_array == nullptr) {
            return "";
        }
        jobject signature_object = env->GetObjectArrayElement((jobjectArray) signature_object_array,
                                                              0);
        env->DeleteLocalRef(package_info_object);
        jclass signature_class = env->GetObjectClass(signature_object);
        methodId = env->GetMethodID(signature_class, "toCharsString", "()Ljava/lang/String;");
        env->DeleteLocalRef(signature_class);
        jobject signature_string = env->CallObjectMethod(signature_object, methodId);
        return jstring2str(env, (jstring) signature_string);
    }
    catch (...) {
        return "";
    }

}
