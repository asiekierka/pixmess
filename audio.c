#define I_WANT_INTERNAL_AUDIO_STUFF
#include "audio.h"

/*

Differences from reference DFPWM implementation:
- Samples are unsigned.
- Samples are 16-bit.
- Antijerk filter follows the C implementation.
- Compression when equal and strengthless MIGHT be around the other way.

*/

//
// UNOPTIMISED IMPLEMENTATION
//
inline void dfpwm_update_model(int *q, int *s, int ri, int rd, int lt, int t)
{
	// calculate strength adjustments
	int st, sr;
	if(t == lt)
	{
		st = 0xFFFF;
		sr = ri;
	} else {
		st = 0x0000;
		sr = rd;
	}
	
	// adjust charge
	int nq = *q + ((*s * (t-*q) + 0x80)>>8);
	if(nq == *q && nq != t)
		nq += (t ? 1 : -1);
	*q = nq;
	
	// adjust strength
	int ns = *s + ((sr * (st-*s) + 0x80)>>8);
	if(ns == *s && ns != st)
		ns += (st ? 1 : -1);
	*s = ns;
}

inline int dfpwm_compress_bit(int *q, int *s, int ri, int rd, int *lt, int v)
{
	int t = (v < *q || v == 0)
		? 0x0000
		: 0xFFFF;
	
	dfpwm_update_model(q,s,ri,rd,*lt,t);
	*lt = t;
	
	return t;
}

inline int dfpwm_decompress_bit(int *q, int *s, int ri, int rd, int *lt, int *fq, int bit)
{
	int t = bit ? 0xFFFF : 0x0000;
	
	dfpwm_update_model(q,s,ri,rd,*lt,t);
	
	// antijerk
	int ret = (t == *lt)
		? t
		: ((t+*lt)>>1);
	
	// low pass filter
	*fq = (100 * (ret-*fq) + 0x80)>>8;
	ret = *fq;
	
	*lt = t;
	
	return ret;
}

// NOTE: len is in compressed bytes!
void dfpwm_compress(int *q, int *s, int ri, int rd, int *lt, int len, u16 *rawbuf, u8 *cmpbuf)
{
	int i,j;
	
	u8 d;
	
	for(i = 0; i < len; i++)
	{
		d  = (0x01 & dfpwm_compress_bit(q,s,ri,rd,lt,*(rawbuf++)));
		d |= (0x02 & dfpwm_compress_bit(q,s,ri,rd,lt,*(rawbuf++)));
		d |= (0x04 & dfpwm_compress_bit(q,s,ri,rd,lt,*(rawbuf++)));
		d |= (0x08 & dfpwm_compress_bit(q,s,ri,rd,lt,*(rawbuf++)));
		d |= (0x10 & dfpwm_compress_bit(q,s,ri,rd,lt,*(rawbuf++)));
		d |= (0x20 & dfpwm_compress_bit(q,s,ri,rd,lt,*(rawbuf++)));
		d |= (0x40 & dfpwm_compress_bit(q,s,ri,rd,lt,*(rawbuf++)));
		d |= (0x80 & dfpwm_compress_bit(q,s,ri,rd,lt,*(rawbuf++)));
		
		*(cmpbuf++) = d;
	}
}

void dfpwm_decompress(int *q, int *s, int ri, int rd, int *lt, int *fq, int len, u16 *rawbuf, u8 *cmpbuf)
{
	int i,j;
	
	u8 d;
	
	for(i = 0; i < len; i++)
	{
		d = *(cmpbuf++);
		
		*(rawbuf++) = dfpwm_decompress_bit(q,s,ri,rd,lt,fq,d&0x01);
		*(rawbuf++) = dfpwm_decompress_bit(q,s,ri,rd,lt,fq,d&0x02);
		*(rawbuf++) = dfpwm_decompress_bit(q,s,ri,rd,lt,fq,d&0x04);
		*(rawbuf++) = dfpwm_decompress_bit(q,s,ri,rd,lt,fq,d&0x08);
		*(rawbuf++) = dfpwm_decompress_bit(q,s,ri,rd,lt,fq,d&0x10);
		*(rawbuf++) = dfpwm_decompress_bit(q,s,ri,rd,lt,fq,d&0x20);
		*(rawbuf++) = dfpwm_decompress_bit(q,s,ri,rd,lt,fq,d&0x40);
		*(rawbuf++) = dfpwm_decompress_bit(q,s,ri,rd,lt,fq,d&0x80);
	}
	
}

int sfp_audio_init()
{
	return sfp_audio_audio_init();
}
