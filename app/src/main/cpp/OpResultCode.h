/*
	S-SIM密码库对外提供的函数功能，以纯C语言的方式。
*/
#ifndef OP_RESULT_CODE_HEADER_FD90AHFS9JFD09SA___FD98AHFA__
#define OP_RESULT_CODE_HEADER_FD90AHFS9JFD09SA___FD98AHFA__

#define CONTAINER_TYPE_CA			0
//#define CONTAINER_KEY_CA			1

#define CONTAINER_TYPE_MAX			3
#define CONTAINER_TYPE_MIN			1

#define CONTAINER_KEY_MAX			16
#define CONTAINER_KEY_MIN			1

#define CONTAINER_ALG_INVALID		0xff
#define CONTAINER_ALG_MIN			0x01
#define CONTAINER_ALG_SM2			0x01
#define CONTAINER_ALG_SMS4			0x02
#define CONTAINER_ALG_AES			0x03
#define CONTAINER_ALG_RSA_1024		0x04
#define CONTAINER_ALG_RSA_2048		0x05
#define CONTAINER_ALG_MAX			0x05


#define FILE_ID_MIN				1
#define FILE_ID_MAX				32

#define FILE_LENGTH_MAX			256
#define PIN_LEVEL_0				(0)
#define PIN_LEVEL_1				(1)
#define PIN_LEVEL_2				(2)
#define PIN_LEVEL_UNKNOWN		(-1)

#define USER_CRED_STATUS_NO_KEY			(0)
#define USER_CRED_STATUS_HAVE_KEY		(1)
#define USER_CRED_STATUS_UNKNOWN		(-1)

#define USER_CRED_STATUS_KEY_CHANGED		(-2)

/* --=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=
	函数相关常数
--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--= */ 

#define SSIM_SUCCESS					(0)
#define SSIM_VERIFYFAILED_ERROR_OTHER_ERROR			(1000) //包验证失败
#define SSIM_VERIFYFAILED_ERROR_PUBLICKEY_ERROR			(1001) //公钥错误
#define SSIM_VERIFYFAILED_ERROR_FILE_ERROR  (1002) //签名文件错误
#define SSIM_VERIFYFAILED_ERROR_VERIFY_ERROR  (1003) //签名错误
#define SSIM_AGENTAPPID_INVALID_ERROR	(2) // AgentAppID无效
#define SSIM_CRL_FILE_ERROR				(3)
#define SSIM_STORE_ERROR				(4)
#define SSIM_USER_CREDENTIAL_UNAVAILABLE_ERROR	(5)
#define SSIM_PIN_LEVEL_ERROR					(6)
#define SSIM_TYPE_KEY_ALG_ID_ERROR				(7)
#define SSIM_EXISTING_ALG_ID_ERROR				(8)
#define SSIM_KEY_INVALID_ERROR			(9)
#define SSIM_DEK_NOT_READY_ERROR		(10)
#define SSIM_KEY_UNSUPPORT_ERROR		(11)
#define SSIM_FILE_ID_ERROR				(12)
#define SSIM_FILE_LENGTH_ERROR			(13)
#define SSIM_BUF_LENGTH_ERROR			(14)
#define SSIM_INPUT_LENGTH_ERROR			(15)
#define SSIM_SIGNATURE_VERIFY_ERROR		(16)
#define SSIM_JNI_PARAM_ERROR			(17)
#define SSIM_INPUT_DATA_ERROR			(18)
#define SSIM_APP_NOT_VERIRIED           (19)
#define SSIM_CA_SERVER_KEYERROR         (20)
#define SSIM_SERVERKEY_VERIFYERROR      (21)
#define SSIM_HASH_ERROR                 (22)
#define SSIM_SERVERMESSAGE_DECRYPT_ERROR (23)
// --=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=

#endif
