#include <jni.h>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include "log.h"

// 需要被验证应用的包名
const char *APP_PACKAGE_NAME = "com.github.gzuliyujiang.jni.demo";
// 应用签名，注意开发版和发布版的区别，发布版需要使用正式签名打包后获取
const char *SIGNATURE_KEY = "308202e2308201ca020101300d06092a864886f70d010105050030373116301406035504030c0d416e64726f69642044656275673110300e060355040a0c07416e64726f6964310b3009060355040613025553301e170d3139303932353032333330325a170d3439303931373032333330325a30373116301406035504030c0d416e64726f69642044656275673110300e060355040a0c07416e64726f6964310b300906035504061302555330820122300d06092a864886f70d01010105000382010f003082010a0282010100a2f642a5f8363b805c5b71718c7d3aa67a7c4f5445094198a4c3c84c0460773b6451c2103a164a344c77fced2c96c0c086c975d46629818df667d585dc80fbf97d32d83ef12f2436e64b6ec1535dc08cd0d508f0a91ad0f45e012fbca56aff0ab7c03821295678fd9833de7fd300ea6a28db4754c89abd30aaa3e2822c6ec38f887dfb90672b087d71ce3ec5f97268187c78cfe8ceebc21a6c1fd99ffa96e4ea153e95edd6cac6bd0d82379c3ca20a2b808e03e93289406cc7d5caf5d5fb1d13c690b809fee52a20783e6e25ed922cdb620df1be9a37da27cce96ed5ec160bbba2384a9e881cbe857021b56c568cdd14021db6459ec5bcca20e8461f343069ab0203010001300d06092a864886f70d010105050003820101007339d75da7db27e85ba6939485bcfd96f3db654e57de3064b264ef502b60434df8b4669ef1a2ab0d96e37a9a228090c63a81b25a9f007e8ddc59b308ca66e38a5a876dafb71b4699bb9330126f99575f15ec9f17c481bde9dd7250f8f53095c02d782cbc18509c5fa6742025af2fb43b021bc90f0875bb72179dc68880ebdc2e6cc8adb2b193cb17c1fd0587689e9a9cf5c49f79c2998adbb18eaf59f06bfd34e2503ceaa9bddeb77a9e9d41e78fa6f09bedf7df2e1274fa5aa8b73ba3daea626512ee924df3a369c8d2db8da932feb9be82bd2e84e292a7ed5f0f3b15bd4b9e82b8f5103363b6570aad4808192dd9035dde3dff072063ec1a3f7869d3fddabe";
// 需要被保护的密钥，请修改成你自己的密钥
const char *DECRYPT_KEY = "successful return key!";

// native 方法所在类的路径
const char *NATIVE_CLASS_PATH = "com/github/gzuliyujiang/jni/JNISecurity";

// 验证是否通过
static jboolean auth = JNI_FALSE;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 获取全局 Application
 */
