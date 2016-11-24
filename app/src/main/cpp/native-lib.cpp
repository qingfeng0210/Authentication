#include <jni.h>
#include <string>
#include "MediatedSM2Client.h"
#include "def.h"
#include "Message.h"
#include "MediatedSM2Service.h"
#include "MediatedSM2Stub.h"
#include "DeviceInfo1.h"
#include "ALog.h"




#define SM2_CIPHER_LENGTH(len) ((len) + 1 + 2 * DCS_ECC_KEY_LENGTH + SM3_HASH_256)
#define SM2_PLAIN_LENGTH(len) ((len) - 1 - 2 * DCS_ECC_KEY_LENGTH - SM3_HASH_256)

struct CUserCredentials
{
    int mode;		// 工作模式，0未设置，1自由，2联网
    bool ready; //是否成功产生了密钥
    byte userCredential[sizeof(CMediatedSM2Client)];	// 用户密钥
    union
    {
        byte userCredential_service[sizeof(CMediatedSM2Service)];	// 自由模式：用户密钥的服务端部分
        CMediatedSM2StubParam userCredential_stub; // 联网模式： 网络参数
    };
    byte encryptedDEK[SM2_CIPHER_LENGTH(SMS4_KEY_LENGTH)];	// DEK密文形式
    byte encryptedSDEK[SM2_CIPHER_LENGTH(SMS4_KEY_LENGTH)];	// SDEK密文形式

    CUserCredentials();
    ~CUserCredentials() {Close();}
    bool Save();
    void Close();
};

struct CUserCredentialsRuntime
{
    bool flagDek;	// DEK明文是否有效
    unsigned char keyDEK[SMS4_KEY_LENGTH];		// DEK密钥
    CSM4_Check cipher;
};


static CMediatedSM2Client *g_client = nullptr;
static CMediatedSM2Service *g_local = nullptr;
static CMediatedSM2Stub *g_online = nullptr;

static CUserCredentialsRuntime g_userCredentialsRuntime = { false };
static CUserCredentials g_userCredentials;



#define getjstring(s) const char *p_##s = NULL; int s##_len = 0; if (s) { p_##s = env->GetStringUTFChars(s, JNI_FALSE); s##_len = env->GetStringLength(s); }
#define releasejstring(s) if (p_##s) env->ReleaseStringUTFChars(s, p_##s)
#define getjbyteArray(s) byte* p_##s = NULL;  int s##_len = 0; if (s) { p_##s = (byte*)env->GetByteArrayElements(s, JNI_FALSE); s##_len = env->GetArrayLength(s); }
#define releasejbyteArray(s) if (p_##s) env->ReleaseByteArrayElements(s, (jbyte*)p_##s, 0)


#ifdef WIN32
#define FILE_ROOT_PATH				"./"
#define FILE_USER_CRED				"pdrf.0"
#define FILE_CONTAINER_KEY			"pdrf.%x.%x.%d.%.64s"
#else
#define FILE_ROOT_PATH				"/data/data/cn.dacas.security/"
#define FILE_USER_CRED				"/data/data/cn.dacas.security/pdrf.0"
#define FILE_CONTAINER_KEY			"/data/data/cn.dacas.security/pdrf.%x.%x.%d.%.64s"
#endif


//算法编号
const int KeyType_Root = 0;
const int KeyType_SM2 = 0x10000;
const int KeyType_SM2_Public = 0x20000;
const int KeyType_HMAC_SM3 = 0x30000;
const int KeyType_SM4 = 0x40000;
const int KeyType_CRL = 0xc0000;
const int KeyType_DATA = 0xd0000;

//密钥选项
const int KeyOption_None = 0;
const int KeyOption_Temp = 0x1000000;
const int KeyOption_Notify = 0x2000000;
const int KeyOption_Confirm = 0x3000000;
const int KeyOption_Auth = 0x4000000;

const int RootKeyId = KeyOption_Auth | KeyType_Root;
const int RootPublicKeyId =  KeyType_Root;


bool WriteFile(const char *path, const void *data, int length)
{
    FILE *f = fopen(path, "wb");
    if (!f) return false;
    int ret = fwrite(data, length, 1, f);
    fclose(f);
    return ret;
}

bool ReadFile(const char *path, void *data, int length)
{
    FILE *f = fopen(path, "rb");
    if (!f) return false;
    int ret = fread(data, 1, length, f);
    fclose(f);
    return ret == length;
}

CUserCredentials::CUserCredentials()
{
    LOGD("load user credentials");
    mode = 0;
    if (ReadFile(FILE_USER_CRED, this, sizeof(CUserCredentials)))
    {
        if (mode == 1) // local
        {
            LOGD("local mode");
            g_local = new CMediatedSM2Service(userCredential_service, sizeof(userCredential_service));
            g_client = new CMediatedSM2Client(*g_local);
            g_client->SetKey(userCredential, sizeof(userCredential));
        }
        else if (mode == 2) // online
        {
            LOGD("online mode");
            userCredential_stub.lastRequest = gettimems();
            g_online = new CMediatedSM2Stub(userCredential_stub);
            g_client = new CMediatedSM2Client(*g_online);
            g_client->SetKey(userCredential, sizeof(userCredential));
        }
    }
    if (!g_client)
    {
        mode = 0;
    }
}

