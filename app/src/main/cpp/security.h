#pragma once
#include "3rd/SMS4.h"
#include "3rd/sm3hash.h"
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "def.h"

#ifdef TOMCRYPT
#include <tomcrypt.h>
#endif

inline std::string URLEncode(const std::string &sIn)
{
#define toHex(x) ((x) > 9 ? (x) - 10 + 'A' : (x) + '0')
	std::string sOut;
	for (size_t ix = 0; ix < sIn.size(); ix++)
	{
		byte buf[4];
		memset(buf, 0, 4);
		if (isalnum((byte)sIn[ix]))
		{
			buf[0] = sIn[ix];
		}
		//else if ( isspace( (byte)sIn[ix] ) ) //貌似把空格编码成%20或者+都可以
		//{
		//    buf[0] = '+';
		//}
		else
		{
			buf[0] = '%';
			buf[1] = toHex((byte)sIn[ix] >> 4);
			buf[2] = toHex((byte)sIn[ix] % 16);
		}
		sOut += (char *)buf;
	}
	return sOut;
};
inline std::string URLDecode(const std::string &sIn)
{
#define fromHex(x) (isdigit(x) ? (x) - '0' : (x) - 'A' + 10)
	std::string sOut;
	for (size_t ix = 0; ix < sIn.size(); ix++)
	{
		byte ch = 0;
		if (sIn[ix] == '%')
		{
			ch = (fromHex(sIn[ix + 1]) << 4);
			ch |= fromHex(sIn[ix + 2]);
			ix += 2;
		}
		else if (sIn[ix] == '+')
		{
			ch = ' ';
		}
		else
		{
			ch = sIn[ix];
		}
		sOut += (char)ch;
	}
	return sOut;
}

inline std::string HexEncode(const std::string &sIn)
{
#define toHex(x) ((x) > 9 ? (x) - 10 + 'A' : (x) + '0')
	std::string sOut;
	char buf[3] = { 0, 0, 0 };
	for (size_t ix = 0; ix < sIn.size(); ix++)
	{
		buf[0] = toHex((byte)sIn[ix] >> 4);
		buf[1] = toHex((byte)sIn[ix] % 16);
		sOut += buf;
	}
	return sOut;
};
inline std::string HexEncode(const void *sIn, int length)
{
#define toHex(x) ((x) > 9 ? (x) - 10 + 'A' : (x) + '0')
	std::string sOut;
	const byte *sB = (const byte*)sIn;
	char buf[3] = { 0, 0, 0 };
	for (int ix = 0; ix < length; ix++)
	{
		buf[0] = toHex(sB[ix] >> 4);
		buf[1] = toHex(sB[ix] % 16);
		sOut += buf;
	}
	return sOut;
};


class CBase64
{
public:
	static std::string Encode(const std::string &str)
	{
		std::string strOut;
		Encode(str.c_str(), str.size(), strOut);
		return strOut;
	}
	static std::string Decode(const std::string &str)
	{
		std::string strOut;
		strOut.resize((str.size() + 3) / 4 * 3);
		unsigned int outLen = strOut.size();
		if (!Decode(str, (unsigned char *)strOut.c_str(), &outLen))
		{
			outLen = 0;
		}
		strOut.resize(outLen);
		return strOut;
	}
	static bool Encode(const void *pIn2, unsigned long uInLen, std::string& strOut)
	{
		static const char *g_pCodes = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		unsigned long i, len2, leven;
		strOut = "";
		const unsigned char *pIn = (const unsigned char *)pIn2;

		//ASSERT((pIn != NULL) && (uInLen != 0) && (pOut != NULL) && (uOutLen != NULL));

		len2 = ((uInLen + 2) / 3) << 2;
		//if((*uOutLen) < (len2 + 1)) return false;

		//p = pOut;
		leven = 3 * (uInLen / 3);
		for (i = 0; i < leven; i += 3)
		{
			strOut += g_pCodes[pIn[0] >> 2];
			strOut += g_pCodes[((pIn[0] & 3) << 4) + (pIn[1] >> 4)];
			strOut += g_pCodes[((pIn[1] & 0xf) << 2) + (pIn[2] >> 6)];
			strOut += g_pCodes[pIn[2] & 0x3f];
			pIn += 3;
		}

		if (i < uInLen)
		{
			unsigned char a = pIn[0];
			unsigned char b = ((i + 1) < uInLen) ? pIn[1] : 0;
			unsigned char c = 0;

			strOut += g_pCodes[a >> 2];
			strOut += g_pCodes[((a & 3) << 4) + (b >> 4)];
			strOut += ((i + 1) < uInLen) ? g_pCodes[((b & 0xf) << 2) + (c >> 6)] : '=';
			strOut += '=';
		}

		//*p = 0; // Append NULL byte
		//*uOutLen = p - pOut;
		return true;
	}

