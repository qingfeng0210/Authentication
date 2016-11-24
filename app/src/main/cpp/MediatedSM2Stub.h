#pragma once
#include "MediatedSM2.h"
#include "Message.h"
#include <string>

struct CMediatedSM2StubParam
{
	char serverAddress[32];
	char serverPort[8];
	byte publicKey[64];
	__int64_t session;
	__int64_t lastRequest;
	byte sessionKey[16];
};

class CMediatedSM2Stub : public IMediatedSM2Service
{
public:
	CMediatedSM2Stub(CMediatedSM2StubParam &param);
	virtual~CMediatedSM2Stub();

	int GetCode(byte *buf, int &length);
	int Register(const char *name, const char *password, const char *code);
	int Login(const char *name, const char *password, const char *code);

	int GenerateKey(const CMpi &pin, const CSM2Point &p1, CSM2Point &p2);
	int Sign(const CMpi &pin, const CMpi &e, const CSM2Point &q1, CMpi &r, CMpi &s2, CMpi &s3);
	int Decrypt(const CMpi &pin, const CSM2Point &t1, CSM2Point &t2);
	int ModifyPIN(const CMpi &oldPIN, const CMpi &newPIN);
private:
	CMediatedSM2StubParam &_param;
	SOCKET _loginSocket;
	int SendAndRecv(int pt, const void *req, int req_len, void *data, int &data_len);
	static SOCKET create_and_connect(const char *address, const char *port);
	static bool sendonce(SOCKET s, const void *buf, int length);
	static bool recvonce(SOCKET s, void *buf, int length);
};

