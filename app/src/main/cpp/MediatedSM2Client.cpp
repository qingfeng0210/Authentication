
#ifndef WIN32
#include <sys/system_properties.h>
#endif
#include "MediatedSM2Client.h"
#include "Message.h"
#include "DeviceInfo1.h"

CMediatedSM2Client::CMediatedSM2Client(IMediatedSM2Service &service) : m_service(service)
{
	ClearKey();
}


CMediatedSM2Client::~CMediatedSM2Client()
{
}

int CMediatedSM2Client::GenerateKey(const unsigned char *pbPIN)
{
	static CMpl st_temp;
	static CMpl st_temp2;

	CMpi pin;
	pin.Import(pbPIN, 32);
	if (!(pin < g_paramN.m_oModulus))
		pin -= g_paramN.m_oModulus;

	CMpiHelp::Random(m_d1);
	m_w1 = g_paramN.BinaryInverse(m_d1);

	MultiplyGByTable(m_p1.x, m_p1.y, m_w1);

	int iRetLen = m_service.GenerateKey(pin, m_p1, m_p2);
	if (iRetLen <= 0) return iRetLen;

	// ���㹫Կ d1^-1*m_p2-G
	CMpi pntPz;
	Multiply(m_pntPx, m_pntPy, pntPz, m_w1, m_p2.x, m_p2.y);
	CMpiHelp::SubG2Stand(m_pntPx, m_pntPy, pntPz);

	if (m_pntPy == 0)
		return -1;

	// 拆分
	CMpi y;
	GetHWD6(y);

	// 随机产生d5
	CMpiHelp::Random(m_d5);

	st_temp = m_d5*y;
	st_temp %= g_paramN.m_oModulus;

	CMpi x;
	GetHWD4(x);
	st_temp += x;
	st_temp %= g_paramN.m_oModulus;

	// 随机产生d3
	CMpiHelp::Random(m_d3);

	st_temp = st_temp.l*m_d3;
	st_temp %= g_paramN.m_oModulus;

	GetHWD2(x);
	st_temp2 = x * x;

	// 随机产生d7
	CMpiHelp::Random(m_d7);

	st_temp2 %= g_paramN.m_oModulus;
	st_temp2 += st_temp;
	st_temp2 += m_d7;
	st_temp2 %= g_paramN.m_oModulus;
	m_d1 -= st_temp2.l;
	if (m_d1.IsNegative()) m_d1 += g_paramN.m_oModulus;

	// 计算t
	GetHWT2(x);

	// 随机产生t3
	CMpiHelp::Random(m_t3);

	// 随机产生t5
	CMpiHelp::Random(m_t5);

	// 随机产生t7
	CMpiHelp::Random(m_t7);

	// 计算w1
	GetHWT6(y);
	st_temp = y*m_t7;
	st_temp %= g_paramN.m_oModulus;
	st_temp += m_t5;
	st_temp %= g_paramN.m_oModulus;
	GetHWT4(y);
	st_temp2 = y*y;
	st_temp2 %= g_paramN.m_oModulus;
	st_temp2 = st_temp2.l*st_temp.l;
	st_temp2 %= g_paramN.m_oModulus;
	m_w1 -= st_temp2.l;
	if (m_w1.IsNegative())
		m_w1 += g_paramN.m_oModulus;
	st_temp = x*m_t3;
	st_temp %= g_paramN.m_oModulus;
	m_w1 -= st_temp.l; // t1
	if (m_w1.IsNegative())
		m_w1 += g_paramN.m_oModulus;

	//使用w1保护d3和d7
	st_temp = m_d3 * m_w1;
	m_d3 = (st_temp %= g_paramN.m_oModulus);
	st_temp2 = m_w1 * m_d7;
	m_d7 = (st_temp2 %= g_paramN.m_oModulus);

	m_w1 = g_paramN.BinaryInverse(m_w1);


	return iRetLen;
}
inline void CMediatedSM2Client::MultyByW(CSM2Point &t1, CSM2Point &c1) // c1 * w ==> t1
{
	CMpl st_temp;
	CMpl st_temp2;
	CMpi z1(1), x2, y2, z2(1), z3(1);
	// 计算t
	GetHWT2(t1.x);
	GetHWT6(t1.y);
	st_temp = t1.y*m_t7;
	st_temp %= g_paramN.m_oModulus;
	st_temp += m_t5;
	st_temp %= g_paramN.m_oModulus;
	GetHWT4(t1.y);
	st_temp2 = t1.y*t1.y;
	st_temp2 %= g_paramN.m_oModulus;
	st_temp2 = st_temp2.l*st_temp.l;
	st_temp2 %= g_paramN.m_oModulus;

	st_temp = t1.x*m_t3;
	Multiply(t1.x, t1.y, z1, st_temp2.l, c1.x, c1.y);

	st_temp %= g_paramN.m_oModulus;
	Multiply(x2, y2, z2, st_temp.l, c1.x, c1.y);

	AddMplJacobian(t1.x, t1.y, z1, x2, y2, z2);

	z2 = g_paramN.BinaryInverse(m_w1);
	Multiply(x2, y2, z3, z2, c1.x, c1.y);

	AddMplJacobian(t1.x, t1.y, z1, x2, y2, z3);
	Jacobian2Stand(t1.x, t1.y, z1);
}
inline void CMediatedSM2Client::MultyByD(CMpl &s)
{
	CMpl mplTemp, mplTemp2;
	CMpi x, y;
	GetHWD6(y);

	mplTemp = m_d5*y;
	mplTemp %= g_paramN.m_oModulus;

	GetHWD4(x);
	mplTemp += x;
	mplTemp %= g_paramN.m_oModulus;

	mplTemp2 = m_d3 * m_w1;
	mplTemp2 %= g_paramN.m_oModulus;

	mplTemp = mplTemp.l*mplTemp2.l;
	mplTemp %= g_paramN.m_oModulus;

	GetHWD2(x);
	mplTemp2 = x * x;
	mplTemp2 %= g_paramN.m_oModulus;

	mplTemp2 += mplTemp;
	mplTemp2 %= g_paramN.m_oModulus;

	s %= g_paramN.m_oModulus;
	mplTemp = mplTemp2.l * s.l;
	mplTemp %= g_paramN.m_oModulus;

	mplTemp2 = m_d1 * s.l;
	mplTemp2 %= g_paramN.m_oModulus;
	mplTemp += mplTemp2.l;
	mplTemp %= g_paramN.m_oModulus;

	mplTemp2 = m_w1 * m_d7; 
	mplTemp2 %= g_paramN.m_oModulus;
	s = mplTemp2.l * s.l;
	s %= g_paramN.m_oModulus;

	s += mplTemp.l;
	s %= g_paramN.m_oModulus;
}
int CMediatedSM2Client::ModifyPIN(const unsigned char *oldpin, const unsigned char *newpin)
{
	CMpi pin1;
	pin1.Import(oldpin, 32);
	if (!(pin1 < g_paramN.m_oModulus))
		pin1 -= g_paramN.m_oModulus;

	CMpi pin2;
	pin2.Import(newpin, 32);
	if (!(pin2 < g_paramN.m_oModulus))
		pin2 -= g_paramN.m_oModulus;

	return m_service.ModifyPIN(pin1, pin2);
}

