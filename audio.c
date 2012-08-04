#define I_WANT_INTERNAL_AUDIO_STUFF
#include "audio.h"

/*

Differences from reference DFPWM implementation:
- Samples are unsigned.
- Samples are 16-bit.
- Antijerk filter follows the C implementation.
- Compression when equal and strengthless MIGHT be around the other way.

*/

u16 audio_rb[AUDIO_RB_SIZE][2];
int audio_rb_start = 0;
int audio_rb_end = 0;

//
// UNOPTIMISED IMPLEMENTATION
//
inline void dfpwm_update_model(int *q, int *s, int ri, int rd, int lt, int t)
{
	// calculate strength adjustments
	int st, sr;
	if(t == lt)
	{
		st = 0xFF;
		sr = ri;
	} else {
		st = 0x00;
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
	
	int lq = *q;
	dfpwm_update_model(q,s,ri,rd,*lt,t);
	
	// antijerk
	int ret = (t == *lt)
		? *q
		: ((*q+lq)>>1);
	
	// low pass filter
	*fq = (100 * (ret-*fq) + 0x80)>>8;
	ret = *fq;
	
	*lt = t;
	
	if(ret < 0x0000)
		ret = 0x0000;
	if(ret >= 0xFFFF)
		ret = 0xFFFF;
	
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

/*int audio_loadchunk(int len, s16 *buf)
{
	int ret = 0;
	
	// deal to ring buffer
	if(audio_rb_start+len > AUDIO_RB_SIZE)
	{
		int nlen = AUDIO_RB_SIZE - audio_rb_start;
		int nret = audio_loadchunk(nlen, buf);
		ret += nret;
		buf += nlen*2;
		len -= nlen;
	}
	
	// calculate remaining sample count
	int smpcount = (audio_rb_start > audio_rb_end)
		? audio_rb_end - audio_rb_start
		: audio_rb_end + AUDIO_RB_SIZE - audio_rb_start;
	
	if(((audio_rb_start+1)%AUDIO_RB_SIZE) == audio_rb_end)
	{
		return 0;
	} else {
		
	}
}

int audio_stealchunk(int len, s16 *buf)
{
	int ret = 0;
	
	// deal to ring buffer
	while(audio_rb_start+len > AUDIO_RB_SIZE)
	{
		int nlen = AUDIO_RB_SIZE - audio_rb_start;
		int nret = audio_stealchunk(nlen, buf);
		ret += nret;
		buf += nlen*2;
		len -= nlen;
	}
	
	if(audio_rb_start == audio_rb_end)
	{
		
	}
}*/


// TODO: refactor it to something similar to that which is above
FILE *autest = NULL;

int dfp_q = 0x8000;
int dfp_s = 0;
int dfp_fq = 0;
int dfp_lt = 0;
int dfp_ri = 7;
int dfp_rd = 20;
int audio_stealchunk(int len, s16 *buf)
{
	if(autest == NULL)
	{
		autest = fopen("autest.raw","rb");
		if(autest == NULL)
		{
			// XXX: tmpfile fails in Windows, I don't know why --GM
			autest = tmpfile();
			if(autest == NULL)
			{
				memset(buf, 0, len*4);
				return len;
			}
		}
	}
	
	int i,j;
	
	u8 tcmp;
	u16 traw[8];
	
	for(i = 0; i < len/8; i++)
	{
		// cmp/decmp
		for(j = 0; j < 8; j++)
		{
			traw[j] = 0;
			fread(&traw[j], 2, 1, autest);
			traw[j] ^= 0x8000;
		}
		
		dfpwm_compress(&dfp_q, &dfp_s, dfp_ri, dfp_rd, &dfp_lt, 1, traw, &tcmp);
		dfpwm_decompress(&dfp_q, &dfp_s, dfp_ri, dfp_rd, &dfp_lt, &dfp_fq, 1, traw, &tcmp);
		
		for(j = 0; j < 8; j++)
		{
			traw[j] ^= 0x8000;
			*(buf++) = traw[j];
			*(buf++) = traw[j];
		}
	}
	
	return len;
}

int sfp_audio_init()
{
	return sfp_audio_audio_init();
}
