/* rc6 (TM)
* Unoptimized sample implementation of Ron Rivest's submission to the
* AES bakeoff.
*
* Salvo Salasio, 19 June 1998
*
* Intellectual property notes:  The name of the algorithm (RC6) is
* trademarked; any property rights to the algorithm or the trademark
* should be discussed with discussed with the authors of the defining
* paper "The RC6(TM) Block Cipher": Ronald L. Rivest (MIT),
* M.J.B. Robshaw (RSA Labs), R. Sidney (RSA Labs), and Y.L. Yin (RSA Labs),
* distributed 18 June 1998 and available from the lead author's web site.
*
* This sample implementation is placed in the public domain by the author,
* Salvo Salasio.  The ROTL and ROTR definitions were cribbed from RSA Labs'
* RC5 reference implementation.
*/

#include <stdio.h>
#include<cstring>
#include"rc6_czq.h"
/* RC6 is parameterized for w-bit words, b bytes of key, and
* r rounds.  The AES version of RC6 specifies b=16, 24, or 32;
* w=32; and r=20.
*/

#define w 32	/* word size in bits */
#define r 20	/* based on security estimates */

#define P32 0xB7E15163	/* Magic constants for key setup */
#define Q32 0x9E3779B9

/* derived constants */
#define bytes   (w / 8)				/* bytes per word */
#define c       ((b + bytes - 1) / bytes)	/* key in words, rounded up */
#define R24     (2 * r + 4)
#define lgw     5                       	/* log2(w) -- wussed out */

/* Rotations */
#define ROTL(x,y) (((x)<<(y&(w-1))) | ((x)>>(w-(y&(w-1)))))
#define ROTR(x,y) (((x)>>(y&(w-1))) | ((x)<<(w-(y&(w-1)))))

unsigned int S[R24 - 1];		/* Key schedule */

void rc6_key_setup(unsigned char *K, int b)
{
	int i, j, s, v;
	unsigned int L[(32 + bytes - 1) / bytes]; /* Big enough for max b */
	unsigned int A, B;

	L[c - 1] = 0;
	for (i = b - 1; i >= 0; i--)
		L[i / bytes] = (L[i / bytes] << 8) + K[i];

	S[0] = P32;
	for (i = 1; i <= 2 * r + 3; i++)
		S[i] = S[i - 1] + Q32;

	A = B = i = j = 0;
	v = R24;
	if (c > v) v = c;
	v *= 3;

	for (s = 1; s <= v; s++)
	{
		A = S[i] = ROTL(S[i] + A + B, 3);
		B = L[j] = ROTL(L[j] + A + B, A + B);
		i = (i + 1) % R24;
		j = (j + 1) % c;
	}
}

void rc6_block_encrypt(unsigned int *pt, unsigned int *ct)
{
	unsigned int A, B, C, D, t, u, x;
	int i, j;

	A = pt[0];
	B = pt[1];
	C = pt[2];
	D = pt[3];
	B += S[0];
	D += S[1];
	for (i = 2; i <= 2 * r; i += 2)
	{
		t = ROTL(B * (2 * B + 1), lgw);
		u = ROTL(D * (2 * D + 1), lgw);
		A = ROTL(A ^ t, u) + S[i];
		C = ROTL(C ^ u, t) + S[i + 1];
		x = A;
		A = B;
		B = C;
		C = D;
		D = x;
	}
	A += S[2 * r + 2];
	C += S[2 * r + 3];
	ct[0] = A;
	ct[1] = B;
	ct[2] = C;
	ct[3] = D;
}

void rc6_block_decrypt(unsigned int *ct, unsigned int *pt)
{
	unsigned int A, B, C, D, t, u, x;
	int i, j;

	A = ct[0];
	B = ct[1];
	C = ct[2];
	D = ct[3];
	C -= S[2 * r + 3];
	A -= S[2 * r + 2];
	for (i = 2 * r; i >= 2; i -= 2)
	{
		x = D;
		D = C;
		C = B;
		B = A;
		A = x;
		u = ROTL(D * (2 * D + 1), lgw);
		t = ROTL(B * (2 * B + 1), lgw);
		C = ROTR(C - S[i + 1], t) ^ u;
		A = ROTR(A - S[i], u) ^ t;
	}
	D -= S[1];
	B -= S[0];
	pt[0] = A;
	pt[1] = B;
	pt[2] = C;
	pt[3] = D;
}

struct RC6_struct
{
	int keylen;
	unsigned char key[32];
	unsigned int pt[4];
	unsigned int ct[4];
};
void encryp_buffer(unsigned char *data_buffer, long long buffer_size, char * key)
{
	RC6_struct p;
	p.keylen = strlen(key) > w ? w : strlen(key);
	memcpy(p.key, key, p.keylen);
	int i;
	rc6_key_setup(p.key, p.keylen);

	if (NULL != data_buffer)
	{
		int once_enc_size = sizeof(unsigned int) * 4;
		int loop = buffer_size / (sizeof(unsigned int) * 4);
		for (i = 0; i < loop; ++i)
		{
			memcpy(p.pt, data_buffer + once_enc_size * i, once_enc_size);
			rc6_block_encrypt(p.pt, p.pt);
			memcpy(data_buffer + once_enc_size * i, p.pt, once_enc_size);
		}
	}
}

void decryp_buffer(unsigned char *data_buffer, long long buffer_size, char* key)
{

	RC6_struct p;
	p.keylen = strlen(key) > w ? w : strlen(key);
	memcpy(p.key, key, p.keylen);
	int i;
	rc6_key_setup(p.key, p.keylen);

	if (NULL != data_buffer)
	{
		int once_dec_size = sizeof(unsigned int) * 4;
		int loop = buffer_size / (sizeof(unsigned int) * 4);
		for (i = 0; i < loop; ++i)
		{
			memcpy(p.ct, data_buffer + once_dec_size * i, once_dec_size);
			rc6_block_decrypt(p.ct, p.ct);
			memcpy(data_buffer + once_dec_size * i, p.ct, once_dec_size);
		}
	}
}

