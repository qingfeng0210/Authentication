#include <stdio.h>
#include <stdlib.h>

#define dbgprintf CLog::Debug
#include "MediatedSM2Stub.h"
#include "ALog.h"


CMediatedSM2Stub::CMediatedSM2Stub(CMediatedSM2StubParam &param) : _param(param)
{
	_loginSocket = INVALID_SOCKET;
}


CMediatedSM2Stub::~CMediatedSM2Stub()
{
	if (_loginSocket != INVALID_SOCKET) {
		closesocket(_loginSocket);
		_loginSocket = INVALID_SOCKET;
	}
}

int CMediatedSM2Stub::GetCode(byte *buf, int &length)
{
	pdrm::CGetCode req;
	if (_loginSocket == INVALID_SOCKET) {
		_loginSocket = create_and_connect(_param.serverAddress, _param.serverPort);
	}
	if (_loginSocket == INVALID_SOCKET) return pdrm::NETWORK_ERROR;

	int iret = SendAndRecv(pdrm::COD, &req, sizeof(req), buf, length);

	if (iret > 0 && length > 64) { // 获取验证码的时候同时获取公钥
		length -= 64;
		memcpy(_param.publicKey, buf + length, 64);
	}

	return iret;
}
int CMediatedSM2Stub::Register(const char *name, const char *password, const char *code)
{
	if (!name || !password || !code
		|| strlen(name) > 31 || strlen(code) > 7) return pdrm::PARAMETER_ERROR;
	byte tempkey[16];
	pdrm::CRegister req;
	memset(&req, 0, sizeof(req));
	strcpy(req.name, name);
	strcpy(req.code, code);
	CSecurity::Hash(password, strlen(password), req.password);
	CRealRandom::GetRandom(req.tempkey, 16);
	memset(req.tempkey, 0, 16);
	memcpy(tempkey, req.tempkey, 16);

	byte sreq[sizeof(pdrm::CRegister) + 32 * 3 + 1];
	byte rand[32];
	CECCPublicKey ecc;
	ecc.SetPublicKey(_param.publicKey, sizeof(_param.publicKey));

	int trycount = 5;
	while(trycount > 0) {
		CRealRandom::GetRandom(rand, sizeof(rand));
		if (ecc.EncryptMessage(sreq, (byte*)&req, sizeof(req), rand, sizeof(rand)) > 0) break;
		trycount--;
	}
	if (trycount == 0) return pdrm::PARAMETER_ERROR;

	pdrm::CRegisterReply reply;
	int len = sizeof(reply);
	int iret = SendAndRecv(pdrm::REG, sreq, sizeof(sreq), &reply, len);
	if (iret < 0) return iret;
	
	CSecurity::Decrypt(tempkey, reply);
	if (reply.time != _param.lastRequest) return pdrm::TIME_ERROR;
	_param.session = reply.session;
	memcpy(_param.sessionKey, reply.sessionKey, sizeof(_param.sessionKey));
	if (_loginSocket != INVALID_SOCKET) {
		closesocket(_loginSocket);
		_loginSocket = INVALID_SOCKET;
	}
	return len;
}
int CMediatedSM2Stub::Login(const char *name, const char *password, const char *code)
{
	if (!name || !password || !code
		|| strlen(name) > 31 || strlen(code) > 7) return pdrm::PARAMETER_ERROR;
	byte tempkey[16];
	pdrm::CLogin req;
	memset(&req, 0, sizeof(req));
	strcpy(req.name, name);
	strcpy(req.code, code);
	CSecurity::Hash(password, strlen(password), req.password);
	CRealRandom::GetRandom(req.tempkey, 16);
	memcpy(tempkey, req.tempkey, 16);

	byte sreq[sizeof(pdrm::CLogin) + 32 * 3 + 1];
	byte rand[32];
	CECCPublicKey ecc;
	ecc.SetPublicKey(_param.publicKey, sizeof(_param.publicKey));

	int trycount = 5;
	while (trycount > 0) {
		CRealRandom::GetRandom(rand, sizeof(rand));
		if (ecc.EncryptMessage(sreq, (byte*)&req, sizeof(req), rand, sizeof(rand)) > 0) break;
		trycount--;
	}
	if (trycount == 0) return pdrm::PARAMETER_ERROR;

	pdrm::CLoginReply reply;
	int len = sizeof(reply);
	int iret = SendAndRecv(pdrm::LGN, sreq, sizeof(sreq), &reply, len);
	if (iret < 0) return iret;

	CSecurity::Decrypt(tempkey, reply);
	if (reply.time != _param.lastRequest) return pdrm::TIME_ERROR;
	_param.session = reply.session;
	memcpy(_param.sessionKey, reply.sessionKey, sizeof(_param.sessionKey));
	if (_loginSocket != INVALID_SOCKET) {
		closesocket(_loginSocket);
		_loginSocket = INVALID_SOCKET;
	}
	return len;
}
int CMediatedSM2Stub::ModifyPIN(const CMpi &oldPIN, const CMpi &newPIN)
{
	pdrm::CModifyPIN req;
	oldPIN.Export(req.oldpin, 32);
	newPIN.Export(req.newpin, 32);
	int len = 0;
	int iret = SendAndRecv(pdrm::PIN, &req, sizeof(req), NULL, len);
	return iret;
}
int CMediatedSM2Stub::GenerateKey(const CMpi &pin, const CSM2Point &p1, CSM2Point &p2)
{
	pdrm::CGenerateKey req;
	pdrm::CGenerateKeyReply res;
	pin.Export(req.pin, 32);
	p1.Export(req.p1);
	int len = sizeof(res);
	int iret = SendAndRecv(pdrm::GEN, &req, sizeof(req), &res, len);
	if (iret < 0) return iret;
	p2.Import(res.p2, 64);
	return len;
}
int CMediatedSM2Stub::Sign(const CMpi &pin, const CMpi &e, const CSM2Point &q1, CMpi &r, CMpi &s2, CMpi &s3)
{
	pdrm::CSign req;
	pdrm::CSignReply reply;
	pin.Export(req.pin, 32);
	e.Export(req.e, 32);
	q1.Export(req.q1);

	int len = sizeof(reply);
	int iret = SendAndRecv(pdrm::SIG, &req, sizeof(req), &reply, len);
	if (iret < 0) return iret;
	r.Import(reply.r, 32);
	s2.Import(reply.s2, 32);
	s3.Import(reply.s3, 32);
	return len;
}
int CMediatedSM2Stub::Decrypt(const CMpi &pin, const CSM2Point &t1, CSM2Point &t2)
{
	pdrm::CDecrypt req;
	pdrm::CDecryptReply reply;
	pin.Export(req.pin, 32);
	t1.Export(req.t1);
	int len = sizeof(reply);
	int iret = SendAndRecv(pdrm::DEC, &req, sizeof(req), &reply, len);
	if (iret < 0) return iret;
	t2.Import(reply.t2, 64);
	return len;
}

