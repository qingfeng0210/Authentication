#ifndef DEVICEINFO_LKJLKFDJOIEJFLKDJK_LJFLDJFLKDJFD________________
#define DEVICEINFO_LKJLKFDJOIEJFLKDJK_LJFLDJFLKDJFD________________

#include "jni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <android/log.h>
#include <pthread.h>


extern JNIEnv* st_env_Thread;
extern pthread_t NewthreadID;


# ifdef LOGRECORD

#define LOGI(message...) __android_log_print(ANDROID_LOG_INFO,"JNILog",message)
#define LOGW(TAG,message) __android_log_print(ANDROID_LOG_WARN,TAG,message)
#define LOGE(TAG,message) __android_log_print(ANDROID_LOG_ERROR,TAG,message)

#define DITAG "DeviceInfo:"

#else

#define LOGI(message...)
#define LOGW(TAG,message)
#define LOGE(TAG,message)

#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//锟斤拷取Mac锟斤拷址锟斤拷锟缴癸拷锟斤拷锟截碉拷址锟斤拷锟饺ｏ拷锟斤拷锟襟返伙拷-1
int GetWifiRssi();//锟斤拷取wifi锟脚猴拷强锟饺ｏ拷锟矫碉拷锟斤拷值锟斤拷一锟斤拷0锟斤拷-100锟斤拷锟斤拷锟街碉拷锟斤拷锟揭伙拷锟絠nt锟斤拷锟斤拷荩锟斤拷锟斤拷锟�锟斤拷-50锟斤拷示锟脚猴拷锟斤拷茫锟�50锟斤拷-70锟斤拷示锟脚猴拷偏锟筋，小锟斤拷-70锟斤拷示锟斤拷锟�没锟叫匡拷wifi锟斤拷锟斤拷锟斤拷-200
long AvailMemory();     //锟斤拷取锟斤拷锟斤拷锟节达拷
long ElapseRealTime();
void ReleaseRef();
//int GetImeiImsiInfo(char* Info);

//SystemClock.elapsedRealtime()
#ifdef __cplusplus
}
#endif // __cplusplus

#endif
