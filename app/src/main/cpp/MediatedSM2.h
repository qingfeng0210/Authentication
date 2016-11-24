#pragma once
#include <stdio.h>
#include "3rd/Mpi.h"
#include "3rd/EllipticCurve.h"

struct CSM2Point
{
	CMpi x;
	CMpi y;

	int Export(BYTE *abOutBytes) const
	{
		if (!abOutBytes) return 0;
		int lengthInBytes = DCS_ECC_KEY_LENGTH;
		int iRetLen = x.Export(abOutBytes, lengthInBytes);
		iRetLen += y.Export(abOutBytes + lengthInBytes);
		return iRetLen;
	}

	int Import(const BYTE *abContent, int iLen)
	{
		int lengthInBytes = DCS_ECC_KEY_LENGTH;
		if (!abContent || iLen < lengthInBytes * 2)
			return 0;	// error

		x.Import(abContent, lengthInBytes);
		y.Import(abContent + lengthInBytes, lengthInBytes);
		return lengthInBytes * 2;
	}

	void Print() const
	{
		printf("x =");
		x.Print();
		printf("y =");
		y.Print();
	}
};

class CMpiHelp
{
public:
	static void Random(CMpi &x);
	static void SubG2Stand(CMpi &x, CMpi &y, CMpi &z);
};

class IMediatedSM2Service
{
public:
	virtual int GenerateKey(const CMpi &pin, const CSM2Point &p1, CSM2Point &p2) = 0;
	virtual int Sign(const CMpi &pin, const CMpi &e, const CSM2Point &q1, CMpi &r, CMpi &s2, CMpi &s3) = 0;
	virtual int Decrypt(const CMpi &pin, const CSM2Point &t1, CSM2Point &t2) = 0;

	virtual int ModifyPIN(const CMpi &oldPIN, const CMpi &newPIN) = 0;

	virtual ~IMediatedSM2Service(){}
};

class CRealRandom
{
public:
	static double CollectData(void *data, int len, double entropy=0);
	static bool GetRandom(void *out, int len);
};