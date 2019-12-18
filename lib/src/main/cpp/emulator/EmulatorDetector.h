///
/// Created by liyujiang on 2019/12/18.
/// Author 大定府羡民
///

#ifndef EMULATOR_DETECTOR_H
#define EMULATOR_DETECTOR_H

#include <jni.h>

std::string detectEmulator(JNIEnv *env, std::string result);

bool isSpecialEmulator();

bool checkContainBaseBand(JNIEnv *env);

bool checkContainCpuTemperature();

bool checkFileExists(const char *);

#endif //EMULATOR_DETECTOR_H
