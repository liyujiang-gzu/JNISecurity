///
/// 不要直接在头文件中定义const或static变量，
/// 更好的写法是在源文件中定义，在头文件中extern声明。
/// 参阅 https://blog.csdn.net/u011726005/article/details/81713471
///
/// Created by liyujiang on 2019/12/18.
/// Author 大定府羡民
///

#ifndef JNISECURITY_CONFIG_H
#define JNISECURITY_CONFIG_H

extern const char *JNI_CLASS_PATH;
extern const char *APP_PACKAGE_NAME;
extern const char *SIGNATURE_KEY;
extern const char *DECRYPT_KEY;
extern const char *MD5_KEY;

#endif //JNISECURITY_CONFIG_H
