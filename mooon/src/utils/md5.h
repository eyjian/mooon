/*
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * MD5Context structure, pass it to MD5Init, call MD5Update as
 * needed on buffers full of bytes, and then call MD5Final, which
 * will fill a supplied 16-byte array with the digest.
 */
#ifndef _H_MD5_H
#define _H_MD5_H
#include <stdint.h>

struct MD5Context {
	uint32_t buf[4];
	uint32_t bits[2];
	uint8_t  in[64];
};

extern void MD5Init(struct MD5Context *);
extern void MD5Update(struct MD5Context *, unsigned char const *, unsigned int);
extern void MD5Final(unsigned char digest[16], struct MD5Context *ctx);
extern void MD5Digest( const unsigned char *msg, int len, unsigned char *digest);
extern void MD5HMAC(const unsigned char *password,  unsigned pass_len,
		const unsigned char *challenge, unsigned chal_len,
		unsigned char response[16]);
extern void MD5HMAC2(const unsigned char *password,  unsigned pass_len,
		const unsigned char *challenge, unsigned chal_len,
		const unsigned char *challenge2, unsigned chal_len2,
		unsigned char response[16]);
#endif
