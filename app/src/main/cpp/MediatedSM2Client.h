#pragma once
#include "MediatedSM2.h"


class CMediatedSM2Client : public CECCPublicKey
{
public:
	CMediatedSM2Client(IMediatedSM2Service &service);
	~CMediatedSM2Client();

	int GenerateKey(const unsigned char *pbPIN/*DCS_ECC_KEY_LENGTH*/);
	int ModifyPIN(const unsigned char *oldpin/*DCS_ECC_KEY_LENGTH*/, const unsigned char *newpin/*DCS_ECC_KEY_LENGTH*/);

	int SetKey(const unsigned char *pKey, int iLen);
	int OutputKey(unsigned char *pKey/* DCS_ECC_KEY_LENGTH * 8 */) const;
	void ClearKey();

	int SignMessage(unsigned char *pOut/*HASH_256+DCS_ECC_KEY_LENGTH*2*/, const unsigned char *pMsg, int iLenOfMsg, const char *pUserName, int iLenOfUserName, const unsigned char *pbPIN/*DCS_ECC_KEY_LENGTH*/);
	int DecryptMessage(unsigned char *pbOut, const unsigned char *pbIn, int iLenOfIn, const unsigned char *pbPIN/*DCS_ECC_KEY_LENGTH*/);

	int EncryptMessage(unsigned char *pbOut, const unsigned char *pbIn, int iLenOfIn);
protected:
	int Sign(unsigned char *pOut, const unsigned char *pIn, int iLen, const unsigned char *pbPIN/*DCS_ECC_KEY_LENGTH*/);

	int Decrypt(const unsigned char *pbCipher1, int iLenOfCipher1, unsigned char *pbX2, unsigned char *pbY2, const unsigned char *pbPIN/*DCS_ECC_KEY_LENGTH*/);

protected:
	IMediatedSM2Service &m_service;
	CMpi m_d1;
	CMpi m_w1; // = m_d1^-1
	CSM2Point m_p1; // = [m_w1]G
	CSM2Point m_p2; // = [m_w2]G

	//	CMpi m_d2;
	CMpi m_d3;
	//	CMpi m_d4;
	CMpi m_d5;
	//	CMpi m_d6;
	CMpi m_d7;

	//	CMpi m_t2;
	CMpi m_t3;
	//	CMpi m_t4;
	CMpi m_t5;
	//	CMpi m_t6;
	CMpi m_t7;

	void GetHWD2(CMpi &x);
	void GetHWD4(CMpi &x);
	void GetHWD6(CMpi &x);
	void GetHWT2(CMpi &x);
	void GetHWT4(CMpi &x);
	void GetHWT6(CMpi &x);

	void MultyByD(CMpl &x);
	void MultyByW(CSM2Point &t1, CSM2Point &c1); // c1 * w ==> t1
public:
	IMediatedSM2Service &GetService() { return m_service; }
};

