#include "DeviceInfo.h"
#include "DeviceInfo1.h"

JNIEnv* st_env_Thread=0;
pthread_t NewthreadID=0;
JNIEnv* st_Used=0;

static jclass contextClass=0;
static jobject WifiSvcObj=0;
static jobject ActivitySvcObj=0;
static jclass wifiMgClass=0;
static jobject wifiInfo=0;
static jclass wifiIfClass=0;
static jstring mac=0;
static jclass AMInfoClass=0;
static jstring wifi=0;
static jobject Meminfo=0;
static jclass SysClockCls = 0;
static jclass AMClass=0;
static jstring activityStr=0;

//static jstring telephonyStr=0;
//static jobject teleSvcObj=0;
//static jclass teleSvcClass=0;
//static jmethodID telegetSubscriberIdMeth=0;
//static jmethodID teleGetDeviceIdMeth=0;
//static jstring SubscriberIdStr=0;
//static jstring DeviceIdStr=0;

static jmethodID SystemServiceMethodID=0;
static jmethodID getConnMethod=0;
static jmethodID getMacMethod=0;
static jmethodID InitMethod=0;
static jmethodID getMeminfoMeth = 0;
static jfieldID availMemID=0;
static jmethodID elapseTimeMethID =0;
static jmethodID getRSSIMethod=0;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


int GetWifiRssi()
{
	LOGI("Thread CurrentThread=%d,GetWifiRssi",pthread_self());
	if(st_env_Thread!=0 && pthread_equal(pthread_self(),NewthreadID)) st_Used=st_env_Thread;
	else st_Used=st_env;
	LOGI("Thread CurrentThread=%x,GetWifiRssi",st_Used);

	if(st_thiz==0 || st_Used==0) return -1;
	if(contextClass==0)
	{
		jclass contextClass1=0;
		//if((contextClass1=st_Used->FindClass("android/app/ContextImpl"))==0) return -1;
		if((contextClass1=st_Used->GetObjectClass(st_thiz))==0) return -1;
		contextClass=(jclass)st_Used->NewGlobalRef(contextClass1);
		st_Used->DeleteLocalRef(contextClass1);
		if(contextClass==0) return -1;
	}
	if(SystemServiceMethodID==0)
	{
		if((SystemServiceMethodID=st_Used->GetMethodID(contextClass,"getSystemService","(Ljava/lang/String;)Ljava/lang/Object;"))==0) return -1;
	}
	if(wifi==0)
	{
		jstring wifi1=0;
		if((wifi1=st_Used->NewStringUTF("wifi"))==0) return -1;
		wifi=(jstring)st_Used->NewGlobalRef(wifi1);
		st_Used->DeleteLocalRef(wifi1);
		if(wifi==0) return -1;
	}
	if(WifiSvcObj == 0)
	{
		jobject WifiSvcObj1=0;
		if((WifiSvcObj1=st_Used->CallObjectMethod(st_thiz,SystemServiceMethodID,wifi))==0) return -1;
		WifiSvcObj =st_Used->NewGlobalRef(WifiSvcObj1);
		st_Used->DeleteLocalRef(WifiSvcObj1);
		if(WifiSvcObj==0) return -1;
	}
	if(wifiMgClass==0)
	{
		jclass wifiMgClass1=0;
		if((wifiMgClass1=st_Used->FindClass("android/net/wifi/WifiManager"))==0) return -1;
		wifiMgClass=(jclass)st_Used->NewGlobalRef(wifiMgClass1);
		st_Used->DeleteLocalRef(wifiMgClass1);
		if(wifiMgClass==0) return -1;
	}
	if(getConnMethod==0)
	{
		if((getConnMethod=st_Used->GetMethodID(wifiMgClass,"getConnectionInfo","()Landroid/net/wifi/WifiInfo;"))==0) return -1;
	}
	if(wifiInfo==0)
	{
		jobject wifiInfo1=0;
		if((wifiInfo1=st_Used->CallObjectMethod(WifiSvcObj,getConnMethod))==0) return -1;
		wifiInfo=st_Used->NewGlobalRef(wifiInfo1);
		st_Used->DeleteLocalRef(wifiInfo1);
		if(wifiInfo==0) return -1;
	}
	if(wifiIfClass==0)
	{
		jclass wifiIfClass1=0;
		if((wifiIfClass1=st_Used->GetObjectClass(wifiInfo))==0) return -1;
		wifiIfClass=(jclass)st_Used->NewGlobalRef(wifiIfClass1);
		st_Used->DeleteLocalRef(wifiIfClass1);
		if(wifiIfClass==0) return -1;
	}
	if(getRSSIMethod==0)
	{
		if((getRSSIMethod=st_Used->GetMethodID(wifiIfClass,"getRssi","()I"))==0) return -1;
	}
	jint Rssi=0;  //jint=signed int
	LOGI("Thread wifiInfo=%x,getRSSIMethod=%d",wifiInfo,getRSSIMethod);
	Rssi= (jint)st_Used->CallIntMethod(wifiInfo,getRSSIMethod);
	//LOGI("GetWifiRssi:Success %d",Rssi);
	return Rssi;
}