void CUserCredentials::Close()
{
    LOGD("free user credentials");
    if (g_local) {
        delete g_local;
        g_local = nullptr;
    }
    if (g_online) {
        delete g_online;
        g_online = nullptr;
    }
    if (g_client) {
        delete g_client;
        g_client = nullptr;
    }
    memset(&g_userCredentials, 0, sizeof(g_userCredentials));
}

bool CUserCredentials::Save()
{
    LOGD("save user credentials");

    if (mode == 1) g_local->OutputKey(userCredential_service);
    else if (mode != 2) return false;
    g_client->OutputKey(userCredential);

    return WriteFile(FILE_USER_CRED, this, sizeof(CUserCredentials));
}

int SaveUserKey(JNIEnv *env, int keyId, byte *key, int length, jbyteArray pin, jstring appId)
{
    int ret = 0;
    char path[256];
    byte iv[32];
    getjstring(appId);
    sprintf(path, FILE_CONTAINER_KEY, (keyId >> 16) & 0xff, (keyId >> 24) & 0xff, keyId & 0xffff, p_appId);
    releasejstring(appId);

    CSecurity::Hash(path, strlen(path), iv);

    if (keyId & KeyOption_Auth)
    {
        byte sk[16];
        getjbyteArray(pin);
        ret = g_client->DecryptMessage(sk, g_userCredentials.encryptedSDEK, sizeof(g_userCredentials.encryptedSDEK), p_pin);
        releasejbyteArray(pin);
        if (ret <= 0) return pdrm::AUTH_REQUESRT;
        CSM4_Check sm4;
        sm4.SetKey(sk);
        sm4.EncryptWithIV(iv, key, length);
    }
    else
    {
        CSM4_Check &sm4 = g_userCredentialsRuntime.cipher;
        sm4.EncryptWithIV(iv, key, length);
    }
    if (!WriteFile(path, key, length)) ret = pdrm::READ_WRITE;
    return length;
}

int LoadUserKey(JNIEnv *env, int keyId, byte *key, int length, jbyteArray pin, jstring appId)
{
    int ret = 0;
    char path[256];
    byte iv[32];
    getjstring(appId);
    sprintf(path, FILE_CONTAINER_KEY, (keyId >> 16) & 0xff, (keyId >> 24) & 0xff, keyId & 0xffff, p_appId);
    releasejstring(appId);

    CSecurity::Hash(path, strlen(path), iv);

    FILE *f = fopen(path, "rb");
    if (!f) return pdrm::READ_WRITE;
    ret = fread(key, length, 1, f);
    fclose(f);

    if (ret <= 0) return pdrm::READ_WRITE;

    if (keyId & KeyOption_Auth)
    {
        byte sk[16];
        getjbyteArray(pin);
        ret = g_client->DecryptMessage(sk, g_userCredentials.encryptedSDEK, sizeof(g_userCredentials.encryptedSDEK), p_pin);
        releasejbyteArray(pin);
        if (ret <=0) return pdrm::AUTH_REQUESRT;
        CSM4_Check sm4;
        sm4.SetKey(sk);
        sm4.DecryptWithIV(iv, key, length);
    }
    else
    {
        CSM4_Check &sm4 = g_userCredentialsRuntime.cipher;
        sm4.DecryptWithIV(iv, key, length);
        ret = length;
    }

    return ret;
}



