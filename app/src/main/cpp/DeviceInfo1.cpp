#include "DeviceInfo1.h"
#include "ALog.h"

jobject st_thiz=0;
JNIEnv *st_env=0;
JavaVM *st_jvm=0;

//static jclass contextClass=0;
//static jstring telephonyStr=0;
//static jobject teleSvcObj=0;
//static jclass teleSvcClass=0;
//static jmethodID telegetSubscriberIdMeth=0;
//static jmethodID teleGetDeviceIdMeth=0;
//static jstring SubscriberIdStr=0;
//static jstring DeviceIdStr=0;
//static jmethodID SystemServiceMethodID=0;


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


int GetImeiImsiInfo(char* Info)
{
    return -1;
	jclass contextClass1=0;
	jstring telephonyStr=0;
	jobject teleSvcObj=0;
	jmethodID SystemServiceMethodID1=0;
	jclass teleSvcClass=0;
	jmethodID telegetSubscriberIdMeth=0;
	jmethodID teleGetDeviceIdMeth=0;
	jstring SubscriberIdStr=0;
	jstring DeviceIdStr=0;

	int len =-1;
	const char* Temp;

	if(st_thiz==0 || st_env==0)
	{
		goto result;
	}

	LOGD("GetImeiImsiInfo GetObjectClass Begin!");
	if((contextClass1=st_env->FindClass("android/content/Context"))==0)
	{
		goto result;
	}
	LOGD("GetImeiImsiInfo GetObjectClass End!");

	LOGD("GetImeiImsiInfo getSystemServiceMethod Begin!");
	if((SystemServiceMethodID1=st_env->GetMethodID(contextClass1,"getSystemService","(Ljava/lang/String;)Ljava/lang/Object;"))==0)
	{
		goto result;
	}
	LOGD("GetImeiImsiInfo getSystemServiceMethod End!");

	LOGD("GetImeiImsiInfo GetPhoneStr Begin!");
	if(telephonyStr==0)
	{
		if((telephonyStr=st_env->NewStringUTF("phone"))==0)
		{
			goto result;
		}
	}
	LOGD("GetImeiImsiInfo GetPhoneStr End!");

	LOGD("GetImeiImsiInfo CallObjectMethod teleSvcObj Begin!");
	if((teleSvcObj=st_env->CallObjectMethod(st_thiz,SystemServiceMethodID1,telephonyStr))==0)
	{
		goto result;
	}
	LOGD("GetImeiImsiInfo CallObjectMethod teleSvcObj End!");

	LOGD("GetImeiImsiInfo TelephonyManager FindClass Begin!");
	if((teleSvcClass=st_env->FindClass("android/telephony/TelephonyManager"))==0) {
		goto result;
	}
	LOGD("GetImeiImsiInfo TelephonyManager FindClass End!");


//	LOGD("GetImeiImsiInfo getSubscriberId GetMethodID Begin!");
//	if((telegetSubscriberIdMeth=st_env->GetMethodID(teleSvcClass,"getSubscriberId","()Ljava/lang/String;"))==0)
//	{
//		goto result;
//	}
//	LOGD("GetImeiImsiInfo getSubscriberId GetMethodID End!");

	LOGD("GetImeiImsiInfo getDeviceId GetMethodID Begin!");
	if((teleGetDeviceIdMeth=st_env->GetMethodID(teleSvcClass,"getDeviceId","()Ljava/lang/String;"))==0){
		goto result;
	}
	LOGD("GetImeiImsiInfo getDeviceId GetMethodID End!");


//	LOGD("GetImeiImsiInfo getSubscriberId Call Begin!");
//	if((SubscriberIdStr=(jstring)st_env->CallObjectMethod(teleSvcObj,telegetSubscriberIdMeth))==0)
//	{
//		goto result;
//	}
//	LOGD("GetImeiImsiInfo getSubscriberId Call End!");

	LOGD("GetImeiImsiInfo getDeviceId Call Begin!");
	if((DeviceIdStr=(jstring)st_env->CallObjectMethod(teleSvcObj,teleGetDeviceIdMeth))==0)
	{
		goto result;
	}
	LOGD("GetImeiImsiInfo getDeviceId Call End!");

	Temp=st_env->GetStringUTFChars(DeviceIdStr,NULL);

	if(Temp==NULL)
	{
		goto result;;
	}
	else
	{
		strcpy(Info,Temp);
		len = len + strlen(Temp);
		st_env->ReleaseStringUTFChars(DeviceIdStr,Temp);
	}

//	Temp=st_env->GetStringUTFChars(SubscriberIdStr,NULL);
//	if(Temp==NULL){
//		goto result;
//	}
//	else
//	{
//		strcpy(Info+len,Temp);
//		len = len + strlen(Temp);
//		st_env->ReleaseStringUTFChars(SubscriberIdStr,Temp);
//	}

	LOGD("IMEIIMSI=%s,Len=%d",Info,len);

result:
	if( contextClass1) st_env->DeleteLocalRef(contextClass1);
	if( telephonyStr) st_env->DeleteLocalRef(telephonyStr);
	if( teleSvcObj) st_env->DeleteLocalRef(teleSvcObj);
	if( SystemServiceMethodID1) SystemServiceMethodID1=0;
	if( teleSvcClass) st_env->DeleteLocalRef(teleSvcClass);
	if( telegetSubscriberIdMeth) telegetSubscriberIdMeth=0;
	if( teleGetDeviceIdMeth) teleGetDeviceIdMeth=0;
	if( SubscriberIdStr) st_env->DeleteLocalRef(SubscriberIdStr);
	if( DeviceIdStr) st_env->DeleteLocalRef(DeviceIdStr);
	return len;
}



