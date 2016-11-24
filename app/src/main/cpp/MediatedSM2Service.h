#pragma once
#include "MediatedSM2.h"

class CMediatedSM2Service : public CECCPublicKey, public IMediatedSM2Service
{
public:
	CMediatedSM2Service();
	CMediatedSM2Service(const unsigned char *pKey, int iLen)
	{
		SetKey(pKey, iLen);
	}
	virtual~CMediatedSM2Service();


	int SetKey(const unsigned char *pKey, int iLen);
	int OutputKey(unsigned char *pKey/* DCS_ECC_KEY_LENGTH * 8 */) const;
	void ClearKey();

	int GenerateKey(const CMpi &pin, const CSM2Point &p1, CSM2Point &p2);
	int Sign(const CMpi &pin, const CMpi &e, const CSM2Point &q1, CMpi &r, CMpi &s2, CMpi &s3);
	int Decrypt(const CMpi &pin, const CSM2Point &t1, CSM2Point &t2);
	int ModifyPIN(const CMpi &oldPIN, const CMpi &newPIN);

protected:
	CMpi m_d2;
	CMpi m_w2; // = m_d2^-1
	CSM2Point m_p1; // = [m_w1]G
	CSM2Point m_p2; // = [m_w2]G

	bool CheckPIN(const CMpi &pin, CMpi &d2, CMpi &w2);
};

