///
/// Created by liyujiang on 2019/12/18.
/// Author 大定府羡民
///

#ifndef EMULATOR_DETECTOR_H
#define EMULATOR_DETECTOR_H

#include <jni.h>

std::string detectEmulator(JNIEnv *env, std::string result);

bool is_special_emulator();

bool check_contain_BaseBand(JNIEnv *);

bool check_contain_cpu_Temperature();

bool detector_file_exists(const char *);

#endif //EMULATOR_DETECTOR_H
