#include "MediatedSM2.h"

#include "3rd/sm3hash.h"
#include "def.h"
#include "security.h"
#include <time.h>
#include <stdlib.h>

class RealRandomState
{
    byte s[32][32];
	double entropy;
	int index;

public:
    RealRandomState()
    {
		index = 0;
		entropy = 0;
        CSecurity::GetRandom(s, sizeof(s));
    }
	double CollectData(void *data, int len, double entropy)
	{
		SM3_HASH_STATE sm3;
		Sm3HashInit(&sm3, (byte*)data, len);
		Sm3HashPending(&sm3, s[index], 32 * (32 - index));
		Sm3HashPending(&sm3, s[0], 32 * index);
		Sm3HashFinal(s[index], &sm3);
		index = (index + 1) % 32;
		entropy *= len;
		if (entropy > 32) entropy = 32;
		this->entropy += entropy;
		if (this->entropy > 32 * 32 * 8) this->entropy = 32 * 32 * 8;
		return this->entropy;
	}
	bool GetRandom(void *out, int len)
	{
#ifndef WIN32
		struct timeval tv;
		gettimeofday(&tv, NULL);
		CollectData(&tv, sizeof(tv), 0);
#endif
		byte *d = (byte*)out;
		CSecurity::GetRandom(out, len);

		while (len > 0)
		{
		    int remain = len > 32 ? 32 : len;
    		int i = index;
			int j;
			CollectData(d, remain, 0);
			for (j = 0; j < remain; j++) {
				d[j] ^= s[i][j];
			}
			d += j;
			len -= j;
			entropy -= j * 8;
		}
        if (entropy < 0) {
            entropy = 0;
            return false;
        } else {
            return true;
        }
	}
};
static RealRandomState g_randomState;

double CRealRandom::CollectData(void *data, int len, double entropy)
{
	return g_randomState.CollectData(data, len, entropy);
}

bool CRealRandom::GetRandom(void *out, int len)
{
	return g_randomState.GetRandom(out, len);
}

void CMpiHelp::Random(CMpi &x)
{
	unsigned char pRnd[DCS_ECC_KEY_LENGTH];
	while(true)
	{
		CRealRandom::GetRandom(pRnd, sizeof(pRnd));
		if (pRnd[0] == 0) continue;
		x.Import(pRnd, DCS_ECC_KEY_LENGTH);
		if (x > g_paramN.m_oModulus)
			x -= g_paramN.m_oModulus;

		if ((x != 0) && (x != g_paramN.m_oModulus))
		{
			return;
		}
	}
}
void CMpiHelp::SubG2Stand(CMpi &x, CMpi &y, CMpi &z)
{
	static bool init = false;
	static CMpi gx, gy, gz;
	if (!init)
	{
		CMpi tx, ty, tz;
		CEllipticCurve::MultiplyGByTable(tx, ty, tz, g_paramN.m_oModulus-1);
		gx = tx;
		gy = ty;
		gz = tz;
		init = true;
	}
	CEllipticCurve::AddMplJacobian(x, y, z, gx, gy, gz);
	CEllipticCurve::Jacobian2Stand(x, y, z);
}