	static bool Decode(const std::string& strIn, unsigned char *pOut, unsigned int *uOutLen)
	{
		return Decode(strIn.c_str(), strIn.length(), pOut, uOutLen);
	}
	static bool Encode(const void *inbuf, unsigned int inlen,
		void *outbuf, unsigned int *outlen)
	{
		static const char *codes = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

		unsigned long i, len2, leven;
		unsigned char *p;
		const unsigned char *in = (const unsigned char *)inbuf;
		unsigned char *out = (unsigned char *)outbuf;

		/* valid output size ? */
		len2 = 4 * ((inlen + 2) / 3);
		if (*outlen < len2 + 1) {
			*outlen = len2 + 1;
			return false;
		}
		p = out;
		leven = 3 * (inlen / 3);
		for (i = 0; i < leven; i += 3) {
			*p++ = codes[(in[0] >> 2) & 0x3F];
			*p++ = codes[(((in[0] & 3) << 4) + (in[1] >> 4)) & 0x3F];
			*p++ = codes[(((in[1] & 0xf) << 2) + (in[2] >> 6)) & 0x3F];
			*p++ = codes[in[2] & 0x3F];
			in += 3;
		}
		/* Pad it if necessary...  */
		if (i < inlen) {
			unsigned a = in[0];
			unsigned b = (i + 1 < inlen) ? in[1] : 0;

			*p++ = codes[(a >> 2) & 0x3F];
			*p++ = codes[(((a & 3) << 4) + (b >> 4)) & 0x3F];
			*p++ = (i + 1 < inlen) ? codes[(((b & 0xf) << 2)) & 0x3F] : '=';
			*p++ = '=';
		}

		/* append a NULL byte */
		*p = '\0';

		/* return ok */
		*outlen = p - out;
		return true;
	}

	static bool Decode(const void *inbuf, unsigned int inlen,
		void *outbuf, unsigned int *outlen)
	{
		static const unsigned char mapb[256] = {
			255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 62, 255, 255, 255, 63,
			52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255,
			255, 254, 255, 255, 255, 0, 1, 2, 3, 4, 5, 6,
			7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
			19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255,
			255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
			37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
			49, 50, 51, 255, 255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255
		};

		unsigned long t, x, y, z;
		unsigned char c;
		int           g;

		const unsigned char *in = (const unsigned char *)inbuf;
		unsigned char *out = (unsigned char *)outbuf;

		g = 3;
		for (x = y = z = t = 0; x < inlen; x++) {
			c = mapb[in[x] & 0xFF];
			if (c == 255) continue;
			/* the final = symbols are read and used to trim the remaining bytes */
			if (c == 254) {
				c = 0;
				/* prevent g < 0 which would potentially allow an overflow later */
				if (--g < 0) {
					return false;
				}
			}
			else if (g != 3) {
				/* we only allow = to be at the end */
				return false;
			}

			t = (t << 6) | c;

			if (++y == 4) {
				if (z + g > *outlen) {
					return false;
				}
				out[z++] = (unsigned char)((t >> 16) & 255);
				if (g > 1) out[z++] = (unsigned char)((t >> 8) & 255);
				if (g > 2) out[z++] = (unsigned char)(t & 255);
				y = t = 0;
			}
		}
		if (y != 0) {
			return false;
		}
		*outlen = z;
		return true;
	}
};