long AvailMemory()
{
	LOGI("Thread CurrentThread=%d,AvailMemory",pthread_self());
	if(st_env_Thread!=0 && pthread_equal(pthread_self(),NewthreadID)) st_Used=st_env_Thread;
	else st_Used=st_env;
	LOGI("Thread CurrentThread=%x£¬AvailMemory",st_Used);
	//LOGI("GetMem:Begin");
	if(st_thiz==0 || st_Used==0) return -1;
	if(contextClass==0)
	{
		jclass contextClass1=0;
		//if((contextClass1=st_Used->FindClass("android/app/ContextImpl"))==0) return -1;
		if((contextClass1=st_Used->GetObjectClass(st_thiz))==0) return -1;
		contextClass=(jclass)st_Used->NewGlobalRef(contextClass1);
		st_Used->DeleteLocalRef(contextClass1);
		if(contextClass==0) return -1;
	}
	if(SystemServiceMethodID==0)
	{
		if((SystemServiceMethodID=st_Used->GetMethodID(contextClass,"getSystemService","(Ljava/lang/String;)Ljava/lang/Object;"))==0) return -1;
	}
	if(activityStr==0)
	{
		jstring activityStr1=0;
		if((activityStr1=st_Used->NewStringUTF("activity"))==0) return -1;
		activityStr=(jstring)st_Used->NewGlobalRef(activityStr1);
		st_Used->DeleteLocalRef(activityStr1);
		if(activityStr==0) return -1;
	}
	if(ActivitySvcObj == 0)
	{
		jobject ActivitySvcObj1=0;
		if((ActivitySvcObj1=st_Used->CallObjectMethod(st_thiz,SystemServiceMethodID,activityStr))==0) return -1;
		ActivitySvcObj =(jclass)st_Used->NewGlobalRef(ActivitySvcObj1);
		st_Used->DeleteLocalRef(ActivitySvcObj1);
		if(ActivitySvcObj==0) return -1;
	}

	if(AMInfoClass==0)
	{
		jclass AMInfoClass1=0;
		if((AMInfoClass1=st_Used->FindClass("android/app/ActivityManager$MemoryInfo"))==0) return -1;
		AMInfoClass =(jclass)st_Used->NewGlobalRef(AMInfoClass1);
		st_Used->DeleteLocalRef(AMInfoClass1);
		if(AMInfoClass==0) return -1;
	}
	if(InitMethod==0)
	{
		if((InitMethod=st_Used->GetMethodID(AMInfoClass,"<init>","()V"))==0) return -1;
	}
	if(Meminfo==0)
	{
		jobject Meminfo1=0;
		if((Meminfo1=st_Used->NewObject(AMInfoClass,InitMethod))==0) return -1;
		Meminfo=st_Used->NewGlobalRef(Meminfo1);
		st_Used->DeleteLocalRef(Meminfo1);
		if(Meminfo==0) return -1;
	}
	if(AMClass==0)
	{
		jclass AMClass1=0;
		if((AMClass1=st_Used->FindClass("android/app/ActivityManager"))==0) return -1;
		AMClass=(jclass)st_Used->NewGlobalRef(AMClass1);
		st_Used->DeleteLocalRef(AMClass1);
		if(AMClass==0) return -1;
	}
	if(getMeminfoMeth == 0)
	{
		if((getMeminfoMeth=st_Used->GetMethodID(AMClass,"getMemoryInfo","(Landroid/app/ActivityManager$MemoryInfo;)V"))==0) return -1;
	}
	st_Used->CallVoidMethod(ActivitySvcObj,getMeminfoMeth,Meminfo);

	if(availMemID==0)
	{
		if((availMemID=st_Used->GetFieldID(AMInfoClass,"availMem","J"))==0) return -1;
	}
	jlong AvailMem =0;
	AvailMem=st_Used->GetLongField(Meminfo,availMemID);

	//LOGI("GetMem:Success %d",AvailMem);
	return AvailMem;
}


