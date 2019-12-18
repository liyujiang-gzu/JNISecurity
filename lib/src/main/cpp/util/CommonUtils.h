//
// Created by liyujiang on 2019/12/18.
//

#ifndef JNISECURITY_COMMONUTILS_H
#define JNISECURITY_COMMONUTILS_H

static const char *const JNI_CLASS_PATH = "com/github/gzuliyujiang/jni/JNISecurity";

#include<string>
#include <jni.h>

int registerNativeMethods(JNIEnv *env, const char *className, JNINativeMethod *getMethods,
                          int methodsNum);

int checkError(JNIEnv *env);

int isEmptyStr(JNIEnv *env, jstring str);

jstring md5(JNIEnv *env, jstring str);

std::string jstring2str(JNIEnv *env, jstring jstr);

jstring str2jstring(JNIEnv *env, const char *pat);

void executeCMD(const char *cmd, char *result, int expectResultSize);

std::string getSerial(JNIEnv *env);

jobject getAppContext(JNIEnv *env);

std::string getAppName(JNIEnv *env);

#endif //JNISECURITY_COMMONUTILS_H