int CMediatedSM2Client::SetKey(const unsigned char *pKey, int iLen)
{
	if (iLen < (8+6) * DCS_ECC_KEY_LENGTH)
		return 0;	// error

	int pos = 0;
	m_d1.Import(pKey, DCS_ECC_KEY_LENGTH);
	pos += DCS_ECC_KEY_LENGTH;
	m_d3.Import(pKey + pos, DCS_ECC_KEY_LENGTH);
	pos += DCS_ECC_KEY_LENGTH;
	m_d5.Import(pKey + pos, DCS_ECC_KEY_LENGTH);
	pos += DCS_ECC_KEY_LENGTH;
	m_d7.Import(pKey + pos, DCS_ECC_KEY_LENGTH);
	pos += DCS_ECC_KEY_LENGTH;

	m_w1.Import(pKey + pos, DCS_ECC_KEY_LENGTH);
	pos += DCS_ECC_KEY_LENGTH;
	m_t3.Import(pKey + pos, DCS_ECC_KEY_LENGTH);
	pos += DCS_ECC_KEY_LENGTH;
	m_t5.Import(pKey + pos, DCS_ECC_KEY_LENGTH);
	pos += DCS_ECC_KEY_LENGTH;
	m_t7.Import(pKey + pos, DCS_ECC_KEY_LENGTH);
	pos += DCS_ECC_KEY_LENGTH;

	m_p1.Import(pKey + pos, DCS_ECC_KEY_LENGTH * 2);
	pos += DCS_ECC_KEY_LENGTH * 2;

	m_p2.Import(pKey + pos, DCS_ECC_KEY_LENGTH * 2);
	pos += DCS_ECC_KEY_LENGTH * 2;

	return CECCPublicKey::SetPublicKey(pKey + pos, 2 * DCS_ECC_KEY_LENGTH);
}
int CMediatedSM2Client::OutputKey(unsigned char *pKey) const
{
	int pos = m_d1.Export(pKey, DCS_ECC_KEY_LENGTH);
	pos += m_d3.Export(pKey + pos, DCS_ECC_KEY_LENGTH);
	pos += m_d5.Export(pKey + pos, DCS_ECC_KEY_LENGTH);
	pos += m_d7.Export(pKey + pos, DCS_ECC_KEY_LENGTH);
	pos += m_w1.Export(pKey + pos, DCS_ECC_KEY_LENGTH);
	pos += m_t3.Export(pKey + pos, DCS_ECC_KEY_LENGTH);
	pos += m_t5.Export(pKey + pos, DCS_ECC_KEY_LENGTH);
	pos += m_t7.Export(pKey + pos, DCS_ECC_KEY_LENGTH);
	pos += m_p1.Export(pKey + pos);
	pos += m_p2.Export(pKey + pos);
	pos += CECCPublicKey::ExportPublicKey(pKey + pos);
	return pos;
}
void CMediatedSM2Client::ClearKey()
{
	m_d1 = m_d3 = m_d5 = m_d7 = m_w1 = m_t3 = m_t5 = m_t7 = m_p1.x = m_p1.y = m_p2.x = m_p2.y = g_paramN.m_oModulus;
	m_d1 = m_d3 = m_d5 = m_d7 = m_w1 = m_t3 = m_t5 = m_t7 = m_p1.x = m_p1.y = m_p2.x = m_p2.y = (unsigned int)0;
}

