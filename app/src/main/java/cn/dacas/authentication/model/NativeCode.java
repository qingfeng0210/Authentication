package cn.dacas.authentication.model;

import android.content.Context;
import android.util.Log;

public class NativeCode {
    static {
        System.loadLibrary("native-lib");
    }

    //算法编号
    public static final int KeyType_Root           = 0;
    public static final int KeyType_SM2            = 0x10000;
    public static final int KeyType_SM2_Public    = 0x20000;
    public static final int KeyType_HMAC_SM3      = 0x30000;
    public static final int KeyType_SM4            = 0x40000;
    public static final int KeyType_CRL            = 0xC0000;
    public static final int KeyType_DATA           = 0xD0000;

    //密钥选项
    public static final int KeyOption_Temp        = 0x1000000;
    public static final int KeyOption_Notify      = 0x2000000;
    public static final int KeyOption_Confirm     = 0x3000000;
    public static final int KeyOption_Auth        = 0x4000000;

    public static final int RootKeyId = KeyOption_Auth | KeyType_Root;
    public static final int RootPublicKeyId = KeyType_Root;

    public static final int SM2CipherAppand = 32 * 3 + 1;
    public static byte[] sm3Hash(String str)
    {
        byte[] hash = new byte[32];
        sm3Hash(str, hash);
        return hash;
    }
    public static void tryTest() {
        byte []pin=new byte[32];
        int iHash=NativeCode.sm3Hash("123456",pin);
        Log.d("PostID", "onClick: iHash: "+iHash);
    }
    public static double collectData(byte[] data)
    {
        return collectData(data, data.length, 0.01);
    }

    public static native double collectData(byte[] data, int length, double entropy);//熵
    public static native int sm3Hash(String str, byte[] hash);


    private static native int setWorkMode(int mode, String serviceUri); // 格式化
    public static native int initialize(Context ctx, byte[] pin);
    public static native void close();
    public static native int destroy();

    public static native int checkApp(Context ctx, int PID, int UID, String appID);
    public static native int setCRL(byte[] CRL,int lenCRL);

    public static native int getCode(byte[] img);
    public static native int register(String name, String password, String code);
    public static native int login(String name, String password, String code);

    public static native int generateUserKeyPair(byte[] pin);
    public static native int getPublicKey(int keyId, byte[] pubKeyData, byte[] pin, String appID);
    public static native int modifyPIN(byte[] oldPIN, byte[] newPIN);// 字符串pin要调用sm3Hash变成32字节

    public static native int generateKey(int keyId, byte[] pin, String appID); // keyId的前2字节为密钥编号，第三字节为算法编号，第四字节为选项
    public static native int importKey(int keyId, byte[] key, byte[] pin, String appID);
    public static native int decryptKey(int keyId, byte[] encryptedKey, byte[] pin, String appID);
    public static native int removeKey(int keyId, String appID);

    public static native int encrypt(int keyId, byte[] plain, byte[] cipher, byte[] pin, String appID);
    public static native int decrypt(int keyId, byte[] cipher, byte[] plain, byte[] pin, String appID);
    public static native int sign(int keyId, byte[] message, byte[] signature, byte[] pin, String appID);
    public static native int verify(int keyId, byte[] message, byte[] signature, byte[] pin, String appID);



}