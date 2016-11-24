#pragma once
#include "MediatedSM2.h"
#include "def.h"
#include "security.h"

#define MaxRequestSize 256
#define MaxReplySize (1024 * 4)

namespace pdrm
{
	struct CMessage
	{
		byte at; // '@'
		byte flags;// 0
		short length; // ���ݳ���
		union{
			int function;
			int reply;
		};
		union {
			struct {
				__uint64_t session;
				__int64_t time;
			};
			byte iv[16];
		};
		byte signature[8];

		byte *data()
		{
			return ((byte*)this) + sizeof(CMessage);
		}
		int totalLength() const
		{
			return sizeof(CMessage) + length;
		}

		CMessage()
		{
			Init();
		}
		void Init()
		{
			at = '@';
			flags = 0;
			length = 0;
			function = 0;
			session = time = 0;
		}
		void box(CCipher &c)
		{
			c.EncryptAuth(iv, data(), length, signature, sizeof(signature));
		}
		bool unbox(CCipher &c)
		{
			return c.DecryptAuth(iv, data(), length, signature, sizeof(signature));
		}
	};
	template<int MaxSize>
	struct CMessageT : public CMessage
	{
		byte _data[MaxSize];
	};

	#define MESSAGE_MIN_LENGTH (sizeof(CMessage))

	enum RequestType
	{
		COD = 0x444f4300, // code
		REG = 0x47455201, // register
		LGN = 0x4e474c02, // login
		GEN = 0x4e454703, // GenerateKey
		SIG = 0x47495304, // sign
		DEC = 0x43454405, // Decrypt
		PIN = 0x4e495006, // changePIN
	};
	enum ReplyCode
	{
		SUCCUCESS			= 1,
		UNKNOW				= 0,
		ERROR_PIN			= 0x80000000,
		NAME_REGISTERED		= 0x80000001,
		TOO_MANY_USER		= 0x80000002,
		ERROR_LOGIN			= 0x80000003,
		ERROR_SESSION		= 0x80000003,
		UNKNOW_FUNCTION		= 0x80000004,
		ERROR_PARAMETER		= 0x80000005,
		ERROR_CHECK_CODE	= 0x80000006,
		AUTH_REQUESRT		= 0x80000007,
		KEY_LOCKED			= 0x80000008,
		UNREADY				= 0x80000009,


		PARAMETER_ERROR		= 0x80000100,
		BUFFER_ERROR		= 0x80000101,
		NETWORK_ERROR		= 0x80000102,
		PROTOCOL_ERROR		= 0x80000103,
		MAC_ERROR			= 0x80000104,
		TIME_ERROR			= 0x80000105,

		APP_CHECK_FAILURE   = 0x90000002,
		UNSUPPORT           = 0x90000006,
		MESSAGE_AUTH_ERROR	= 0x90000007,
		READ_WRITE			= 0x90000009,
	};

	struct CGetCode
	{
		char phone[16];
	};

	struct CRegister
	{
		char name[32];
		byte password[16];
		byte tempkey[16];
		char code[8];
	};

	struct CRegisterReply
	{
		__int64_t session;
		__int64_t time;
		byte sessionKey[16];
	};

	struct CLogin
	{
		char name[32];
		byte password[16];
		byte tempkey[16];
		char code[8];
	};
 
	struct CLoginReply
	{
		__int64_t session;
		__int64_t time;
		byte sessionKey[16];
	};

	struct CGenerateKey
	{
		byte pin[32];
		byte p1[64];
	};
	struct CGenerateKeyReply
	{
		byte p2[64];
	};
	struct CModifyPIN
	{
		byte oldpin[32];
		byte newpin[32];
	};
	struct CSign
	{
		byte pin[32];
		byte e[32];
		byte q1[64];
	};
	struct CSignReply
	{
		byte r[32];
		byte s2[32];
		byte s3[32];
	};
	struct CDecrypt
	{
		byte pin[32];
		byte t1[64];
	};
	struct CDecryptReply
	{
		byte t2[64];
	};
}