int CMediatedSM2Client::SignMessage(unsigned char *pOut, const unsigned char *pMsg, int iLenOfMsg, const char *pUserName, int iLenOfUserName, const unsigned char *pbPIN/*DCS_ECC_KEY_LENGTH*/)
{
	unsigned char e[HASH_256];
	MessageDigest(e, pMsg, iLenOfMsg, pUserName, iLenOfUserName);

	return Sign(pOut, e, HASH_256, pbPIN);
}
int CMediatedSM2Client::EncryptMessage(unsigned char *pbOut, const unsigned char *pbIn, int iLenOfIn)
{
	int iRet = 0;
	while (iRet == 0)
	{
		byte rand[32];
		CRealRandom::GetRandom(rand, sizeof(rand));
		iRet = CECCPublicKey::EncryptMessage(pbOut, pbIn, iLenOfIn, rand, sizeof(rand));
	}
	return iRet;
}
int CMediatedSM2Client::DecryptMessage(unsigned char *pbOut, const unsigned char *pbIn, int iLenOfIn, const unsigned char *pbPIN/*DCS_ECC_KEY_LENGTH*/)
{
	int i;
	if (iLenOfIn <= (1+2*DCS_ECC_KEY_LENGTH+HASH_256))
		return pdrm::ERROR_PARAMETER;

	unsigned char secret[DCS_ECC_KEY_LENGTH*2];
	int iRet = Decrypt(pbIn, 2 * DCS_ECC_KEY_LENGTH + 1, secret, secret + DCS_ECC_KEY_LENGTH, pbPIN);
	if (iRet <= 0)	return iRet;	// error

	int iLenOfOut = iLenOfIn - (1+2*DCS_ECC_KEY_LENGTH+HASH_256);
	int iLenOfSecret = 2*DCS_ECC_KEY_LENGTH;

	KDF(pbOut, iLenOfOut, secret, iLenOfSecret);
	for (i = 0; i < iLenOfOut; i++)
		pbOut[i] ^= pbIn[1+iLenOfSecret+i];

	// C3
	unsigned char digest[HASH_256];
	AuthenticateMsg(digest, secret, pbOut, iLenOfOut);

	i = 0;
	while (i < HASH_256)
	{
		if (digest[i] != pbIn[iLenOfIn+i-HASH_256])
			return pdrm::MESSAGE_AUTH_ERROR;
		i++;
	}

	return iLenOfOut;
}

