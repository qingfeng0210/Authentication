#include <time.h>
#include <stdlib.h>
#include "MediatedSM2Service.h"
#include "Message.h"


CMediatedSM2Service::CMediatedSM2Service()
{
	ClearKey();
}


CMediatedSM2Service::~CMediatedSM2Service()
{
}

int CMediatedSM2Service::GenerateKey(const CMpi &pin, const CSM2Point &p1, CSM2Point &p2)
{
	m_p1 = p1;
	do
	{
		CMpiHelp::Random(m_d2);
		m_w2 = g_paramN.BinaryInverse(m_d2);

		CMpi pntPz;
		Multiply(m_pntPx, m_pntPy, pntPz, m_w2, m_p1.x, m_p1.y);
		CMpiHelp::SubG2Stand(m_pntPx, m_pntPy, pntPz);
	} while(m_pntPy == 0);

	MultiplyGByTable(m_p2.x, m_p2.y, m_w2);

	m_d2 -= pin;
	if (m_d2 < 0) m_d2 += g_paramN.m_oModulus;
	m_w2 -= pin;
	if (m_w2 < 0) m_w2 += g_paramN.m_oModulus;

	p2 = m_p2;
	return DCS_ECC_KEY_LENGTH * 2;
}

bool CMediatedSM2Service::CheckPIN(const CMpi &pin, CMpi &d2, CMpi &w2)
{
	CMpl lltmp = m_d2 + pin;
	lltmp %= g_paramN.m_oModulus;
	d2 = lltmp.l;
	lltmp = m_w2 + pin;
	lltmp %= g_paramN.m_oModulus;
	w2 = lltmp.l;

	lltmp = d2 * w2;
	lltmp %= g_paramN.m_oModulus;
	return lltmp.l == 1;
}
int CMediatedSM2Service::ModifyPIN(const CMpi &oldPIN, const CMpi &newPIN)
{
	CMpi d2, w2;
	if (!CheckPIN(oldPIN, d2, w2)) return pdrm::ERROR_PIN;

	m_d2 = d2 - newPIN;
	if (m_d2 < 0) m_d2 += g_paramN.m_oModulus;
	m_w2 = w2 - newPIN;
	if (m_w2 < 0) m_w2 += g_paramN.m_oModulus;
	return pdrm::SUCCUCESS;
}

int CMediatedSM2Service::SetKey(const unsigned char *pKey, int iLen)
{
	if (iLen < 8 * DCS_ECC_KEY_LENGTH)
		return 0;	// error

	int pos = 0;
	m_d2.Import(pKey, DCS_ECC_KEY_LENGTH);
	pos += DCS_ECC_KEY_LENGTH;

	m_w2.Import(pKey + pos, DCS_ECC_KEY_LENGTH);
	pos += DCS_ECC_KEY_LENGTH;

	m_p1.Import(pKey + pos, DCS_ECC_KEY_LENGTH * 2);
	pos += DCS_ECC_KEY_LENGTH * 2;

	m_p2.Import(pKey + pos, DCS_ECC_KEY_LENGTH * 2);
	pos += DCS_ECC_KEY_LENGTH * 2;

	return CECCPublicKey::SetPublicKey(pKey + pos, 2 * DCS_ECC_KEY_LENGTH);
}

int CMediatedSM2Service::OutputKey(unsigned char *pKey) const
{
	int pos = m_d2.Export(pKey, DCS_ECC_KEY_LENGTH);
	pos += m_w2.Export(pKey + pos, DCS_ECC_KEY_LENGTH);
	pos += m_p1.Export(pKey + pos);
	pos += m_p2.Export(pKey + pos);
	pos += CECCPublicKey::ExportPublicKey(pKey + pos);
	return pos;
}

void CMediatedSM2Service::ClearKey()
{
	m_d2 = m_w2 = m_p1.x = m_p1.y = m_p2.x = m_p2.y = g_paramN.m_oModulus;
	m_d2 = m_w2 = m_p1.x = m_p1.y = m_p2.x = m_p2.y = (unsigned int)0;
}

int CMediatedSM2Service::Sign(const CMpi &pin, const CMpi &e, const CSM2Point &q1, CMpi &r, CMpi &s2, CMpi &s3)
{
	CMpi d2, w2;
	if (!CheckPIN(pin, d2, w2)) return pdrm::ERROR_PIN;
	CMpi k2, k3;
	CMpl lltmp;
	do
	{
		CSM2Point tmpp, tmpp2;
		CMpiHelp::Random(k2);
		CMpiHelp::Random(k3);
		CMpi tmppz, tmpp2z;
		MultiplyGByTable(tmpp.x, tmpp.y, tmppz, k2);
		Multiply(tmpp2.x, tmpp2.y, tmpp2z, k3, q1.x, q1.y);
		AddMplJacobian(tmpp.x, tmpp.y, tmppz, tmpp2.x, tmpp2.y, tmpp2z);
		Jacobian2Stand(tmpp.x, tmpp.y, tmppz);
		lltmp = tmpp.x + e;
		lltmp %= g_paramN.m_oModulus;
	} while(lltmp.l == 0);
	
	r = lltmp.l;

	lltmp = d2 * k3;
	lltmp %= g_paramN.m_oModulus;
	s2 = lltmp.l;

	lltmp = r + k2;
	lltmp %= g_paramN.m_oModulus;
	lltmp = d2 * lltmp.l;
	lltmp %= g_paramN.m_oModulus;
	s3 = lltmp.l;

	return DCS_ECC_KEY_LENGTH * 3;
}
int CMediatedSM2Service::Decrypt(const CMpi &pin, const CSM2Point &t1, CSM2Point &t2)
{
	CMpi d2, w2;
	if (!CheckPIN(pin, d2, w2)) return pdrm::ERROR_PIN;
	Multiply(t2.x, t2.y, w2, t1.x, t1.y);
	return DCS_ECC_KEY_LENGTH * 2;
}
