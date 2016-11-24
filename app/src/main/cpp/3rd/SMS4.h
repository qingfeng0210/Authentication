// SMS4∂‘≥∆º”√‹À„∑®

#ifndef SMS4_HEADER_FDA90FJA09H___FDA98SFHA____FD98ASFH__
#define SMS4_HEADER_FDA90FJA09H___FDA98SFHA____FD98ASFH__

#define SMS4_KEY_LENGTH				(128/8)
#define SMS4_BLOCK_LENGTH			(128/8)
#define SMS4_ROUND					32

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void Sms4ExtendKey(unsigned int *subkey, const unsigned char *key);
void Sms4Encrypt(unsigned char *cipher, const unsigned char *plain, const unsigned int *subkey);
void Sms4Decrypt(unsigned char *plain, const unsigned char *cipher, const unsigned int *subkey);

unsigned int Sms4F(unsigned int w0, unsigned int w1, unsigned int w2, unsigned int w3, unsigned int rkey);
unsigned int Sms4FinExtendedKey(unsigned int w0, unsigned int w1, unsigned int w2, unsigned int w3, unsigned int ck);

#ifdef __cplusplus
}

#endif // __cplusplus


#endif // SMS4_HEADER_FDA90FJA09H___FDA98SFHA____FD98ASFH__