#ifdef __cplusplus
extern "C" {
#endif


jint JNICALL
        Java_cn_dacas_authentication_model_NativeCode_destroy(JNIEnv *env, jclass type) ;


JNIEXPORT jdouble JNICALL
Java_cn_dacas_authentication_model_NativeCode_collectData__Lbyte_3_093_2ID(JNIEnv *env, jclass type,
                                                                       jbyteArray data_,
                                                                       jint length,
                                                                       jdouble entropy) {
    if (!data_) return 0;
    getjbyteArray(data_);
    jdouble r = CRealRandom::CollectData(p_data_, data__len > length ? length : data__len, entropy);
    releasejbyteArray(data_);
    return r;
}

JNIEXPORT jint JNICALL
Java_cn_dacas_authentication_model_NativeCode_sm3Hash__Ljava_lang_String_2Lbyte_3_093_2(JNIEnv *env,
                                                                                    jclass type,
                                                                                    jstring str_,
                                                                                    jbyteArray hash_) {
    if (!str_ || !hash_ || env->GetArrayLength(hash_) < 32) return pdrm::ERROR_PARAMETER;
    getjstring(str_);
    getjbyteArray(hash_);
    CSecurity::Hash(p_str_, str__len, p_hash_);
    releasejbyteArray(hash_);
    releasejstring(str_);
    return pdrm::SUCCUCESS;
}

JNIEXPORT jint JNICALL
Java_cn_dacas_authentication_model_NativeCode_setWorkMode__ILjava_lang_String_2(JNIEnv *env,
                                                                            jclass type, jint mode,
                                                                            jstring serviceUri_) {
    Java_cn_dacas_authentication_model_NativeCode_destroy(env, type);
    if (mode == 1) // local
    {
        g_userCredentials.mode = mode;
        g_local = new CMediatedSM2Service();
        g_client = new CMediatedSM2Client(*g_local);
    }
    else if (mode == 2) // online
    {
        const char *uri = env->GetStringUTFChars(serviceUri_, JNI_FALSE);
        if (strlen(uri) == 0 && strlen(uri) >= sizeof(g_userCredentials.userCredential_stub.serverAddress))
        {
            env->ReleaseStringUTFChars(serviceUri_, uri);
            return pdrm::ERROR_PARAMETER;
        }
        strcpy(g_userCredentials.userCredential_stub.serverAddress, uri);
        strcpy(g_userCredentials.userCredential_stub.serverPort, "8800");

        env->ReleaseStringUTFChars(serviceUri_, uri);

        g_userCredentials.userCredential_stub.lastRequest = gettimems();

        g_userCredentials.mode = mode;
        g_online = new CMediatedSM2Stub(g_userCredentials.userCredential_stub);
        g_client = new CMediatedSM2Client(*g_online);
    }
    else
    {
        return pdrm::ERROR_PARAMETER;
    }
    g_userCredentials.ready = false;
    if (g_userCredentials.Save()) return pdrm::SUCCUCESS;
    else return pdrm::READ_WRITE;
}

JNIEXPORT jint JNICALL
Java_cn_dacas_authentication_model_NativeCode_initialize(JNIEnv *env, jclass type, jobject ctx,
                                                     jbyteArray pin_) {
    if (g_userCredentials.mode == 0) return pdrm::UNSUPPORT;
    if (!g_userCredentials.ready) return pdrm::UNREADY;
    if (!pin_) return pdrm::AUTH_REQUESRT;
    st_env = env;
    st_thiz= ctx;
    LOGD("pid uid\n");
    LOGD("pid=%d, uid=%d\n", getpid(),getuid());

    //if (!ValidateApp(getpid(),getuid(),".")) return pdrm::APP_CHECK_FAILURE;

    getjbyteArray(pin_);
    byte key[16];
    int iRet = g_client->DecryptMessage(key, g_userCredentials.encryptedDEK, sizeof(g_userCredentials.encryptedDEK), p_pin_);
    if (iRet > 0)
    {
        memcpy(g_userCredentialsRuntime.keyDEK, key, 16);
        g_userCredentialsRuntime.cipher.SetKey(g_userCredentialsRuntime.keyDEK);
        g_userCredentialsRuntime.flagDek = true;
    }
    releasejbyteArray(pin_);

    return iRet;
}

JNIEXPORT jint JNICALL
Java_cn_dacas_authentication_model_NativeCode_checkApp(JNIEnv *env, jclass type, jobject ctx, jint PID,
                                                   jint UID, jstring appID_) {
    //return pdrm::SUCCUCESS;
    st_env = env;
    if (ctx != nullptr) st_thiz= ctx;
    getjstring(appID_);
    //iRet=ValidateApp(PID, UID, (const char *) appID_);

    bool iRet=false;/*ValidateApp(PID, UID, (const char *) appID_);*/
    releasejstring(appID_);
    return iRet?pdrm::SUCCUCESS:pdrm::APP_CHECK_FAILURE;
}
JNIEXPORT void JNICALL
Java_cn_dacas_authentication_model_NativeCode_close(JNIEnv *env, jclass type) {

    // TODO
    LOGD("close");
    memset(g_userCredentialsRuntime.keyDEK, 0, sizeof(g_userCredentialsRuntime.keyDEK));
    g_userCredentialsRuntime.cipher.SetKey(g_userCredentialsRuntime.keyDEK);
    g_userCredentialsRuntime.flagDek = false;
}

JNIEXPORT jint JNICALL
Java_cn_dacas_authentication_model_NativeCode_destroy(JNIEnv *env, jclass type) {

    // TODO
    LOGD("destroy");
    Java_cn_dacas_authentication_model_NativeCode_close(env, type);
    g_userCredentials.Close();
#ifndef WIN32
    const char *path = FILE_ROOT_PATH;
    DIR *dir;
    dirent *dir_info;
    char file_path[PATH_MAX];
    if ((dir = opendir(path)) == NULL)
        return -1;
    while ((dir_info = readdir(dir)) != NULL)
    {
        if (strlen(dir_info->d_name) > 4 && 0 == memcmp(dir_info->d_name, "pdrf", 4))
        {
            strcpy(file_path, path);
            strcat(file_path, dir_info->d_name);
            remove(file_path);
        }
    }
#endif
    return pdrm::SUCCUCESS;
}

JNIEXPORT jint JNICALL
Java_cn_dacas_authentication_model_NativeCode_setCRL(JNIEnv *env, jclass type, jbyteArray CRL_,
                                                     jint lenCRL) {

    return 0;
}

JNIEXPORT jint JNICALL
Java_cn_dacas_authentication_model_NativeCode_getCode(JNIEnv *env, jclass type, jbyteArray img_) {
    if (g_userCredentials.mode != 2) return pdrm::UNSUPPORT;
    getjbyteArray(img_);

    int iRet = g_online->GetCode(p_img_, img__len);

    releasejbyteArray(img_);

    return iRet > 0 ? img__len : iRet;
}

JNIEXPORT jint JNICALL
Java_cn_dacas_authentication_model_NativeCode_register(JNIEnv *env, jclass type, jstring name_,
                                                   jstring password_, jstring code_) {
    if (g_userCredentials.mode != 2) return pdrm::UNSUPPORT;
    getjstring(name_);
    getjstring(password_);
    getjstring(code_);

    int iRet = g_online->Register(p_name_, p_password_, p_code_);

    releasejstring(name_);
    releasejstring(password_);
    releasejstring(code_);
    return iRet;
}
JNIEXPORT jint JNICALL
Java_cn_dacas_authentication_model_NativeCode_login(JNIEnv *env, jclass type, jstring name_,
                                                jstring password_, jstring code_) {
    if (g_userCredentials.mode != 2) return pdrm::UNSUPPORT;
    getjstring(name_);
    getjstring(password_);
    getjstring(code_);

    int iRet = g_online->Login(p_name_, p_password_, p_code_);
    if (iRet > 0) iRet = g_userCredentials.Save();

    releasejstring(name_);
    releasejstring(password_);
    releasejstring(code_);
    return iRet;
}
JNIEXPORT jint JNICALL
Java_cn_dacas_authentication_model_NativeCode_generateUserKeyPair(JNIEnv *env, jclass type,
                                                              jbyteArray pin_) {
    if (g_userCredentials.mode == 0) return pdrm::UNSUPPORT;
    getjbyteArray(pin_);
    int iRet = g_client->GenerateKey(p_pin_);
    if (iRet > 0)
    {
        byte r[16];
        CRealRandom::GetRandom(r, sizeof(r));
        g_client->EncryptMessage(g_userCredentials.encryptedSDEK, r, sizeof(r));
        CRealRandom::GetRandom(g_userCredentialsRuntime.keyDEK, sizeof(g_userCredentialsRuntime.keyDEK));
        g_client->EncryptMessage(g_userCredentials.encryptedDEK, g_userCredentialsRuntime.keyDEK, sizeof(g_userCredentialsRuntime.keyDEK));
        g_userCredentialsRuntime.cipher.SetKey(g_userCredentialsRuntime.keyDEK);

        g_userCredentialsRuntime.flagDek = true;
        g_userCredentials.ready = true;
        if (!g_userCredentials.Save()) {
            g_userCredentialsRuntime.flagDek = false;
            g_userCredentials.ready = false;
            iRet = 0;
        }
    }
    releasejbyteArray(pin_);
    return iRet;
}
JNIEXPORT jint JNICALL
Java_cn_dacas_authentication_model_NativeCode_getPublicKey(JNIEnv *env, jclass type, jint keyId,
                                                       jbyteArray pubKeyData_, jbyteArray pin_,
                                                       jstring appID_) {
    if (g_userCredentials.mode == 0) return pdrm::UNSUPPORT;
    if (!g_userCredentials.ready) return pdrm::UNREADY;
    if (!pubKeyData_ || env->GetArrayLength(pubKeyData_) < 64) return pdrm::ERROR_PARAMETER;
    getjbyteArray(pubKeyData_);
    int iRet = 64;

    switch (keyId & 0xFF0000) // key type
    {
        case KeyType_Root:
            iRet = g_client->ExportPublicKey(p_pubKeyData_);
            break;
        case KeyType_SM2:
        {
            byte p_key[3 * 32];
            iRet = LoadUserKey(env, keyId, p_key, sizeof(p_key), pin_, appID_);
            if (iRet > 0)
            {
                if (iRet != sizeof(p_key))
                {
                    iRet = pdrm::READ_WRITE;
                }
                else
                {
                    memcpy(p_pubKeyData_, p_key, 64);
                }
            }
        }
            break;
        case KeyType_SM2_Public:
        {
            byte p_key[2 * 32];
            iRet = LoadUserKey(env, keyId, p_key, sizeof(p_key), pin_, appID_);
            if (iRet > 0)
            {
                if (iRet != sizeof(p_key))
                {
                    iRet = pdrm::READ_WRITE;
                }
                else
                {
                    memcpy(p_pubKeyData_, p_key, 64);
                }
            }
        }
            break;
        case KeyType_HMAC_SM3:
        case KeyType_SM4:
        case KeyType_DATA:
        default:
            iRet = pdrm::UNSUPPORT;
            break;
    }

    releasejbyteArray(pubKeyData_);
    return iRet;
}
JNIEXPORT jint JNICALL
Java_cn_dacas_authentication_model_NativeCode_modifyPIN(JNIEnv *env, jclass type, jbyteArray oldPIN_,
                                                    jbyteArray newPIN_) {
    // TODO
    if (g_userCredentials.mode == 0) return pdrm::UNSUPPORT;
    if (!g_userCredentials.ready) return pdrm::UNREADY;
    if (!oldPIN_ || !newPIN_) return pdrm::ERROR_PARAMETER;
    int iRet;
    if (!g_userCredentialsRuntime.flagDek) {
        iRet = Java_cn_dacas_authentication_model_NativeCode_initialize(env, type, nullptr, oldPIN_);
        if (iRet <= 0) return iRet;
    }

    getjbyteArray(oldPIN_);
    getjbyteArray(newPIN_);
    iRet = g_client->ModifyPIN(p_oldPIN_, p_newPIN_);
    releasejbyteArray(oldPIN_);
    releasejbyteArray(newPIN_);

    if (!g_userCredentials.Save()) {
        Java_cn_dacas_authentication_model_NativeCode_close(env, type);
        iRet = 0;
    }
    return iRet;
}

#define CHECK_FIRST  if (g_userCredentials.mode == 0) return pdrm::UNSUPPORT;\
    if (!g_userCredentials.ready) return pdrm::UNREADY;\
    if (keyId < 0 || ((keyId & 0xff0000) != 0 && !appID_)) return pdrm::ERROR_PARAMETER;\
    if (!g_userCredentialsRuntime.flagDek) {\
        iRet = Java_cn_dacas_authentication_model_NativeCode_initialize(env, type, nullptr, pin_);\
        if (iRet <= 0) return iRet;\
    }\
    if (keyId & KeyOption_Auth)\
    {\
        if (!pin_ || env->GetArrayLength(pin_) != 32) return pdrm::AUTH_REQUESRT;\
    }

JNIEXPORT jint JNICALL
Java_cn_dacas_authentication_model_NativeCode_generateKey(JNIEnv *env, jclass type, jint keyId,
                                                      jbyteArray pin_, jstring appID_) {
    int iRet = 0;
    CHECK_FIRST;

    switch (keyId & 0xFF0000) // key type
    {
        case KeyType_SM2:
        {
            CECCPrivateKey sm2;
            while (iRet <= 0)
            {
                byte r[32];
                CRealRandom::GetRandom(r, sizeof(r));
                iRet = sm2.GenerateKey(r, sizeof(r));
            }
            byte p_key[3 * 32];
            sm2.OutputKey(p_key);
            iRet = SaveUserKey(env, keyId, p_key, sizeof(p_key), pin_, appID_);
            break;
        }
        case KeyType_HMAC_SM3:
        {
            byte p_key[32];
            CRealRandom::GetRandom(p_key, sizeof(p_key));
            iRet = SaveUserKey(env, keyId, p_key, sizeof(p_key), pin_, appID_);
            break;
        }
        case KeyType_SM4:
        {
            byte p_key[16];
            CRealRandom::GetRandom(p_key, sizeof(p_key));
            iRet = SaveUserKey(env, keyId, p_key, sizeof(p_key), pin_, appID_);
            break;
        }
        default:
            iRet = pdrm::UNSUPPORT;
    }

    return iRet;
}

JNIEXPORT jint JNICALL
Java_cn_dacas_authentication_model_NativeCode_importKey(JNIEnv *env, jclass type, jint keyId,
                                                    jbyteArray key_, jbyteArray pin_,
                                                    jstring appID_) {
    int iRet = 0;
    CHECK_FIRST;
    if (!key_) return pdrm::ERROR_PARAMETER;
    getjbyteArray(key_);

    switch (keyId & 0xFF0000) // key type
    {
        case KeyType_SM2:
        {
            CECCPrivateKey sm2;
            iRet = sm2.SetKey(p_key_, key__len);
            if (iRet > 0)
            {
                iRet = SaveUserKey(env, keyId, p_key_, 3*32, pin_, appID_);
            }
            break;
        }
        case KeyType_SM2_Public:
        {
            CECCPublicKey sm2;
            iRet = sm2.SetPublicKey(p_key_, key__len);
            if (iRet > 0)
            {
                iRet = SaveUserKey(env, keyId, p_key_, 2*32, pin_, appID_);
            }
            break;
        }
        case KeyType_HMAC_SM3:
            if (key__len < 32) iRet = pdrm::ERROR_PARAMETER;
            else iRet = SaveUserKey(env, keyId, p_key_, 32, pin_, appID_);
            break;
        case KeyType_SM4:
            if (key__len < 16) iRet = pdrm::ERROR_PARAMETER;
            else iRet = SaveUserKey(env, keyId, p_key_, 16, pin_, appID_);
            break;
        default:
            iRet = pdrm::UNSUPPORT;
    }
    releasejbyteArray(key_);
    return iRet;
}
JNIEXPORT jint JNICALL
Java_cn_dacas_authentication_model_NativeCode_decryptKey(JNIEnv *env, jclass type, jint keyId,
                                                     jbyteArray encryptedKey_, jbyteArray pin_,
                                                     jstring appID_) {
    int iRet = 0;
    CHECK_FIRST;
    byte p_key[3 * 32];

    getjbyteArray(encryptedKey_);
    getjbyteArray(pin_);

    if (sizeof(p_key) < SM2_PLAIN_LENGTH(encryptedKey__len)) iRet = pdrm::ERROR_PARAMETER;
    else iRet = g_client->DecryptMessage(p_key, p_encryptedKey_, encryptedKey__len, p_pin_);

    releasejbyteArray(pin_);
    releasejbyteArray(encryptedKey_);

    if (iRet > 0) {
        int key_len = iRet;
        switch (keyId & 0xFF0000) // key type
        {
            case KeyType_SM2:
            {
                CECCPrivateKey sm2;
                iRet = sm2.SetKey(p_key, key_len);
                if (iRet > 0)
                {
                    iRet = SaveUserKey(env, keyId, p_key, 3 * 32, pin_, appID_);
                }
                break;
            }
            case KeyType_SM2_Public:
            {
                CECCPublicKey sm2;
                iRet = sm2.SetPublicKey(p_key, key_len);
                if (iRet > 0)
                {
                    iRet = SaveUserKey(env, keyId, p_key, 2 *32, pin_, appID_);
                }
                break;
            }
            case KeyType_HMAC_SM3:
                if (key_len < 32) iRet = pdrm::ERROR_PARAMETER;
                else iRet = SaveUserKey(env, keyId, p_key, 32, pin_, appID_);
                break;
            case KeyType_SM4:
                if (key_len < 16) iRet = pdrm::ERROR_PARAMETER;
                else iRet = SaveUserKey(env, keyId, p_key, 16, pin_, appID_);
                break;
            default:
                iRet = pdrm::UNSUPPORT;
        }

    }

    return iRet;
}

JNIEXPORT jint JNICALL
Java_cn_dacas_authentication_model_NativeCode_removeKey(JNIEnv *env, jclass type, jint keyId,
                                                    jstring appID_) {
    char path[256];
    getjstring(appID_);
    sprintf(path, FILE_CONTAINER_KEY, (keyId >> 16) & 0xff, (keyId >> 24) & 0xff, keyId & 0xffff, p_appID_);
    releasejstring(appID_);

    remove(path);
    return pdrm::SUCCUCESS;
}


JNIEXPORT jint JNICALL
Java_cn_dacas_authentication_model_NativeCode_encrypt(JNIEnv *env, jclass type, jint keyId,
                                                  jbyteArray plain_, jbyteArray cipher_,
                                                  jbyteArray pin_, jstring appID_) {
    int iRet = 0;
    CHECK_FIRST;
    if (!plain_ || !cipher_) return pdrm::ERROR_PARAMETER;
    getjbyteArray(plain_);
    getjbyteArray(cipher_);

    switch (keyId & 0xFF0000) // key type
    {
        case KeyType_Root:
            if (cipher__len < SM2_CIPHER_LENGTH(plain__len)) iRet = pdrm::ERROR_PARAMETER;
            else iRet = g_client->EncryptMessage(p_cipher_, p_plain_, plain__len);
            break;
        case KeyType_SM2:
            if (cipher__len < SM2_CIPHER_LENGTH(plain__len)) iRet = pdrm::ERROR_PARAMETER;
            else
            {
                byte p_key[3 * 32];
                iRet = LoadUserKey(env, keyId, p_key, sizeof(p_key), pin_, appID_);
                if (iRet > 0)
                {
                    if (iRet != sizeof(p_key))
                    {
                        iRet = pdrm::READ_WRITE;
                    }
                    else
                    {
                        CECCPrivateKey sm2;
                        iRet = sm2.SetKey(p_key, sizeof(p_key));
                        if (iRet > 0)
                        {
                            byte rand[32];
                            iRet = 0;
                            while (iRet == 0)
                            {
                                CRealRandom::GetRandom(rand, sizeof(rand));
                                iRet = sm2.EncryptMessage(p_cipher_, p_plain_, plain__len, rand, 32);
                            }
                        }
                    }
                }
            }
            break;
        case KeyType_SM2_Public:
            if (cipher__len < SM2_CIPHER_LENGTH(plain__len)) iRet = pdrm::ERROR_PARAMETER;
            else
            {
                byte p_key[2 * 32];
                iRet = LoadUserKey(env, keyId, p_key, sizeof(p_key), pin_, appID_);
                if (iRet > 0)
                {
                    if (iRet != sizeof(p_key))
                    {
                        iRet = pdrm::READ_WRITE;
                    }
                    else
                    {
                        CECCPublicKey sm2;
                        iRet = sm2.SetPublicKey(p_key, sizeof(p_key));
                        if (iRet > 0)
                        {
                            byte rand[32];
                            iRet = 0;
                            while (iRet == 0)
                            {
                                CRealRandom::GetRandom(rand, sizeof(rand));
                                iRet = sm2.EncryptMessage(p_cipher_, p_plain_, plain__len, rand, 32);
                            }
                        }
                    }
                }
            }
            break;
        case KeyType_HMAC_SM3:
            iRet = pdrm::UNSUPPORT;
            break;
        case KeyType_SM4:
            if (cipher__len < plain__len)  iRet = pdrm::ERROR_PARAMETER;
            else {
                byte key[16];
                int length = 16;
                iRet = LoadUserKey(env, keyId, key, length, pin_, appID_);
                if (iRet > 0) {
                    CSM4 e;
                    e.SetKey(key);
                    e.Encrypt(p_plain_, plain__len, p_cipher_);
                    iRet = plain__len / 16 * 16;
                }
            }
            break;
        case KeyType_DATA:
            if (!appID_ || cipher__len < plain__len + 32) iRet = pdrm::ERROR_PARAMETER;
            else {
                byte *iv = p_cipher_ + plain__len;
                CSM3Hash sm3;
                getjstring(appID_);
                sm3.Init(&keyId, 4);
                sm3.Pending(p_appID_, appID__len);
                sm3.Pending(p_plain_, plain__len);
                sm3.Final(iv);
                releasejstring(appID_);
                byte sk[16];
                byte *p_key = 0;

                if (keyId & KeyOption_Auth) {
                    getjbyteArray(pin_);
                    iRet = g_client->DecryptMessage(sk, g_userCredentials.encryptedSDEK, sizeof(g_userCredentials.encryptedSDEK), p_pin_);
                    releasejbyteArray(pin_);
                    if (iRet > 0) p_key = sk;
                } else {
                    p_key = g_userCredentialsRuntime.keyDEK;
                }
                if (p_key){
                    memcpy(p_cipher_, p_plain_, plain__len);
                    CSecurity::EncryptCTR(p_key, iv, p_cipher_, plain__len);
                    iRet = plain__len + 32;
                }
            }
            break;
        default:
            iRet = pdrm::UNSUPPORT;
            break;
    }

    releasejbyteArray(plain_);
    releasejbyteArray(cipher_);
    return iRet;
}


JNIEXPORT jint JNICALL
Java_cn_dacas_authentication_model_NativeCode_decrypt(JNIEnv *env, jclass type, jint keyId,
                                                  jbyteArray cipher_, jbyteArray plain_,
                                                  jbyteArray pin_, jstring appID_) {
    int iRet = 0;
    CHECK_FIRST;
    if (!plain_ || !cipher_) return pdrm::ERROR_PARAMETER;
    getjbyteArray(plain_);
    getjbyteArray(cipher_);
    getjstring(appID_);
    getjbyteArray(pin_);

    switch (keyId & 0xFF0000) // key type
    {
        case KeyType_Root:
            if (!(keyId & KeyOption_Auth)) iRet = pdrm::UNSUPPORT;
            else if (plain__len < SM2_PLAIN_LENGTH(cipher__len)) iRet = pdrm::ERROR_PARAMETER;
            else iRet = g_client->DecryptMessage(p_plain_, p_cipher_, cipher__len, p_pin_);
            break;
        case KeyType_SM2:
            if (plain__len < SM2_PLAIN_LENGTH(cipher__len)) iRet = pdrm::ERROR_PARAMETER;
            else
            {
                byte p_key[3 * 32];
                iRet = LoadUserKey(env, keyId, p_key, sizeof(p_key), pin_, appID_);
                if (iRet > 0)
                {
                    if (iRet != sizeof(p_key))
                    {
                        iRet = pdrm::READ_WRITE;
                    }
                    else
                    {
                        CECCPrivateKey sm2;
                        iRet = sm2.SetKey(p_key, sizeof(p_key));
                        if (iRet > 0)
                        {
                            iRet = sm2.DecryptMessage(p_plain_, p_cipher_, cipher__len);
                        }
                    }
                }
            }
            break;
        case KeyType_SM2_Public:
            iRet = pdrm::UNSUPPORT;
            break;
        case KeyType_HMAC_SM3:
            iRet = pdrm::UNSUPPORT;
            break;
        case KeyType_SM4:
            if (cipher__len > plain__len)  iRet = pdrm::ERROR_PARAMETER;
            else {
                byte key[16];
                int length = 16;
                iRet = LoadUserKey(env, keyId, key, length, pin_, appID_);
                if (iRet > 0) {
                    CSM4 e;
                    e.SetKey(key);
                    e.Decrypt(p_cipher_, cipher__len, p_plain_);
                    iRet = plain__len / 16 * 16;
                }
            }
            break;
        case KeyType_DATA:
            if (!appID_ || cipher__len <= 32 || plain__len < cipher__len - 32) iRet = pdrm::ERROR_PARAMETER;
            else {
                byte *iv = p_cipher_ + plain__len;
                byte sk[32];
                byte *p_key = 0;

                if (keyId & KeyOption_Auth) {
                    getjbyteArray(pin_);
                    iRet = g_client->DecryptMessage(sk, g_userCredentials.encryptedSDEK, sizeof(g_userCredentials.encryptedSDEK), p_pin_);
                    releasejbyteArray(pin_);
                    if (iRet > 0) p_key = sk;
                }
                else {
                    p_key = g_userCredentialsRuntime.keyDEK;
                }
                if (p_key){
                    memcpy(p_plain_, p_cipher_, plain__len);
                    CSecurity::EncryptCTR(p_key, iv, p_plain_, plain__len);

                    CSM3Hash sm3;
                    sm3.Init(&keyId, 4);
                    sm3.Pending(p_appID_, appID__len);
                    sm3.Pending(p_plain_, plain__len);
                    sm3.Final(sk);
                    if (0 == memcmp(iv, sk, 32)) iRet = cipher__len - 32;
                    else iRet = pdrm::MESSAGE_AUTH_ERROR;
                }
            }
            break;
        default:
            iRet = pdrm::UNSUPPORT;
            break;
    }

    releasejbyteArray(pin_);
    releasejstring(appID_);
    releasejbyteArray(plain_);
    releasejbyteArray(cipher_);
    return iRet;
}

JNIEXPORT jint JNICALL
Java_cn_dacas_authentication_model_NativeCode_sign(JNIEnv *env, jclass type, jint keyId,
                                               jbyteArray message_, jbyteArray signature_,
                                               jbyteArray pin_, jstring appID_) {
    int iRet = 0;
    CHECK_FIRST;
    if (!message_ || !signature_) return pdrm::ERROR_PARAMETER;
    getjbyteArray(message_);
    getjbyteArray(signature_);
    getjbyteArray(pin_);

    switch (keyId & 0xFF0000) // key type
    {
        case KeyType_Root:
            if (!(keyId & KeyOption_Auth)) iRet = pdrm::UNSUPPORT;
            if (signature__len < DCS_ECC_KEY_LENGTH * 2) iRet = pdrm::ERROR_PARAMETER;
            else iRet = g_client->SignMessage(p_signature_, p_message_, message__len, "1234567812345678", 16, p_pin_);
            break;
        case KeyType_SM2:
            if (signature__len < DCS_ECC_KEY_LENGTH * 2) iRet = pdrm::ERROR_PARAMETER;
            else
            {
                byte p_key[3 * 32];
                iRet = LoadUserKey(env, keyId, p_key, sizeof(p_key), pin_, appID_);
                if (iRet > 0)
                {
                    if (iRet != sizeof(p_key))
                    {
                        iRet = pdrm::READ_WRITE;
                    }
                    else
                    {
                        CECCPrivateKey sm2;
                        iRet = sm2.SetKey(p_key, sizeof(p_key));
                        if (iRet > 0)
                        {
                            byte rand[32];
                            iRet = 0;
                            while (iRet == 0)
                            {
                                CRealRandom::GetRandom(rand, sizeof(rand));
                                iRet = sm2.SignMessage(p_signature_, p_message_, message__len, "1234567812345678", 16, rand, 32);
                            }
                        }
                    }
                }
            }
            break;
        case KeyType_HMAC_SM3:
            if (signature__len < 32) iRet = pdrm::ERROR_PARAMETER;
            else
            {
                byte p_key[32];
                iRet = LoadUserKey(env, keyId, p_key, sizeof(p_key), pin_, appID_);
                if (iRet > 0)
                {
                    if (iRet != sizeof(p_key))
                    {
                        iRet = pdrm::READ_WRITE;
                    }
                    else
                    {
                        Sm3Hmac(p_signature_, p_message_, message__len, p_key, sizeof(p_key));
                        iRet = 32;
                    }
                }
            }
            break;
        case KeyType_SM4:
            iRet = pdrm::UNSUPPORT;
            break;
        default:
            iRet = pdrm::UNSUPPORT;
    }

    releasejbyteArray(message_);
    releasejbyteArray(signature_);
    releasejbyteArray(pin_);
    return iRet;
}



JNIEXPORT jint JNICALL
Java_cn_dacas_authentication_model_NativeCode_verify(JNIEnv *env, jclass type, jint keyId,
                                                 jbyteArray message_, jbyteArray signature_,
                                                 jbyteArray pin_, jstring appID_) {
    int iRet = 0;
    CHECK_FIRST;
    if (!message_ || !signature_) return pdrm::ERROR_PARAMETER;

    getjbyteArray(message_);
    getjbyteArray(signature_);

    switch (keyId & 0xFF0000) // key type
    {
        case KeyType_Root:
            if (signature__len < DCS_ECC_KEY_LENGTH * 2) iRet = pdrm::ERROR_PARAMETER;
            else iRet = g_client->VerifyMessage(p_message_, message__len, p_signature_, DCS_ECC_KEY_LENGTH * 2, "1234567812345678", 16);
            break;
        case KeyType_SM2:
            if (signature__len < DCS_ECC_KEY_LENGTH * 2) iRet = pdrm::ERROR_PARAMETER;
            else
            {
                byte p_key[3 * 32];
                iRet = LoadUserKey(env, keyId, p_key, sizeof(p_key), pin_, appID_);
                if (iRet > 0)
                {
                    if (iRet != sizeof(p_key))
                    {
                        iRet = pdrm::READ_WRITE;
                    }
                    else
                    {
                        CECCPrivateKey sm2;
                        iRet = sm2.SetKey(p_key, sizeof(p_key));
                        if (iRet > 0)
                        {
                            iRet = sm2.VerifyMessage(p_message_, message__len, p_signature_, signature__len, "1234567812345678", 16);
                        }
                    }
                }
            }
            break;
        case KeyType_SM2_Public:
            if (signature__len < DCS_ECC_KEY_LENGTH * 2) iRet = pdrm::ERROR_PARAMETER;
            else
            {
                byte p_key[2 * 32];
                iRet = LoadUserKey(env, keyId, p_key, sizeof(p_key), pin_, appID_);
                if (iRet > 0)
                {
                    if (iRet != sizeof(p_key))
                    {
                        iRet = pdrm::READ_WRITE;
                    }
                    else
                    {
                        CECCPublicKey sm2;
                        iRet = sm2.SetPublicKey(p_key, sizeof(p_key));
                        if (iRet > 0)
                        {
                            iRet = sm2.VerifyMessage(p_message_, message__len, p_signature_, signature__len, "1234567812345678", 16);
                        }
                    }
                }
            }
            break;
        case KeyType_HMAC_SM3:
            if (signature__len < 8 || signature__len > 32) iRet = pdrm::ERROR_PARAMETER;
            else
            {
                byte p_key[32];
                iRet = LoadUserKey(env, keyId, p_key, sizeof(p_key), pin_, appID_);
                if (iRet > 0)
                {
                    if (iRet != sizeof(p_key))
                    {
                        iRet = pdrm::READ_WRITE;
                    }
                    else
                    {
                        byte hash[32];
                        Sm3Hmac(hash, p_message_, message__len, p_key, sizeof(p_key));
                        if (0 == memcmp(hash, p_signature_, signature__len)) iRet = 1;
                        else iRet = 0;
                    }
                }
            }
            break;
        case KeyType_SM4:
            iRet = pdrm::UNSUPPORT;
            break;
        default:
            iRet = pdrm::UNSUPPORT;
    }

    releasejbyteArray(message_);
    releasejbyteArray(signature_);
    return iRet;
}


JNIEXPORT jstring JNICALL
Java_cn_dacas_authentication_ui_LoginActivity_stringFromJNI(JNIEnv *env, jclass type) {

    // TODO

    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}


#ifdef __cplusplus
}
#endif
