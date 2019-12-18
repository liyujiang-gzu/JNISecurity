//
// Created by liyujiang on 2019/12/19.
// Author 大定府羡民
//

#ifndef JNISECURITY_FILEOPER_H
#define JNISECURITY_FILEOPER_H

#include <jni.h>

jint fileEncrypt(JNIEnv *env, jstring sourcePath, jstring filePath);

jint fileDecrypt(JNIEnv *env, jstring sourceFile, jstring file);

jint fileDivision(JNIEnv *env, jstring filePath, jstring path, jint count);

jint fileMerge(JNIEnv *env, jstring sourcePath, jobjectArray paths);

#endif //JNISECURITY_FILEOPER_H