class CCipher
{
public:
	CCipher()	{ }
	virtual~CCipher(){}
	bool SetEncodeKey(const std::string &skey)
	{
		if (skey.length() == 16)
		{
			SetKey((byte*)skey.c_str());
			return true;
		}
		byte key[32];
		unsigned int len = 32;
		if (CBase64::Decode(skey, key, &len) && len >= 16)
		{
			if (len == 32)
			{
				for (int i = 0; i < 16; i++) key[i] ^= key[i + 16];
			}
			SetKey(key);
			return true;
		}
		return false;
	}
	virtual void SetKey(const unsigned char key[/*16*/])	{ }
	virtual void EncryptOne(const void * data, void *out) = 0;
	virtual void DecryptOne(const void * data, void *out) = 0;
	template<typename T>
	void Encrypt(T & data)
	{
		Encrypt(&data, sizeof(data));
	}
	template<typename T>
	void Decrypt(T & data)
	{
		Decrypt(&data, sizeof(data));
	}
	void Encrypt(void * data, int len)
	{
		Encrypt(data, len, data);
	}
	void Decrypt(void * data, int len)
	{
		Decrypt(data, len, data);
	}
	virtual void Encrypt(const void * data, int len, void * out)
	{
		unsigned char *p = (unsigned char*)data;
		unsigned char *c = (unsigned char*)out;
		for (int i = 0; i+16<=len; i += 16, p += 16, c += 16)
		{
			EncryptOne(p, c);
		}
	}
	virtual void Decrypt(const void * data, int len, void * out)
	{
		unsigned char *c = (unsigned char*)data;
		unsigned char *p = (unsigned char*)out;
		for (int i = 0; i+16<=len; i += 16, c += 16, p += 16)
		{
			DecryptOne(c, p);
		}
	}
	void IncIv(byte *iv)
	{
		int &i = *(int*)iv;
		i++;
	}
	void EncryptAuth(const void *_iv, void *data, int len, void *auth=0, int auth_len=16)
	{
		byte *d = (byte*)data;
		byte iv[16], a[16];
		if (auth && auth_len) {
			byte h[32];
			Sm3Hash(h, (byte*)data, len);
			memcpy(auth, h, auth_len);
		}
		EncryptOne(_iv, iv);
		while(len > 16) {
			IncIv(iv);
			EncryptOne(iv, a);
			for (int i = 0; i < 16; i++) {
				d[i] ^= a[i];
			}
			d += 16;
			len -= 16;
		}
		if (len > 0) {
			IncIv(iv);
			EncryptOne(iv, a);
			for (int i = 0; i < len; i++) {
				d[i] ^= a[i];
			}
		}
	}
	bool DecryptAuth(const void *_iv, void *data, int length, void *auth = 0, int auth_len = 16)
	{
		byte *d = (byte*)data;
		byte iv[16], a[16];
		int len = length;
		EncryptOne(_iv, iv);
		while (len > 16) {
			IncIv(iv);
			EncryptOne(iv, a);
			for (int i = 0; i < 16; i++) {
				d[i] ^= a[i];
			}
			d += 16;
			len -= 16;
		}
		if (len > 0) {
			IncIv(iv);
			EncryptOne(iv, a);
			for (int i = 0; i < len; i++) {
				d[i] ^= a[i];
			}
		}
		if (auth && auth_len) {
			byte h[32];
			Sm3Hash(h, (byte*)data, length);
			return 0 == memcmp(auth, h, auth_len);
		}
		return true;
	}
};

