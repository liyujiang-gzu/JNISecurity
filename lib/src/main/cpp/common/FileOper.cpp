//
// Created by liyujiang on 2019/12/19.
// Author 大定府羡民
//

#include <cstdio>
#include <cstring>
#include <jni.h>
#include "FileOper.h"
#include "log.h"
#include "config.h"
#include "CommonUtils.h"

static const int KEY = 20191104;
static const int BUFFER = 1024;

int encodeFileData(int b) {
    return b + KEY;
}

int decodeFileData(int b) {
    return b - KEY;
}

jint fileEncrypt(JNIEnv *env, jstring sourcePath, jstring filePath) {

    const char *SPath = env->GetStringUTFChars(sourcePath, JNI_FALSE);
    const char *FPath = env->GetStringUTFChars(filePath, JNI_FALSE);

    //获取到两个文件，没有则创建
    FILE *SOURCE_FILE = fopen(SPath, "rbe");
    FILE *ENCODE_FILE = fopen(FPath, "wb+e");

    if (SOURCE_FILE == nullptr) {
        DEBUG("文件不存在");
        return EOF;
    }

    int b;
    while ((b = fgetc(SOURCE_FILE)) != EOF) {
        fputc(encodeFileData(b), ENCODE_FILE);
    }

    fclose(SOURCE_FILE);
    fclose(ENCODE_FILE);

    env->ReleaseStringUTFChars(sourcePath, SPath);
    env->ReleaseStringUTFChars(filePath, FPath);

    DEBUG("文件加密完成");
    return 1;
}

jint fileDecrypt(JNIEnv *env, jstring sourceFile, jstring file) {

    const char *SFILE = env->GetStringUTFChars(sourceFile, JNI_FALSE);
    const char *PFILE = env->GetStringUTFChars(file, JNI_FALSE);

    FILE *SOURCE_FILE = fopen(SFILE, "rbe");
    FILE *DECODE_FILE = fopen(PFILE, "wb+e");

    if (SOURCE_FILE == nullptr) {
        DEBUG("没有文件");
        return EOF;
    }

    int b;

    while ((b = fgetc(SOURCE_FILE)) != EOF) {
        fputc(decodeFileData(b), DECODE_FILE);
    }

    fclose(SOURCE_FILE);
    fclose(DECODE_FILE);

    env->ReleaseStringUTFChars(sourceFile, SFILE);
    env->ReleaseStringUTFChars(file, PFILE);
    DEBUG("解码成功");
    return 1;
}

jint fileDivision(JNIEnv *env, jstring filePath, jstring path, jint count) {

    //获取到地址
    const char *FILE_PATH = env->GetStringUTFChars(filePath, JNI_FALSE);
    const char *FILE_NAME = env->GetStringUTFChars(path, JNI_FALSE);

    //打开原文件
    FILE *SOURCE_FILE = fopen(FILE_PATH, "rbe");

    if (SOURCE_FILE == nullptr) {
        DEBUG("没有文件");
        return EOF;
    }

    //分解文件的列表
    FILE *file_array[count];

    //创建分解文件
    for (int i = 0; i < count; i++) {
        char buf[BUFFER];
        strcpy(buf, FILE_NAME);
        sprintf(buf, "%s%d", buf, i);
        const char *mp = (const char *) buf;
        FILE *D_FILE = fopen(mp, "wb+e");
        file_array[i] = D_FILE;
    }

    //将数据依次写到各个文件
    int b;
    int i = 0;
    while ((b = fgetc(SOURCE_FILE)) != EOF) {
        fputc(encodeFileData(b), file_array[i]);
        i++;
        if (i >= count) {
            i = 0;
        }
    }

    //关闭各个文件
    for (int a = 0; a < count; a++) {
        fclose(file_array[a]);
    }

    //释放内存
    env->ReleaseStringUTFChars(filePath, FILE_PATH);
    env->ReleaseStringUTFChars(path, FILE_NAME);

    return 1;
}

jint fileMerge(JNIEnv *env, jstring sourcePath, jobjectArray paths) {

    const char *SOURCE_PATH = env->GetStringUTFChars(sourcePath, JNI_FALSE);

    int fileCount = env->GetArrayLength(paths);

    FILE *source_file = fopen(SOURCE_PATH, "wb+e");

    FILE *files[fileCount];

    for (int i = 0; i < fileCount; i++) {
        jobject jb = env->GetObjectArrayElement(paths, i);
        const char *file = env->GetStringUTFChars((jstring) jb, JNI_FALSE);

        FILE *f = fopen(file, "rbe");

        if (f == nullptr) {
            DEBUG("%s没有找到", file);
            return 0;
        }
        files[i] = f;

        env->ReleaseStringUTFChars((jstring) jb, file);
    }

    int i = 0;
    int byte;

    while ((byte = fgetc(files[i])) != EOF) {
        fputc(decodeFileData(byte), source_file);
        i++;
        if (i >= fileCount) {
            i = 0;
        }
    }

    for (int a = 0; a < fileCount; a++) {
        fclose(files[a]);
    }
    fclose(source_file);
    env->ReleaseStringUTFChars(sourcePath, SOURCE_PATH);
    return 1;
}
