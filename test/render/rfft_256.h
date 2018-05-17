/*
 * rfft_256.h
 *
 *  Created on: 2015¦~7¤ë20¤é
 *      Author: ych
 */

#ifndef RFFT_256_H_
#define RFFT_256_H_
#include "ring_buffer.h"

#define nIter 48
typedef struct {
	long real;
	long imag;
} Complex32_t;
typedef long long Word64;
typedef long Word32;
typedef short int Word16;

typedef struct {
	Word16 mic1Buf[1024];
	Word16 mic2Buf[1024];
	Word16 buffer_Olpa[768];
	Word16 buffer_Olpa1[768];
	Word16 pca_flag[513];
	Word16 Gain[513];
	Word16 cntr;

	RingBuffer* ringBuffer1[512];
	RingBuffer* ringBuffer2[512];

	float _Complex DataCh1[512][nIter];
	float _Complex DataCh2[512][nIter];
	float _Complex IcaCh1[512][nIter]; //overall/deMixing
	float _Complex IcaCh2[512][nIter];
	float _Complex PcaCh1[512][nIter]; //pca_outputs
	float _Complex PcaCh2[512][nIter];
	float _Complex WCh1[512][2];
	float _Complex WCh2[512][2];
	float _Complex PCh1[512][2];
	float _Complex PCh2[512][2];
	float _Complex PWC1[512][2];
	float _Complex PWC2[512][2];
	float _Complex DCh1[512][2];
	float _Complex DCh2[512][2];
	float _Complex uCh1[512][2];
	float _Complex uCh2[512][2];
	float _Complex aCh1[512][2];
	float _Complex aCh2[512][2];

	Complex32_t ICAD1[512][nIter];
	Complex32_t ICAD2[512][nIter];
	Complex32_t ICAD3[512][nIter];
	Complex32_t ICAD4[512][nIter];

	float eivalues[513][2];
	float doas[513][2];
	Word64 noise[513];
	Word32 preSNR[513];
	sig_atomic_t isEmpty;

	pthread_mutex_t state_mutex;
	pthread_mutex_t filter_mutex;
	pthread_mutex_t queue_mutex;
} ica_state;

typedef struct {
	ica_state *ica_config;
	pthread_mutex_t mx;
} ARG;

void *pca_ica_thread_func(void *arg);
void ICA_SeparateInit(ica_state *ica_config);
void ICA_Separation(short *MIC1, short *MIC2, short *Sout, short *Sout1,
		ica_state *ica_config);
#endif /* RFFT_256_H_ */