template<typename Cipher>
class CCheckMode : public Cipher
{
public:
	CCheckMode()
	{
	}
	using Cipher::Encrypt;
	using Cipher::Decrypt;
	void EncryptWithIV(const void *_iv, void *data, int length)
	{
		unsigned int *p = (unsigned int*)data;
		const unsigned int *iv = (const unsigned int*)_iv;
		for (int i = 0; i < 16 && i < length; i++) {
			p[i] ^= iv[i];
		}

		Encrypt(data, length);

		if ((length % 16) != 0) {
			byte ivs[16];
			if (length > 16) Cipher::EncryptOne(data, ivs);
			else Cipher::EncryptOne(iv, ivs);
			for (int i = (length / 16 * 16); i < length; i++) {
				p[i] ^= ivs[i % 16];
			}
		}
	}
	void DecryptWithIV(const void *_iv, void *data, int length)
	{
		unsigned int *p = (unsigned int*)data;
		const unsigned int *iv = (const unsigned int*)_iv;
		if ((length % 16) != 0) {
			byte ivs[16];
			if (length > 16) Cipher::EncryptOne(data, ivs);
			else Cipher::EncryptOne(iv, ivs);
			for (int i = (length / 16 * 16); i < length; i++) {
				p[i] ^= ivs[i % 16];
			}
		}

		Decrypt(data, length);

		for (int i = 0; i < 16 && i < length; i++) {
			p[i] ^= iv[i];
		}
	}
	void Encrypt(const void * data, int len, void * out) override
	{
		if (len < 16) return;
		const unsigned int *p = (const unsigned int*)data;
		unsigned int *c = (unsigned int*)out;
		len /= 16;
		unsigned int IV[4] = {0, 0, 0, 0};
		for (unsigned int i=0; i<len; i++, p+=4)
		{
			IV[0] ^= p[0]; 
			IV[1] ^= p[1]; 
			IV[2] ^= p[2]; 
			IV[3] ^= p[3];
		}

		Cipher::EncryptOne(IV, c);

		p = (unsigned int*)data;

		for (unsigned int i=1; i<len; i++)
		{
			p+=4;
			IV[0] = p[0] ^ c[0];
			IV[1] = p[1] ^ c[1]; 
			IV[2] = p[2] ^ c[2];
			IV[3] = p[3] ^ c[3]; 
			c+=4;
			Cipher::EncryptOne(IV, c);
		}
	}
	void Decrypt(const void * data, int len, void * out) override
	{
		if (len < 16) return;
		unsigned int *p = (unsigned int*)out;
		const unsigned int *c = (const unsigned int*)data;
		len /= 16;
		unsigned int i;

		for (i=len-1, p += 4 * i, c += 4 * i; i>0; i--)
		{
			Cipher::DecryptOne(c, p);
			c-=4;
			p[0] ^= c[0]; 
			p[1] ^= c[1];
			p[2] ^= c[2];
			p[3] ^= c[3]; 
			p-=4;
		}
		Cipher::DecryptOne(c, p);
		
		c = p + 4;
		for (i=1; i<len; i++, c+=4)
		{
			p[0] ^= c[0];
			p[1] ^= c[1];
			p[2] ^= c[2]; 
			p[3] ^= c[3];
		}
	}
};
class CNoCipher : public CCipher
{
public:
	void EncryptOne(const void * data, void *out) {}
	void DecryptOne(const void * data, void *out) {}
};
class CSM4 : public CCipher
{
protected:
	unsigned int subkey[32];
public:
	CSM4()
	{
	}
	void SetKey(const unsigned char sm4key[/*16*/]) override
	{
		Sms4ExtendKey(subkey, (unsigned char *)sm4key);
	}
	void EncryptOne(const void * data, void * out) override
	{
		const unsigned char *p = (const unsigned char*)data;
		unsigned char *c = (unsigned char*)out;
		Sms4Encrypt(c, p, subkey);
	}
	void DecryptOne(const void * data, void * out) override
	{
		const unsigned char *c = (const unsigned char*)data;
		unsigned char *p = (unsigned char*)out;
		Sms4Decrypt(p, c, subkey);
	}
};

typedef CCheckMode<CSM4> CSM4_Check;

#ifdef TOMCRYPT
class CAES128 : public CCipher
{
protected:
	symmetric_key aeskey;
public:
	CAES128()
	{
	}
	void SetKey(const unsigned char keybuf[/*16*/]) override
	{
		if ((aes_setup(keybuf, 16, 0, &aeskey)) != CRYPT_OK)
		{
			printf("AES key setup error\n");
		}
	}
	void EncryptOne(const void * data, void * out) override
	{
		const unsigned char *p = (const unsigned char*)data;
		unsigned char *c = (unsigned char*)out;
		aes_ecb_encrypt(p, c, &aeskey);
	}
	void DecryptOne(const void * data, void * out) override
	{
		const unsigned char *c = (const unsigned char*)data;
		unsigned char *p = (unsigned char*)out;
		aes_ecb_decrypt(c, p, &aeskey);
	}
};

typedef CCheckMode<CAES128> CAES128_Check;
#endif



class CSM3Hash
{
	SM3_HASH_STATE state;
	int index;
public:
	void Init(const void *pIn, unsigned int iInLen)	// 第一次调用
	{
		Sm3HashInit(&state, (const unsigned char *)pIn, iInLen);
		//state.H[0] = 0x7380166F;
		//state.H[1] = 0x4914B2B9;
		//state.H[2] = 0x172442D7;
		//state.H[3] = 0xDA8A0600;
		//state.H[4] = 0xA96F30BC;
		//state.H[5] = 0x163138AA;
		//state.H[6] = 0xE38DEE4D;
		//state.H[7] = 0xB0FB0E4E;
		//index = 0;
		//Pending(pIn, iInLen);

	}
	void Pending(const void *pIn, unsigned int iInLen)		// 任意多次调用
	{
		Sm3HashPending(&state, (const unsigned char *)pIn, iInLen);
		/*byte *p = (byte*)pIn;
		byte *h = (byte*)state.H;
		for (int i = 0; i < iInLen; i++){
			h[index] ^= p[i];
			index = (index + 1) % 32;
		}*/
	}
	bool Final(unsigned char *pOut)	// 最后一次调用
	{
		return Sm3HashFinal(pOut, &state);
		/*unsigned int *out = (unsigned int *)pOut;
		out[7] = state.H[7];
		for (int i = 6; i >= 0; i--)
		{
			out[i] = out[i + 1] ^ state.H[i];
		}
		return true;*/
	}
};

