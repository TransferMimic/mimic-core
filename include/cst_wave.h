/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                        Copyright (c) 1999                             */
/*                        All Rights Reserved.                           */
/*                                                                       */
/*  Permission is hereby granted, free of charge, to use and distribute  */
/*  this software and its documentation without restriction, including   */
/*  without limitation the rights to use, copy, modify, merge, publish,  */
/*  distribute, sublicense, and/or sell copies of this work, and to      */
/*  permit persons to whom this work is furnished to do so, subject to   */
/*  the following conditions:                                            */
/*   1. The code must retain the above copyright notice, this list of    */
/*      conditions and the following disclaimer.                         */
/*   2. Any modifications must be clearly marked as such.                */
/*   3. Original authors' names are not deleted.                         */
/*   4. The authors' names are not used to endorse or promote products   */
/*      derived from this software without specific prior written        */
/*      permission.                                                      */
/*                                                                       */
/*  CARNEGIE MELLON UNIVERSITY AND THE CONTRIBUTORS TO THIS WORK         */
/*  DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING      */
/*  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   */
/*  SHALL CARNEGIE MELLON UNIVERSITY NOR THE CONTRIBUTORS BE LIABLE      */
/*  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES    */
/*  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN   */
/*  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,          */
/*  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF       */
/*  THIS SOFTWARE.                                                       */
/*                                                                       */
/*************************************************************************/
/*             Author:  Alan W Black (awb@cs.cmu.edu)                    */
/*               Date:  August 2000                                      */
/*************************************************************************/
/*                                                                       */
/*  Waveforms                                                            */
/*                                                                       */
/*************************************************************************/
#ifndef CST_WAVE_H
#define CST_WAVE_H

#include <stdint.h>
#include "cst_lib_visibility.h"
#include "cst_file.h"
#include "cst_error.h"
#include "cst_alloc.h"
#include "cst_val.h"

typedef struct cst_wave_struct {
    const char *type;
    int sample_rate;
    int num_samples;
    int num_channels;
    short *samples;
} cst_wave;

typedef struct cst_wave_header_struct {
    const char *type;
    int32_t hsize;
    int32_t num_bytes;
    int32_t sample_rate;
    int32_t num_samples;
    int32_t num_channels;
} cst_wave_header;

MIMIC_CORE_PUBLIC cst_wave *new_wave();
MIMIC_CORE_PUBLIC cst_wave *copy_wave(const cst_wave *w);
MIMIC_CORE_PUBLIC void delete_wave(cst_wave *val);
MIMIC_CORE_PUBLIC int concat_wave(cst_wave *dest, const cst_wave *src);

#define cst_wave_num_samples(w) (w?w->num_samples:0)
#define cst_wave_num_channels(w) (w?w->num_channels:0)
#define cst_wave_sample_rate(w) (w?w->sample_rate:0)
#define cst_wave_samples(w) (w->samples)

#define cst_wave_set_num_samples(w,s) w->num_samples=s
#define cst_wave_set_num_channels(w,s) w->num_channels=s
#define cst_wave_set_sample_rate(w,s) w->sample_rate=s

MIMIC_CORE_PUBLIC int cst_wave_save(cst_wave *w, const char *filename, const char *type);
MIMIC_CORE_PUBLIC int cst_wave_save_riff(cst_wave *w, const char *filename);
MIMIC_CORE_PUBLIC int cst_wave_save_raw(cst_wave *w, const char *filename);
int cst_wave_append_riff(cst_wave *w, const char *filename);

extern const char* cst_wave_fmt_riff;
extern const char* cst_wave_fmt_raw;

int cst_wave_save_riff_fd(cst_wave *w, cst_file fd);
int cst_wave_save_raw_fd(cst_wave *w, cst_file fd);

MIMIC_CORE_PUBLIC int cst_wave_load(cst_wave *w, const char *filename, const char *type);
MIMIC_CORE_PUBLIC int cst_wave_load_riff(cst_wave *w, const char *filename);
MIMIC_CORE_PUBLIC int cst_wave_load_raw(cst_wave *w, const char *filename,
                      const char *bo, int sample_rate);

int cst_wave_load_riff_header(cst_wave_header *header, cst_file fd);
int cst_wave_load_riff_fd(cst_wave *w, cst_file fd);
int cst_wave_load_raw_fd(cst_wave *w, cst_file fd,
                         const char *bo, int sample_rate);

MIMIC_CORE_PUBLIC int cst_wave_resize(cst_wave *w, int samples, int num_channels);
MIMIC_CORE_PUBLIC void cst_wave_resample(cst_wave *w, int sample_rate);
MIMIC_CORE_PUBLIC void cst_wave_rescale(cst_wave *w, int factor);

/* Resampling code */
typedef struct cst_rateconv_struct {
    int channels;               /* what do you think? */
    int up, down;               /* up/down sampling ratio */

    double gain;                /* output gain */
    int lag;                    /* lag time (in samples) */
    int *sin, *sout, *coep;     /* filter buffers, coefficients */

    /* n.b. outsize is the minimum buffer size for
       cst_rateconv_out() when streaming */
    int insize, outsize;        /* size of filter buffers */
    int incount;                /* amount of input data */
    int len;                    /* size of filter */

    /* internal foo coefficients */
    double fsin, fgk, fgg;
    /* internal counters */
    int inbaseidx, inoffset, cycctr, outidx;
} cst_rateconv;

cst_rateconv *new_rateconv(int up, int down, int channels);
void delete_rateconv(cst_rateconv *filt);
int cst_rateconv_in(cst_rateconv *filt, const short *inptr, int max);
int cst_rateconv_leadout(cst_rateconv *filt);
int cst_rateconv_out(cst_rateconv *filt, short *outptr, int max);

/* File format cruft. */

#define RIFF_FORMAT_PCM    0x0001
#define RIFF_FORMAT_ADPCM  0x0002
#define RIFF_FORMAT_MULAW  0x0006
#define RIFF_FORMAT_ALAW   0x0007

/* Sun/Next header, short and sweet, note its always BIG_ENDIAN though */
typedef struct {
    uint32_t magic;         /* magic number */
    uint32_t hdr_size;      /* size of this header */
    int32_t data_size;              /* length of data (optional) */
    uint32_t encoding;      /* data encoding format */
    uint32_t sample_rate;   /* samples per second */
    uint32_t channels;      /* number of interleaved channels */
} snd_header;

#define CST_SND_MAGIC (uint32_t)0x2e736e64
#define CST_SND_ULAW  1
#define CST_SND_UCHAR 2
#define CST_SND_SHORT 3

/* Convertion functions */
MIMIC_CORE_PUBLIC unsigned char cst_short_to_ulaw(int16_t sample);
MIMIC_CORE_PUBLIC int16_t cst_ulaw_to_short(unsigned char ulawbyte);

#define CST_G721_LEADIN 8
unsigned char *cst_g721_decode(int *actual_size, int size,
                               const unsigned char *packed_residual);
unsigned char *cst_g721_encode(int *packed_size, int actual_size,
                               const unsigned char *unpacked_residual);

CST_VAL_USER_TYPE_DCLS(wave, cst_wave);
#endif
