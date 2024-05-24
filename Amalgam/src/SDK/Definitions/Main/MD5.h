#pragma once

#define MD5_DIGEST_LENGTH 16
#define MD5_BIT_LENGTH ( MD5_DIGEST_LENGTH * sizeof(unsigned char) )

struct MD5Value_t
{
	unsigned char bits[MD5_DIGEST_LENGTH];

	void Zero();
	bool IsZero() const;

	bool operator==(const MD5Value_t& src) const;
	bool operator!=(const MD5Value_t& src) const;

};

typedef struct
{
	unsigned int	buf[4];
	unsigned int	bits[2];
	unsigned char	in[64];
} MD5Context_t;

void MD5Init(MD5Context_t* context);
void MD5Update(MD5Context_t* context, unsigned char const* buf, unsigned int len);
void MD5Final(unsigned char digest[MD5_DIGEST_LENGTH], MD5Context_t* context);
char* MD5_Print(unsigned char* digest, int hashlen);
void MD5_ProcessSingleBuffer(const void* p, int len, MD5Value_t& md5Result);
unsigned int MD5_PseudoRandom(unsigned int nSeed);
bool MD5_Compare(const MD5Value_t& data, const MD5Value_t& compare);

inline bool MD5Value_t::operator==(const MD5Value_t& src) const {
	return MD5_Compare(*this, src);
}

inline bool MD5Value_t::operator!=(const MD5Value_t& src) const {
	return !MD5_Compare(*this, src);
}