//
// Created by liyujiang on 2019/12/18.
//

#include <string>
#include <jni.h>
#include "CommonUtils.h"
#include "log.h"

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
    jmethodID mid = env->GetStaticMethodID(clsstring, "isNonEmpty", "(Ljava/lang/String;)Z");
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

jstring md5(JNIEnv *env, jstring str) {
    jclass clsstring = env->FindClass(JNI_CLASS_PATH);
    if (clsstring == nullptr) {
        return (env)->NewStringUTF("error");
    }
    jmethodID mid = env->GetStaticMethodID(clsstring, "md5",
                                           "(Ljava/lang/String;)Ljava/lang/String;");
    if (mid == nullptr) {
        return (env)->NewStringUTF("error");
    }
    return (jstring) env->CallStaticObjectMethod(clsstring, mid, str);
}

std::string jstring2str(JNIEnv *env, jstring jstr) {
    if (isEmptyStr(env, jstr) < 0) {
        return "";
    }
    char *rtn = nullptr;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("GB2312");
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
        ptr = nullptr;
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
    jclass jniClass = env->FindClass(JNI_CLASS_PATH);
    jmethodID staticMethodId = env->GetStaticMethodID(jniClass, "getAppContext",
                                                      "()Landroid/app/Application;");
    return env->CallStaticObjectMethod(jniClass, staticMethodId);
}

std::string getAppName(JNIEnv *env) {
    jclass jniClass = env->FindClass(JNI_CLASS_PATH);
    jobject appContext = getAppContext(env);
    jmethodID jmiAppName = env->GetStaticMethodID(jniClass, "getAppName",
                                                  "(Landroid/content/Context;)Ljava/lang/String;");
    jobject appName = env->CallStaticObjectMethod(jniClass, jmiAppName, appContext);
    std::string strAppName = jstring2str(env, (jstring) appName);
    return strAppName;
}