int CMediatedSM2Client::Sign(unsigned char *pOut, const unsigned char *pIn, int iLen, const unsigned char *pbPIN/*DCS_ECC_KEY_LENGTH*/)
{
	CMpi e, k1, r, s2, s3;
	CSM2Point q1;
	CMpl s;

	CMpi pin;
	pin.Import(pbPIN, 32);
	if (!(pin < g_paramN.m_oModulus))
		pin -= g_paramN.m_oModulus;


	if (!e.Import(pIn, iLen)) return 0;

	do 
	{
		/// ���������k1������q1 = [k1]G
		CMpiHelp::Random(k1);
		MultiplyGByTable(q1.x, q1.y, k1);
		int iRet = m_service.Sign(pin, e, q1, r, s2, s3);
		if (iRet <= 0) return iRet;

		s = k1 * s2;
		s %= g_paramN.m_oModulus;
		s += s3;
		//s = m_d1 * s.l;
		//s %= g_paramN.m_oModulus;
		MultyByD(s);

		s.l -= r;
		if (s.l.IsNegative())
			s.l += g_paramN.m_oModulus;
		s.h = g_paramN.m_oModulus - r;
	} while(s.l == 0 || s.h == s.l);


	r.Export(pOut, DCS_ECC_KEY_LENGTH);
	s.l.Export(pOut+DCS_ECC_KEY_LENGTH, DCS_ECC_KEY_LENGTH);
	return DCS_ECC_KEY_LENGTH * 2;

}
int CMediatedSM2Client::Decrypt(const unsigned char *pbCipher, int iLenOfCipher, unsigned char *pbX2, unsigned char *pbY2, const unsigned char *pbPIN/*DCS_ECC_KEY_LENGTH*/)
{
	CSM2Point c1, t1, t2;
	if (iLenOfCipher < 2*DCS_ECC_KEY_LENGTH+1 
		|| !ImportPoint(pbCipher, 2*DCS_ECC_KEY_LENGTH+1, c1.x, c1.y) || !CheckPoint(c1.x, c1.y))
		return 0;		// error

	CMpi pin;
	pin.Import(pbPIN, 32);
	if (!(pin < g_paramN.m_oModulus))
		pin -= g_paramN.m_oModulus;

	//Multiply(t1.x, t1.y, m_w1, c1.x, c1.y);
	MultyByW(t1, c1);

	int iRet = m_service.Decrypt(pin, t1, t2);
	if (iRet <= 0) return iRet;

	CMpi c1z, pntPz;
	CSM2Point tmpp = m_p2;
	Multiply(c1.x, c1.y, c1z, g_paramN.m_oModulus-1, c1.x, c1.y);
	AddMplJacobian(c1.x, c1.y, c1z, t2.x, t2.y);
	Jacobian2Stand(c1.x, c1.y, c1z);
	c1.x.Export(pbX2, DCS_ECC_KEY_LENGTH);
	c1.y.Export(pbY2, DCS_ECC_KEY_LENGTH);

	return DCS_ECC_KEY_LENGTH * 2;
}