class CSecurityRandom
{
	SM3_HASH_STATE state[4];
	static void GetRandom(void *out, int len)
	{
		char *p = (char*)out;
#ifdef WIN32
		srand((unsigned int)time(NULL) + (unsigned int)out + (unsigned int)p[0] * len);
		for (int i = 0; i<len; i++)
		{
			p[i] = (char)rand();
		}
#else
		static int dev_random_fd = -1;
		if (dev_random_fd == -1)
		{
			dev_random_fd = open("/dev/urandom", O_RDONLY);
			assert(dev_random_fd != -1);
		}
		do
		{
			len -= read(dev_random_fd, p, len);

		} while (len > 0);
#endif
	}
public:
	CSecurityRandom() {
		byte r[32];
		for (int i = 0; i < 4; i++) {
			GetRandom(r, 32);
			Sm3HashInit(&state[i], r, 32);
			__int64_t t = gettimems();
			Sm3HashPending(&state[i], (byte*)&t, 8);
		}
	}

	
};

class CSecurity
{
public:
	static void GetRandom(void *out, int len)
	{
		char *p = (char*)out;
#ifdef WIN32
		srand((unsigned int)time(NULL) + (unsigned int)out + (unsigned int)p[0] * len);
		for (int i = 0; i<len; i++)
		{
			p[i] = (char)rand();
		}
#else
		static int dev_random_fd = -1;
		if (dev_random_fd == -1)
		{
			dev_random_fd = open("/dev/urandom", O_RDONLY);
			assert(dev_random_fd != -1);
		}
		do
		{
			len -= read(dev_random_fd, p, len);

		}while(len > 0);
#endif
	}
	static void GetRealRandom(void *out, int len)
	{
		char *p = (char*)out;
#ifdef WIN32
		srand((unsigned int)time(NULL));
		for (int i = 0; i<len; i++)
		{
			p[i] = (char)rand();
		}
#else
		static int dev_random_fd = -1;
		if (dev_random_fd == -1)
		{
			dev_random_fd = open("/dev/random", O_RDONLY);
			assert(dev_random_fd != -1);
		}
		do
		{
			len -= read(dev_random_fd, p, len);

		} while (len > 0);
#endif
	}
	static bool Hash(const void *pIn, unsigned int iInLen, unsigned char *pOut)
	{
		CSM3Hash sm3;
		sm3.Init(pIn, iInLen);
		return sm3.Final(pOut);
	}
	static bool ComputeAuthKey(const char *user, const char *password, unsigned char *pOut)
	{
		CSM3Hash sm3;
		sm3.Init("AUTHKEY:", 8);
		sm3.Pending(user, strlen(user));
		sm3.Pending("/", 1);
		sm3.Pending(password, strlen(password));
		return sm3.Final(pOut);
	}
	static void EncryptCTR(const byte key[], const void *CTR, void *data, int length)
	{
		if (!key || !CTR || !data || !length) return;
		CSM4 sm4;
		sm4.SetKey(key);
		
		byte ctr[16], c[16];
		unsigned int &i_ctr = *(unsigned int*)ctr;
		byte *d = (byte*)data;
		memcpy(ctr, CTR, 16);
		for (int i = 0; i < length; i_ctr++) {
			sm4.EncryptOne(ctr, c);
			for (int j = 0; j < 16 && i < length; j++, i++) {
				d[i] ^= c[j];
			}
		}
	}
	template<typename T>
	static bool Encrypt(const std::string &key, T &t)
	{
		CSM4_Check sm4;
		if (sm4.SetEncodeKey(key))
		{
			sm4.Encrypt(t);
			return true;
		}
		return false;
	}
	template<typename T>
	static bool Decrypt(const std::string &key, T &t)
	{
		CSM4_Check sm4;
		if (sm4.SetEncodeKey(key))
		{
			sm4.Decrypt(t);
			return true;
		}
		return false;
	}
	template<typename T>
	static bool Encrypt(const byte key[], T &t)
	{
		CSM4_Check sm4;
		sm4.SetKey(key);
		sm4.Encrypt(t);
		return true;
	}
	template<typename T>
	static bool Decrypt(const byte key[], T &t)
	{
		CSM4_Check sm4;
		sm4.SetKey(key);
		sm4.Decrypt(t);
		return true;
	}
};