jobject getApplicationContext(JNIEnv *env) {
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
JNICALL jboolean init(JNIEnv *env, jclass) {

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
    jstring packNameString = (jstring) env->CallObjectMethod(context, packageName);
    jobject packageInfoObject = env->CallObjectMethod(packageManagerObject, packageInfo,
                                                      packNameString, 64);
    jfieldID signaturefieldID = env->GetFieldID(packageInfoClass, "signatures",
                                                "[Landroid/content/pm/Signature;");
    jobjectArray signatureArray = (jobjectArray) env->GetObjectField(packageInfoObject,
                                                                     signaturefieldID);
    jobject signatureObject = env->GetObjectArrayElement(signatureArray, 0);
    jstring runningPackageName = (jstring) env->CallObjectMethod(packageManagerObject, nameForUid,
                                                                 uid);

    if (runningPackageName) {// 正在运行应用的包名
        const char *charPackageName = env->GetStringUTFChars(runningPackageName, JNI_FALSE);
        if (strcmp(charPackageName, APP_PACKAGE_NAME) != 0) {
            return JNI_FALSE;
        }
        env->ReleaseStringUTFChars(runningPackageName, charPackageName);
    } else {
        return JNI_FALSE;
    }

    jstring signatureStr = (jstring) env->CallObjectMethod(signatureObject, toCharsString);
    const char *signature = env->GetStringUTFChars(
            (jstring) env->CallObjectMethod(signatureObject, toCharsString), NULL);

    env->DeleteLocalRef(binderClass);
    env->DeleteLocalRef(contextClass);
    env->DeleteLocalRef(signatureClass);
    env->DeleteLocalRef(packageNameClass);
    env->DeleteLocalRef(packageInfoClass);

    LOGW("current apk signature %s", signature);
    LOGW("reserved signature %s", SIGNATURE_KEY);
    if (strcmp(signature, SIGNATURE_KEY) == 0) {
        LOGW("verification passed");
        env->ReleaseStringUTFChars(signatureStr, signature);
        auth = JNI_TRUE;
        return JNI_TRUE;
    } else {
        LOGW("verification failed");
        auth = JNI_FALSE;
        return JNI_FALSE;
    }
}

/*
 * 获取 Key
 */
JNIEXPORT jstring JNICALL getKey(JNIEnv *env, jclass) {
    if (auth) {
        return env->NewStringUTF(DECRYPT_KEY);
    } else {// 你没有权限，验证没有通过。
        return env->NewStringUTF("You don't have permission, the verification didn't pass.");
    }
}


int encodeFileData(int b) {
    return b + 20191104;
}

int decodeFileData(int b) {
    return b - 20191104;
}

/**
 * 对传入的文件进行加密，保存
 */
extern "C"
JNIEXPORT jint JNICALL
fileEncrypt(JNIEnv *env, jclass, jstring sourcePath, jstring filePath) {

    //将jstring转化为普通的字符串
    const char *SPath = env->GetStringUTFChars(sourcePath, JNI_FALSE);
    const char *FPath = env->GetStringUTFChars(filePath, JNI_FALSE);

    //获取到两个文件，没有则创建
    FILE *SOURCE_FILE = fopen(SPath, "rb");
    FILE *ENCODE_FILE = fopen(FPath, "wb+");

    if (SOURCE_FILE == NULL) {
        LOGW("文件不存在");
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

    LOGW("文件加密完成");
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
fileDecrypt(JNIEnv *env, jclass, jstring sourceFile, jstring file) {

    const char *SFILE = env->GetStringUTFChars(sourceFile, JNI_FALSE);
    const char *PFILE = env->GetStringUTFChars(file, JNI_FALSE);

    FILE *SOURCE_FILE = fopen(SFILE, "rb");
    FILE *DECODE_FILE = fopen(PFILE, "wb+");

    if (SOURCE_FILE == NULL) {
        LOGW("没有文件");
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
    LOGW("解码成功");
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
fileDivision(JNIEnv *env, jclass, jstring filePath, jstring path, jint count) {


    //获取到地址
    const char *FILE_PATH = env->GetStringUTFChars(filePath, JNI_FALSE);
    const char *FILE_NAME = env->GetStringUTFChars(path, JNI_FALSE);


    //打开原文件
    FILE *SOURCE_FILE = fopen(FILE_PATH, "rb");

    if (SOURCE_FILE == NULL) {
        LOGW("没有文件");
        return EOF;
    }


    //分解文件的列表
    FILE *file_array[count];

    //创建分解文件
    for (int i = 0; i < count; i++) {
        char buf[1024];
        strcpy(buf, FILE_NAME);
        sprintf(buf, "%s%d", buf, i);
        const char *mp = (const char *) buf;
        FILE *D_FILE = fopen(mp, "wb+");
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


/**
 * 文件合并
 * @param env
 * @param sourcePath
 * @param paths
 * @return
 */
JNIEXPORT jint JNICALL
fileMerge(JNIEnv *env, jclass, jstring sourcePath, jobjectArray paths) {

    const char *SOURCE_PATH = env->GetStringUTFChars(sourcePath, JNI_FALSE);

    int fileCount = env->GetArrayLength(paths);

    FILE *source_file = fopen(SOURCE_PATH, "wb+");

    FILE *files[fileCount];

    for (int i = 0; i < fileCount; i++) {
        jstring jb = static_cast<jstring>(env->GetObjectArrayElement(paths, i));
        const char *file = env->GetStringUTFChars(jb, JNI_FALSE);

        FILE *f = fopen(file, "rb");

        if (f == NULL) {
            LOGW("%s没有找到", file);
            return 0;
        }
        files[i] = f;

        env->ReleaseStringUTFChars(jb, file);
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

/*
 * 动态注册 native 方法数组，可以不受方法名称的限制，与 Java native 方法一一对应
 *
 * 私密数据存储部分：https://github.com/RockyQu/JNIKeyProtection
 * 文件加密解密部分：https://github.com/earthWo/FileEncryption
 */
static JNINativeMethod registerMethods[] = {
        {"init",         "()Z",                                      (jboolean *) init},
        {"getKey",       "()Ljava/lang/String;",                     (jstring *) getKey},
        {"fileEncrypt",  "(Ljava/lang/String;Ljava/lang/String;)I",  (void *) fileEncrypt},
        {"fileDecrypt",  "(Ljava/lang/String;Ljava/lang/String;)I",  (void *) fileDecrypt},
        {"fileDivision", "(Ljava/lang/String;Ljava/lang/String;I)I", (void *) fileDivision},
        {"fileMerge",    "(Ljava/lang/String;[Ljava/lang/String;)I", (void *) fileMerge},
};

/*
 * 动态注册 native 方法
 */
static int registerNativeMethods(JNIEnv *env, const char *className, JNINativeMethod *gMethods,
                                 int numMethods) {
    jclass clazz = env->FindClass(className);
    if (clazz == NULL) {
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

/*
 * 默认执行的初始化方法
 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOGW("JNI_OnLoad");

    JNIEnv *env = NULL;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    LOGW("register native methods");
    if (!registerNativeMethods(env, NATIVE_CLASS_PATH, registerMethods,
                               sizeof(registerMethods) / sizeof(registerMethods[0]))) {
        LOGW("register native methods failed");
        return JNI_ERR;
    }

    LOGW("register native methods success");
    return JNI_VERSION_1_6;
}

#ifdef __cplusplus
}
#endif