inline void CMediatedSM2Client::GetHWD2(CMpi &x)
{
#ifdef WIN32
	static bool flag = false;
	static unsigned char data[DCS_ECC_KEY_LENGTH];

	if (!flag)
	{
		srand(2);
		for (int i = 0; i < DCS_ECC_KEY_LENGTH; i++)
			data[i] = rand();
		flag = true;
	}

	x.Import(data, DCS_ECC_KEY_LENGTH);
#else
	SM3_HASH_STATE hashState;
	char prop[128];
	int propLen = __system_property_get("ro.product.name", prop);
	Sm3HashInit(&hashState, (unsigned char *)prop, propLen);
	propLen = __system_property_get("ro.product.cpu.abi", prop);
	Sm3HashPending(&hashState, (unsigned char *)prop, propLen);
	propLen = __system_property_get("ro.product.manufacturer", prop);
	Sm3HashPending(&hashState, (unsigned char *)prop, propLen);
	propLen = __system_property_get("ro.product.model", prop);
	Sm3HashPending(&hashState, (unsigned char *)prop, propLen);
	propLen = __system_property_get("ro.product.device", prop);
	Sm3HashPending(&hashState, (unsigned char *)prop, propLen);
	propLen = __system_property_get("ro.product.board", prop);
	Sm3HashPending(&hashState, (unsigned char *)prop, propLen);

	unsigned char hashValue[SM3_HASH_256];
	Sm3HashFinal(hashValue, &hashState);

	x.Import(hashValue, SM3_HASH_256);
#endif
	if (!(x < g_paramN.m_oModulus))
		x -= g_paramN.m_oModulus;
}

inline void CMediatedSM2Client::GetHWD4(CMpi &x)
{
#ifdef WIN32
	static bool flag = false;
	static unsigned char data[DCS_ECC_KEY_LENGTH];

	if (!flag)
	{
		srand(3);
		for (int i = 0; i < DCS_ECC_KEY_LENGTH; i++)
			data[i] = rand();
		flag = true;
	}

	x.Import(data, DCS_ECC_KEY_LENGTH);
#else
	SM3_HASH_STATE hashState;
	char prop[128];
	int propLen = __system_property_get("ro.product.name", prop);
	Sm3HashInit(&hashState, (unsigned char *)prop, propLen);
	propLen = __system_property_get("ro.product.device", prop);
	Sm3HashPending(&hashState, (unsigned char *)prop, propLen);
	propLen = __system_property_get("ro.product.board", prop);
	Sm3HashPending(&hashState, (unsigned char *)prop, propLen);
	propLen = __system_property_get("ro.product.cpu.abi", prop);
	Sm3HashPending(&hashState, (unsigned char *)prop, propLen);
	propLen = __system_property_get("ro.product.manufacturer", prop);
	Sm3HashPending(&hashState, (unsigned char *)prop, propLen);
	propLen = __system_property_get("ro.product.model", prop);
	Sm3HashPending(&hashState, (unsigned char *)prop, propLen);

	unsigned char hashValue[SM3_HASH_256];
	Sm3HashFinal(hashValue, &hashState);

	x.Import(hashValue, SM3_HASH_256);
#endif

	if (!(x < g_paramN.m_oModulus))
		x -= g_paramN.m_oModulus;
}

inline void CMediatedSM2Client::GetHWD6(CMpi &x)
{
#ifdef WIN32
	static bool flag = false;
	static unsigned char data[DCS_ECC_KEY_LENGTH];

	if (!flag)
	{
		srand(4);
		for (int i = 0; i < DCS_ECC_KEY_LENGTH; i++)
			data[i] = rand();
		flag = true;
	}

	x.Import(data, DCS_ECC_KEY_LENGTH);
#else
	SM3_HASH_STATE hashState;
	char prop[128];
	int propLen = __system_property_get("ro.product.name", prop);
	Sm3HashInit(&hashState, (unsigned char *)prop, propLen);
	propLen = __system_property_get("ro.product.device", prop);
	Sm3HashPending(&hashState, (unsigned char *)prop, propLen);
	propLen = __system_property_get("ro.product.board", prop);
	Sm3HashPending(&hashState, (unsigned char *)prop, propLen);
	propLen = __system_property_get("ro.product.cpu.abi", prop);
	Sm3HashPending(&hashState, (unsigned char *)prop, propLen);

	unsigned char hashValue[SM3_HASH_256];
	Sm3HashFinal(hashValue, &hashState);

	x.Import(hashValue, SM3_HASH_256);
#endif
	if (!(x < g_paramN.m_oModulus))
		x -= g_paramN.m_oModulus;
}

