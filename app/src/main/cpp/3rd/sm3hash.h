// SM3 Hash计算

#ifndef DCS_ECC_HEADER_DF0AJRW_DF90Y834TERN90_F98AH4____F98AH4___F9A8HTR9______5784__
#define DCS_ECC_HEADER_DF0AJRW_DF90Y834TERN90_F98AH4____F98AH4___F9A8HTR9______5784__

#define SM3_HASH_256				32	// length = 256 bits = 32 bytes

// 一次性完成, 输出在pOut, 长度是SM3_HASH_256
bool Sm3Hash(unsigned char *pOut, const unsigned char *pIn, unsigned int iInLen /* in bytes */);


// 多次调用
typedef struct _SM3_HASH_STATE
{
	unsigned int H[8];		// 状态寄存器
	unsigned char BB[64];	// 未处理的数据, 有效长度是(u64Length % 64)

#ifdef WIN32
	unsigned __int64 u64Length;
#else	// ARM ads, linux
	unsigned long int u64Length;
#endif
} SM3_HASH_STATE;

void Sm3HashInit(SM3_HASH_STATE *pState, const unsigned char *pIn, unsigned int iInLen);	// 第一次调用
void Sm3HashPending(SM3_HASH_STATE *pState, const unsigned char *pIn, unsigned int iInLen);		// 任意多次调用
bool Sm3HashFinal(unsigned char *pOut, SM3_HASH_STATE *pState);	// 最后一次调用


// KDF用在SM2的加解密中
unsigned int Sm3KDF(unsigned char *pKeyOut, unsigned int iLenOfOut /* in bytes */, const unsigned char *pSecret, unsigned int iLenOfSecret /* in bytes */, unsigned int ct = 1);




// ----------------------------------------------------------------------
// SM3 HMAC
#define HMAC_B_LENGTH		64
#define HMAC_IPAD			0x36
#define HMAC_OPAD			0x5c

bool Sm3Hmac(unsigned char *pOut, const unsigned char *pMsg, unsigned int iLenOfMsg, const unsigned char *pSecret, int iLenOfSecret);

typedef struct _SM3_HMAC_STATE
{
	unsigned char padding [HMAC_B_LENGTH];
	SM3_HASH_STATE hashState;
} SM3_HMAC_STATE;

void Sm3HmacInit(SM3_HMAC_STATE *pState, const unsigned char *pSecret, int iLenOfSecret);	// 第一次调用
void Sm3HmacPending(SM3_HMAC_STATE *pState, const unsigned char *pIn, unsigned int iInLen);		// 任意多次调用
bool Sm3HmacFinal(unsigned char *pOut, SM3_HMAC_STATE *pState);	// 最后一次调用



#endif // DCS_ECC_HEADER_DF0AJRW_DF90Y834TERN90_F98AH4____F98AH4___F9A8HTR9______5784__
