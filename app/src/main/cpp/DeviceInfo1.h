#ifndef DEVICEINFO1_LKJLKFDJOIEJFLKDJK_LJFLDJFLKDJFD________________
#define DEVICEINFO1_LKJLKFDJOIEJFLKDJK_LJFLDJFLKDJFD________________

#include "jni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <android/log.h>

#include "DeviceInfo.h"

extern jobject st_thiz;
extern JNIEnv *st_env;
extern JavaVM *st_jvm;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//��ȡMac��ַ���ɹ����ص�ַ���ȣ����󷵻�-1

int GetImeiImsiInfo(char* Info);
int GetProcessPathByPIDFromJava(int pid,char* FilePath);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