long ElapseRealTime()
{
	LOGI("Thread CurrentThread=%d,ElapseRealTime",pthread_self());
	if(st_env_Thread!=0 && pthread_equal(pthread_self(),NewthreadID)) st_Used=st_env_Thread;
	else st_Used=st_env;
	LOGI("Thread CurrentThread=%x,ElapseRealTime",st_Used);
	//LOGI("GetRealTime:Begin");
	if(st_thiz==0 || st_Used==0) return -1;
	if(SysClockCls ==0)
	{
		jclass SysClockCls1=0;
		if((SysClockCls1=st_Used->FindClass("android/os/SystemClock"))==0) return -1;
		SysClockCls=(jclass)st_Used->NewGlobalRef(SysClockCls1);
		st_Used->DeleteLocalRef(SysClockCls1);
		if(SysClockCls==0) return -1;
	}
	if(elapseTimeMethID ==0)
	{
		if((elapseTimeMethID=st_Used->GetStaticMethodID(SysClockCls,"elapsedRealtime","()J"))==0) return -1;
	}
	jlong time =0;
	time = st_Used->CallStaticLongMethod(SysClockCls,elapseTimeMethID);
	//LOGI("GetRealTime:Success %d",time);
	return time;
}




void ReleaseRef()
{
	//LOGI("ReleaseRef:Begin");
	if(st_thiz==0 || st_env==0) return;
	if(contextClass!=0)
	{
		st_env->DeleteGlobalRef(contextClass);
		contextClass=0;
	}
	if(wifi!=0)
	{
		st_env->DeleteGlobalRef(wifi);
		wifi=0;
	}
	if(WifiSvcObj!=0)
	{
		st_env->DeleteGlobalRef(WifiSvcObj);
		WifiSvcObj=0;
	}
	if(ActivitySvcObj!=0)
	{
		st_env->DeleteGlobalRef(ActivitySvcObj);
		ActivitySvcObj=0;
	}
	if(wifiInfo!=0)
	{
		st_env->DeleteGlobalRef(wifiInfo);
		wifiInfo=0;
	}
	if(wifiIfClass!=0)
	{
		st_env->DeleteGlobalRef(wifiIfClass);
		wifiIfClass=0;
	}
	if(wifiMgClass!=0)
	{
		st_env->DeleteGlobalRef(wifiMgClass);
		wifiMgClass=0;
	}
	if(mac!=0)
	{
		st_env->DeleteGlobalRef(mac);
		mac=0;
	}
	if(AMInfoClass!=0)
	{
		st_env->DeleteGlobalRef(AMInfoClass);
		AMInfoClass=0;
	}
	if(Meminfo!=0)
	{
		st_env->DeleteGlobalRef(Meminfo);
		Meminfo=0;
	}
	if(AMClass!=0)
	{
		st_env->DeleteGlobalRef(AMClass);
		AMClass=0;
	}
	if(SysClockCls != 0)
	{
		st_env->DeleteGlobalRef(SysClockCls);
		SysClockCls=0;
	}
	if(activityStr!=0)
	{
		st_env->DeleteGlobalRef(activityStr);
		activityStr=0;
	}
	if(SystemServiceMethodID!=0)  SystemServiceMethodID=0;
	if(getConnMethod!=0) getConnMethod=0;
	if(getMacMethod!=0) getMacMethod=0;
	if(InitMethod!=0) InitMethod=0;
	if(getMeminfoMeth != 0) getMeminfoMeth = 0;
	if(availMemID!=0) availMemID=0;
	if(elapseTimeMethID !=0) elapseTimeMethID =0;
	if(getRSSIMethod!=0) getRSSIMethod=0;
	//LOGI("ReleaseRef:End");
}

#ifdef __cplusplus
}
#endif // __cplusplus