inline void CMediatedSM2Client::GetHWT2(CMpi &x)
{
#ifdef WIN32
	static bool flag = false;
	static unsigned char data[DCS_ECC_KEY_LENGTH];

	if (!flag)
	{
		srand(5);
		for (int i = 0; i < DCS_ECC_KEY_LENGTH; i++)
			data[i] = rand();
		flag = true;
	}

	x.Import(data, DCS_ECC_KEY_LENGTH);
#else
	SM3_HASH_STATE hashState;
	char prop[128];
	int propLen = __system_property_get("ro.product.board", prop);
	Sm3HashInit(&hashState, (unsigned char *)prop, propLen);
	propLen = __system_property_get("ro.product.cpu.abi", prop);
	Sm3HashPending(&hashState, (unsigned char *)prop, propLen);
	propLen = __system_property_get("ro.product.manufacturer", prop);
	Sm3HashPending(&hashState, (unsigned char *)prop, propLen);
	propLen = __system_property_get("ro.product.model", prop);
	Sm3HashPending(&hashState, (unsigned char *)prop, propLen);

	unsigned char hashValue[SM3_HASH_256];
	Sm3HashFinal(hashValue, &hashState);

	x.Import(hashValue, SM3_HASH_256);
#endif

	if (!(x < g_paramN.m_oModulus))
		x -= g_paramN.m_oModulus;
}

inline void CMediatedSM2Client::GetHWT4(CMpi &x)
{
#ifdef WIN32
	static bool flag = false;
	static unsigned char data[DCS_ECC_KEY_LENGTH];

	if (!flag)
	{
		srand(6);
		for (int i = 0; i < DCS_ECC_KEY_LENGTH; i++)
			data[i] = rand();
		flag = true;
	}

	x.Import(data, DCS_ECC_KEY_LENGTH);
#else
	SM3_HASH_STATE hashState;
	char prop[128];
	int propLen = __system_property_get("ro.product.board", prop);
	Sm3HashInit(&hashState, (unsigned char *)prop, propLen);
	propLen = __system_property_get("ro.product.cpu.abi", prop);
	Sm3HashPending(&hashState, (unsigned char *)prop, propLen);
	propLen = __system_property_get("ro.product.manufacturer", prop);
	Sm3HashPending(&hashState, (unsigned char *)prop, propLen);
	propLen = __system_property_get("ro.product.model", prop);
	Sm3HashPending(&hashState, (unsigned char *)prop, propLen);

	int st;
	st = GetImeiImsiInfo(prop);

	if (st>0 && st<128)
	{
		Sm3HashPending(&hashState, (unsigned char *)prop, st);
	}

	unsigned char hashValue[SM3_HASH_256];
	Sm3HashFinal(hashValue, &hashState);

	x.Import(hashValue, SM3_HASH_256);
#endif

	if (!(x < g_paramN.m_oModulus))
		x -= g_paramN.m_oModulus;
}

inline void CMediatedSM2Client::GetHWT6(CMpi &x)
{
#ifdef WIN32
	static bool flag = false;
	static unsigned char data[DCS_ECC_KEY_LENGTH];

	if (!flag)
	{
		srand(7);
		for (int i = 0; i < DCS_ECC_KEY_LENGTH; i++)
			data[i] = rand();
		flag = true;
	}

	x.Import(data, DCS_ECC_KEY_LENGTH);
#else
	SM3_HASH_STATE hashState;
	char prop[128];
	int propLen = __system_property_get("ro.product.name", prop);
	Sm3HashInit(&hashState, (unsigned char *)prop, propLen);
	propLen = __system_property_get("ro.product.device", prop);
	Sm3HashPending(&hashState, (unsigned char *)prop, propLen);
	propLen = __system_property_get("ro.product.board", prop);
	Sm3HashPending(&hashState, (unsigned char *)prop, propLen);
	propLen = __system_property_get("ro.product.cpu.abi", prop);
	Sm3HashPending(&hashState, (unsigned char *)prop, propLen);

	int st;
	st = GetImeiImsiInfo(prop);

	if (st>0 && st<128)
	{
		Sm3HashPending(&hashState, (unsigned char *)prop, st);
	}

	unsigned char hashValue[SM3_HASH_256];
	Sm3HashFinal(hashValue, &hashState);

	x.Import(hashValue, SM3_HASH_256);
#endif

	if (!(x < g_paramN.m_oModulus))
		x -= g_paramN.m_oModulus;
}