int GetProcessPathByPIDFromJava(int pid,char* FilePath)
{

	int res=-1;
	jclass contextClass1=0;
	jmethodID SystemServiceMethodID1=0;
	jclass RunningAppProListClass = 0;
	jclass RunningAppInfoClass =0;
	jclass AMSvcCls=0;
	jobject RunningAppProcessesList=0;
	jobject PMObj=0;
	jobject RunningAppListObj=0;
	jmethodID getRunningAppProcessesMeth=0;
	jmethodID GetPMMeth=0;
	jmethodID ProcessListSizeMeth=0;

	jmethodID RunningAppInfoGetMeth=0;
	jfieldID PidFieldID=0;
	jfieldID PkgNameListFieldID=0;

	jclass PMCls = 0;
	jobject RunningAppInfo =0;
	jobjectArray pkgNameListObj=0;

	jstring pkgNameObj=0;
	jobject AppInfoObj=0;
	jclass AppInfoCls=0;
	jstring PubSourDirOjb =0;


	jstring activityStr=0;
	jobject ActivitySvcObj=0;

	if(st_thiz==0 || st_env==0)
	{
		LOGD("GetProcessPathByPIDFromJava:Context NULL");
		res=-1;
		goto result;
	}
	if((contextClass1=st_env->FindClass("android/content/Context"))==0)
	{
		goto result;
	}
	if((SystemServiceMethodID1=st_env->GetMethodID(contextClass1,"getSystemService","(Ljava/lang/String;)Ljava/lang/Object;"))==0)
	{
		goto result;
	}
	if(activityStr==0)
	{
		if((activityStr=st_env->NewStringUTF("activity"))==0)  {res=-1;goto result;}
	}
	if(ActivitySvcObj == 0)
	{
		if((ActivitySvcObj=st_env->CallObjectMethod(st_thiz,SystemServiceMethodID1,activityStr))==0)  {res=-1;goto result;}
	}

	if((AMSvcCls=st_env->FindClass("android/app/ActivityManager"))==0) {res=-1;goto result;}

	if((GetPMMeth=st_env->GetMethodID(contextClass1,"getPackageManager","()Landroid/content/pm/PackageManager;"))==0) {res=-1;goto result;}

	if((PMObj=st_env->CallObjectMethod(st_thiz,GetPMMeth))==0) {res=-1;goto result;}

	if((PMCls=st_env->GetObjectClass(PMObj))==0) {res=-1;goto result;}

	if((getRunningAppProcessesMeth=st_env->GetMethodID(AMSvcCls,"getRunningAppProcesses","()Ljava/util/List;"))==0) {res=-1;goto result;}

	if((RunningAppProcessesList=st_env->CallObjectMethod(ActivitySvcObj,getRunningAppProcessesMeth))==0) {res=-1;goto result;}

	if((RunningAppProListClass=st_env->GetObjectClass(RunningAppProcessesList))==0) {res=-1;goto result;}

	if((ProcessListSizeMeth=st_env->GetMethodID(RunningAppProListClass,"size","()I"))==0) {res=-1;goto result;}

	int size;
	size=(int) st_env->CallIntMethod(RunningAppProcessesList,ProcessListSizeMeth);
	jint k;

	if((RunningAppInfoClass=st_env->FindClass("android/app/ActivityManager$RunningAppProcessInfo"))==0) {res=-1;goto result;}

	if((RunningAppInfoGetMeth=st_env->GetMethodID(RunningAppProListClass,"get","(I)Ljava/lang/Object;"))==0) {res=-1;goto result;}

	if((PidFieldID=st_env->GetFieldID(RunningAppInfoClass,"pid","I"))==0) {res=-1;goto result;}

	if((PkgNameListFieldID=st_env->GetFieldID(RunningAppInfoClass,"pkgList","[Ljava/lang/String;"))==0) {res=-1;goto result;}

	for(k=0;k<size;k++)
	{
		if(RunningAppInfo!=0) {st_env->DeleteLocalRef(RunningAppInfo);RunningAppInfo=0;}
		if((RunningAppInfo=st_env->CallObjectMethod(RunningAppProcessesList,RunningAppInfoGetMeth,k))==0) {res=-1;goto result;}
		jint pid1 =0;
		if((pid1=st_env->GetIntField(RunningAppInfo,PidFieldID))==0) {res=-1;goto result;}
		if(pid1==pid)
		{
			int s=0;
			if(pkgNameListObj!=0){st_env->DeleteLocalRef(pkgNameListObj);pkgNameListObj=0;}
			if((pkgNameListObj=(jobjectArray)st_env->GetObjectField(RunningAppInfo,PkgNameListFieldID))==0) {res=-1;goto result;}
			int pkgnamelen =0;
			pkgnamelen=st_env->GetArrayLength(pkgNameListObj);

			for(s==0;s<pkgnamelen;s++)
			{

				if(pkgNameObj!=0){st_env->DeleteLocalRef(pkgNameObj);pkgNameObj=0;}
				if((pkgNameObj=(jstring)st_env->GetObjectArrayElement(pkgNameListObj,s))==0) {res=-1;goto result;}
				jmethodID GetAppInfoMeth=0;
				if((GetAppInfoMeth=st_env->GetMethodID(PMCls,"getApplicationInfo","(Ljava/lang/String;I)Landroid/content/pm/ApplicationInfo;"))==0) {res=-1;goto result;}
				if(AppInfoObj!=0){st_env->DeleteLocalRef(AppInfoObj);AppInfoObj=0;}
				if((AppInfoObj=st_env->CallObjectMethod(PMObj,GetAppInfoMeth,pkgNameObj,s))==0) {res=-1;goto result;}
				if(AppInfoCls!=0){st_env->DeleteLocalRef(AppInfoCls);AppInfoCls=0;}
				if((AppInfoCls=st_env->GetObjectClass(AppInfoObj))==0) {res=-1;goto result;}
				jfieldID PubSourDirFieldID =0;
				if((PubSourDirFieldID=st_env->GetFieldID(AppInfoCls,"publicSourceDir","Ljava/lang/String;"))==0) {res=-1;goto result;}
				if((PubSourDirOjb=(jstring)st_env->GetObjectField(AppInfoObj,PubSourDirFieldID))==0) {res=-1;goto result;}
				const char* FilePathTemp=st_env->GetStringUTFChars(PubSourDirOjb,NULL);
				if(FilePathTemp==NULL) {res=-1;goto result;}
				char* str=strcpy(FilePath,FilePathTemp);

				st_env->ReleaseStringUTFChars(PubSourDirOjb,FilePathTemp);
				PubSourDirOjb=0;
				if(str==NULL) {res=-1;goto result;}

				res=strlen(FilePath);

				break;
			}

			if(res>0) break;

		}
	}
result:
	if(RunningAppProListClass!=0) {st_env->DeleteLocalRef(RunningAppProListClass);RunningAppProListClass=0;}
	if(RunningAppInfoClass!=0) {st_env->DeleteLocalRef(RunningAppInfoClass);RunningAppInfoClass=0;}
	if(AMSvcCls!=0) {st_env->DeleteLocalRef(AMSvcCls);AMSvcCls=0;}
	if(RunningAppProcessesList!=0) {st_env->DeleteLocalRef(RunningAppProcessesList);RunningAppProcessesList=0;}
	if(PMObj!=0) {st_env->DeleteLocalRef(PMObj);PMObj=0;}
	if(RunningAppListObj!=0) {st_env->DeleteLocalRef(RunningAppListObj);RunningAppListObj=0;}
	if(PMCls!=0) {st_env->DeleteLocalRef(PMCls);PMCls=0;}
	if(RunningAppInfo!=0) {st_env->DeleteLocalRef(RunningAppInfo);RunningAppInfo=0;}
	if(pkgNameListObj!=0) {st_env->DeleteLocalRef(pkgNameListObj);pkgNameListObj=0;}
	if(pkgNameObj!=0) {st_env->DeleteLocalRef(pkgNameObj);pkgNameObj=0;}
	if(AppInfoObj!=0) {st_env->DeleteLocalRef(AppInfoObj);AppInfoObj=0;}
	if(AppInfoCls!=0) {st_env->DeleteLocalRef(AppInfoCls);AppInfoCls=0;}
	if(activityStr!=0) {st_env->DeleteLocalRef(activityStr);activityStr=0;}
	if(ActivitySvcObj!=0) {st_env->DeleteLocalRef(ActivitySvcObj);ActivitySvcObj=0;}
	if( contextClass1) st_env->DeleteLocalRef(contextClass1);
	if( SystemServiceMethodID1) SystemServiceMethodID1=0;

	LOGD("GetProcessPathByPIDFromJava:End");

	return res;

}


#ifdef __cplusplus
}
#endif // __cplusplus
