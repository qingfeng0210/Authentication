// 椭圆曲线计算功能
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ELLIPTICCURVE_H__0372B568_C633_4807_B268_B59E2CDE28B5__INCLUDED_)
#define AFX_ELLIPTICCURVE_H__0372B568_C633_4807_B268_B59E2CDE28B5__INCLUDED_

#ifdef _MSC_VER
	#if _MSC_VER > 1000
		#pragma once
	#endif // _MSC_VER > 1000
#endif // _MSC_VER

#define DCS_ECC_KEY_LENGTH				32		// 256 = 8 * 32, in bytes, SM2算法标准, 256 bits

#ifndef NULL
	#define NULL    0
#endif


#include "sm3hash.h"
#define KDF				Sm3KDF
#define HASH_256		SM3_HASH_256
#define HASH_192		SM3_HASH_192
#define HASH_STATE		SM3_HASH_STATE
#define HashInit		Sm3HashInit
#define HashPending		Sm3HashPending
#define HashFinal		Sm3HashFinal

#include "Mpi.h"

// 对于point at infinity, 记为(0, 0)


// 基于素域的椭圆曲线
class CEllipticCurve  
{
public:
	int KeyExchangeRndMsg(unsigned char *pbRndMsg, const unsigned char *pbRnd, int iLenOfRnd);	// 密钥协商时发送的随机数消息

protected:
	CEllipticCurve();

public:
	static void Jacobian2Stand(CMpi &x, CMpi &y, CMpi &z);		// 坐标转化

	static bool CheckPoint(const CMpi &x, const CMpi &y);
	static void InitParam();		// 给参数赋值

	static const CMpi *GetN();

	static int ExportPoint(unsigned char *pbOut, const CMpi &x, const CMpi &y, bool fCompression = false);
	static int ImportPoint(const unsigned char *pbIn, int iInLen, CMpi &x, CMpi &y);

	static void MultiplyGByTable(CMpi &x, CMpi &y, const CMpi &m);
	static void MultiplyGByTable(CMpi &x, CMpi &y, CMpi &z, const CMpi &m);

	// (x, y) = m*(Gx, GY)
	static void Multiply(CMpi &x, CMpi &y, const CMpi &m, const CMpi &Gx, const CMpi &Gy);
	// 输入和输出都使用标准坐标
	static void Multiply(CMpi &x, CMpi &y, CMpi &z, const CMpi &m, const CMpi &Gx, const CMpi &Gy);
	// 输入使用标准坐标(强制z=1)，输出使用Jacobian坐标

	static void DoubleMplJacobian(CMpi &x, CMpi &y, CMpi &z);		// (x, y, z) = (x, y, z) + (x, y, z)
	static void DoubleMplJacobian(CMpi &x2, CMpi &y2, CMpi &z2, const CMpi &x, const CMpi &y, const CMpi &z);		// (x, y, z) = (x, y, z) + (x, y, z)
	// 计算之前，不做CheckPoint

	// (x, y, z) = (x, y, z) + (mx, my, mz)
	static void AddMplJacobian(CMpi &x, CMpi &y, CMpi &z, const CMpi &mx, const CMpi &my, const CMpi &mz);
	static void AddMplJacobian(CMpi &x, CMpi &y, CMpi &z, const CMpi &mx, const CMpi &my);		// only for mz == 1
};



class CECCPublicKey : public CEllipticCurve
{
public:
	CECCPublicKey();

	int SetPublicKey(const unsigned char *pKey, int iLen);
	int ExportPublicKey(unsigned char *pOut) const;

	int HashUserId(unsigned char *pbOut, const char *pUserName, int iLenOfName) const;
	//  签名时, 对签名者的id进行处理
	int MessageDigest(unsigned char *pbDigest, const unsigned char *pMsg, int iLenOfMsg, const char *pUserName, int iLenOfUserName);
	// 签名时, 产生真正计算的输入
	int AuthenticateMsg(unsigned char *pDigest, const unsigned char *pSecret, const unsigned char *pMsg, int iLenOfMsg);
	// 加密时, 计算校验值

	int EncryptMessage(unsigned char *pbOut, const unsigned char *pbIn, int iLenOfIn, const unsigned char *pRnd, int iLenOfRnd);
	// 使用SM2算法, 配合SM3 HASH算法。

	int VerifyMessage(const unsigned char *pMsg, int iLenOfMsg, const unsigned char *pSig, int iLenOfSig, const char *pUserName, int iLenOfUserName);

	const CMpi * GetParamPx() const { return &m_pntPx; };
	const CMpi * GetParamPy() const { return &m_pntPy; };

protected:
	int SetKey(const CMpi &paramPx, const CMpi &paramPy);		// 导入时, 不对Px/Py进行检查


	int Verify(const unsigned char *pDigest, int iLenOfDigest, const unsigned char *pSig, int iLenOfSig);
	int Encrypt(unsigned char *pbCipher1, unsigned char *pbX2, unsigned char *pbY2, const unsigned char *pRnd, int iLenOfRnd);
	// pbCipher1 = k*G
	// pbCipher2(x2, y2) = [k*inv(h)]*[h]*P

	CMpi m_pntPx;			// 公钥P点, P = D*G
	CMpi m_pntPy;
};

class CECCPrivateKey : public CECCPublicKey
{
public:
	CECCPrivateKey();

	int GenerateKey(const unsigned char *pRandomUser, // 调用者提供的随机数锟结供锟斤拷锟斤拷锟斤拷锟?
					int iLenOfRandom); // pRandomUser所指向的数据长度, in bytes


	int SetKey(const unsigned char *pKey, int iLen);
	int OutputKey(unsigned char *pKey) const;

	int SignMessage(unsigned char *pOut, const unsigned char *pMsg, int iLenOfMsg, const char *pUserName, int iLenOfUserName, const unsigned char *pRnd, int iLenOfRnd);
	int DecryptMessage(unsigned char *pbOut, const unsigned char *pbIn, int iLenOfIn);

	int KeyExchangeResult(unsigned char *pOut, int iLenOut,
							const unsigned char *pbMyRnd, int iLenOfRnd, const char *pMyUserName, int iLenOfMyUserName,
							const unsigned char *pbOtherRndMsg, int iLenOfRndMsg, const CECCPublicKey *pOtherPublicKey, const char *pOtherUserName, int iLenOfOtherUserName,
							bool fInit);

	const CMpi * GetParamD() const { return &m_paramD; };

protected:
	int SetKey(const CMpi &paramD, bool fComputePubKey = true);		// fComputePubKey是否要计算公钥点


	int Sign(unsigned char *pOut, const unsigned char *pIn, int iLen, const unsigned char *pRnd, int iLenOfRnd);
	
	int Decrypt(const unsigned char *pbCipher1, int iLenOfCipher1, unsigned char *pbX2, unsigned char *pbY2);

	CMpi m_paramD;				// 私钥
	CMpi m_inverseDplus1;			// 用于简化签名计算

};

extern CModulus g_paramFieldP;
extern CModulus g_paramN;


#endif // !defined(AFX_ELLIPTICCURVE_H__0372B568_C633_4807_B268_B59E2CDE28B5__INCLUDED_)