SOCKET CMediatedSM2Stub::create_and_connect(const char *address, const char *port)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s;
	SOCKET sfd;

	LOGD("create_and_connect\n");

	memset(&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
	hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
	hints.ai_flags = AI_PASSIVE;     /* All interfaces */

	s = getaddrinfo(address, port, &hints, &result);
	if (s != 0)
	{
		LOGD("getaddrinfo: %s\n", gai_strerror(s));
		return INVALID_SOCKET;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == INVALID_SOCKET)
			continue;

		s = connect(sfd, rp->ai_addr, rp->ai_addrlen);
		if (s == 0)
		{
			/* We managed to bind successfully! */
			int flags = 1;
			if (setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flags, sizeof(int)) < 0)
			{
				LOGD("setsockopt TCP_NODELAY error\n");
			}
			break;
		}
		closesocket(sfd);
	}

	if (rp == NULL)
	{
		LOGD("Could not connect\n");
		return INVALID_SOCKET;
	}

	freeaddrinfo(result);

	return sfd;
}

bool CMediatedSM2Stub::sendonce(SOCKET s, const void *buf, int length)
{
	char *buffer = (char*)buf;
	LOGD("sendonce\n");
	while (length > 0)
	{
		int r = send(s, buffer, length, 0);
		if (r <= 0) return false;
		length -= r;
		buffer += r;
	}
	return true;
}

bool CMediatedSM2Stub::recvonce(SOCKET s, void *buf, int length)
{
	char *buffer = (char*)buf;
	while (length > 0)
	{
		int r = recv(s, buffer, length, 0);
		if (r <= 0) return false;
		length -= r;
		buffer += r;
	}
	return true;
}

int CMediatedSM2Stub::SendAndRecv(int pt, const void *req, int req_len, void *data, int &data_len)
{
	if (!req || req_len > MaxRequestSize || (data_len && !data)) return pdrm::PARAMETER_ERROR;
	int ret = pdrm::NETWORK_ERROR;
	int retry = 3;
	SOCKET s = _loginSocket;

	if (s == INVALID_SOCKET) s = create_and_connect(_param.serverAddress, _param.serverPort);
	while (s != INVALID_SOCKET)
	{
		pdrm::CMessageT<MaxRequestSize> request;
		request.session = _param.session;
		request.time = gettimems();
		if (request.time <= _param.lastRequest)
		{
			request.time = _param.lastRequest + 1;
		}
		_param.lastRequest = request.time;
		request.function = pt;
		request.length = req_len;
		memcpy(request.data(), req, req_len);
		CSM4 sessionKey;
		sessionKey.SetKey(_param.sessionKey);
		request.box(sessionKey);

		pdrm::CMessage h;

		if (sendonce(s, &request, request.totalLength()) // 发送请求
			&& recvonce(s, &h, sizeof(h)) // 接收响应头
			)
		{
			if (h.at != '@' || h.time != request.time || (_param.session != 0 && _param.session != h.session)) // 协议错误
			{
				ret = pdrm::PROTOCOL_ERROR;
				break;
			}
			LOGD("SendAndRecv: flags=%d length=%d\n", h.flags, h.length);
			if (data_len < h.length) // 接收响应的缓冲区太小
			{
				data_len = h.length;
				ret = pdrm::BUFFER_ERROR;
				break;
			}
			if (recvonce(s, data, h.length)) // 接收响应的内容
			{
				if (sessionKey.DecryptAuth(h.iv, data, h.length, h.signature, sizeof(h.signature)))
				{
					data_len = h.length;
					ret = h.reply;
				}
				else
				{
					ret = pdrm::MAC_ERROR;
				}
				break;
			}
		}
		if (s != INVALID_SOCKET) closesocket(s);
		if (retry--) // 重试
		{
			s = create_and_connect(_param.serverAddress, _param.serverPort);
		}
		else
		{
			s = INVALID_SOCKET;
		}
		if (_loginSocket != INVALID_SOCKET) {
			_loginSocket = s;
		}
	}
	if (_loginSocket != s) closesocket(s);
	return ret;
}
