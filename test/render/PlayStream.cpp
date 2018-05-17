/*
 * Copyright (c) 2006, Creative Labs Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided
 * that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and
 * 	     the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 * 	     and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of Creative Labs Inc. nor the names of its contributors may be used to endorse or
 * 	     promote products derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <pthread.h>
#include "Framework.h"
#include "CWaves.h"

extern "C" {
#include "rfft_256.h"
#include "ring_buffer.h"
}

typedef union {
	ALuint a;
	struct {
		ALuint low :16;
		ALuint high :16;
	};
} u32;

typedef struct {
	CWaves *pWaveLoader;
	RingBuffer *ringbuffer;
} ARGv;

typedef struct {
	RingBuffer *ringbuffer1;
	RingBuffer *ringbuffer2;
} ARGs;

#define NUMBUFFERS              (4)
#define	SERVICE_UPDATE_PERIOD	(32)

#define	TEST_WAVE_FILE		"Capture.wav"

pthread_mutex_t mxq1;
pthread_mutex_t mxq2;
pthread_mutex_t mxq3;

ica_state ica_type;

void *thread_funcs2(void *args) {
	RingBuffer *ringbuffer1 = reinterpret_cast<ARGs*>(args)->ringbuffer1;
	RingBuffer *ringbuffer2 = reinterpret_cast<ARGs*>(args)->ringbuffer2;
	u32 pData[256];
	ALushort bufferL[256];
	ALushort bufferR[256];

	while (1) {
		bool flag;
		u32 const *data_ptr;
		pthread_mutex_lock(&mxq1);
		flag = (WebRtc_available_read(ringbuffer1) >= 256);
		pthread_mutex_unlock(&mxq1);

		if (flag) {
			pthread_mutex_lock(&mxq1);
			WebRtc_ReadBuffer(ringbuffer1, (void**) &data_ptr, pData, 256);
			pthread_mutex_unlock(&mxq1);
			if (data_ptr != pData) {
				memmove(pData, data_ptr, 1024);
			}
			for (ALint i = 0; i < 256; i += 1) {
				bufferL[i] = pData[i].low;
				bufferR[i] = pData[i].high;
			}
			ICA_Separation((short*) bufferL, (short*) bufferR, (short*) bufferL,
					(short*) bufferR, &ica_type);
			for (ALint i = 0; i < 256; i += 1) {
				pData[i].low = bufferL[i];
				pData[i].high = bufferR[i];
			}
			pthread_mutex_lock(&mxq2);
			WebRtc_WriteBuffer(ringbuffer2, pData, 256);
			pthread_mutex_unlock(&mxq2);
		} else {
			Sleep(32);
		}
	}

	return (NULL);
}

void *thread_funcs1(void *args) {
	RingBuffer *ringbuffer = reinterpret_cast<ARGv*>(args)->ringbuffer;
	CWaves *pWaveLoader = reinterpret_cast<ARGv*>(args)->pWaveLoader;
	WAVEID WaveID;
	WAVEFORMATEX wfex;
	unsigned long ulDataSize = 0;
	unsigned long ulFrequency = 0;
	unsigned long ulFormat = 0;
	unsigned long ulBufferSize = 1024; //32ms=256samples
	unsigned long ulBytesWritten;
	void *pData = NULL;

	if ((pWaveLoader)
			&& (SUCCEEDED(
					pWaveLoader->OpenWaveFile(ALFWaddMediaPath(TEST_WAVE_FILE), &WaveID)))) {
		pWaveLoader->GetWaveSize(WaveID, &ulDataSize);
		pWaveLoader->GetWaveFrequency(WaveID, &ulFrequency);
		pWaveLoader->GetWaveALBufferFormat(WaveID, &alGetEnumValue, &ulFormat);
		pWaveLoader->GetWaveFormatExHeader(WaveID, &wfex);
		pData = malloc(ulBufferSize);
		if (pData) {
			pWaveLoader->SetWaveDataOffset(WaveID, 0);
			while (1) {
				pWaveLoader->ReadWaveData(WaveID, pData, ulBufferSize,
						&ulBytesWritten);
				if (ulBytesWritten) {
					pthread_mutex_lock(&mxq1);
					WebRtc_WriteBuffer(ringbuffer, pData, 256);
					pthread_mutex_unlock(&mxq1);
					Sleep(32);
				} else {
					break;
				}
			}
			free(pData);
			pData = NULL;
			pWaveLoader->DeleteWaveFile(WaveID);
		}
	}
	return (NULL);
}

int main() {
	ALuint uiBuffers[NUMBUFFERS];
	ALuint uiSource;
	ALuint uiBuffer;
	ALint iState;
	CWaves *pWaveLoader = NULL;
	WAVEID WaveID;
	ALint iLoop;
	ALint iBuffersProcessed, iTotalBuffersProcessed, iQueuedBuffers;
	WAVEFORMATEX wfex;
	unsigned long ulDataSize = 0;
	unsigned long ulFrequency = 0;
	unsigned long ulFormat = AL_FORMAT_STEREO16;
	unsigned long ulBufferSize;
	unsigned long ulBytesWritten;
	void *pData = NULL;
	pthread_t pid1, pid2, pid3;
	ALint err1, err2, err3;
	RingBuffer *ringbuffer1, *ringbuffer2;

	pWaveLoader = new CWaves;
	ringbuffer1 = WebRtc_CreateBuffer(1024, sizeof(u32));
	ringbuffer2 = WebRtc_CreateBuffer(1024, sizeof(u32));
	ARG arg = { .ica_config = &ica_type, .mx = &mxq3 };
	ARGv argv = { .pWaveLoader = pWaveLoader, .ringbuffer = ringbuffer1 };
	ARGs args = { .ringbuffer1 = ringbuffer1, .ringbuffer2 = ringbuffer2 };
	ICA_SeparateInit(&ica_type);
	pthread_mutex_init(&mxq1, NULL);
	pthread_mutex_init(&mxq2, NULL);
	pthread_mutex_init(&mxq3, NULL);

	err1 = pthread_create(&pid1, NULL, thread_funcs1, &argv);
	err2 = pthread_create(&pid2, NULL, thread_funcs2, &args);
	err3 = pthread_create(&pid3, NULL, pca_ica_thread_func, &arg);
	pthread_mutex_lock(&mxq3);
	assert(err1 == 0);
	assert(err2 == 0);
	assert(err3 == 0);

// Initialize Framework
	ALFWInit();

	ALFWprintf("PlayStream Test Application\n");

	if (!ALFWInitOpenAL()) {
		ALFWprintf("Failed to initialize OpenAL\n");
		ALFWShutdown();
		return 0;
	}

// Generate some AL Buffers for streaming
	alGenBuffers( NUMBUFFERS, uiBuffers);

// Generate a Source to playback the Buffers
	alGenSources(1, &uiSource);

	pData = malloc(1024);
	memset(pData, 0, 1024);

	for (iLoop = 0; iLoop < 4; iLoop++) {
		alBufferData(uiBuffers[iLoop], AL_FORMAT_STEREO16, pData, 1024, 8000);
		alSourceQueueBuffers(uiSource, 1, &uiBuffers[iLoop]);
	}

	alSourcePlay(uiSource);
	iTotalBuffersProcessed = 0;

	while (!ALFWKeyPress()) {
		Sleep( SERVICE_UPDATE_PERIOD);

		// Request the number of OpenAL Buffers have been processed (played) on the Source
		iBuffersProcessed = 0;
		alGetSourcei(uiSource, AL_BUFFERS_PROCESSED, &iBuffersProcessed);

		// Keep a running count of number of buffers processed (for logging purposes only)
		iTotalBuffersProcessed += iBuffersProcessed;
		ALFWprintf("Buffers Processed %d\r", iTotalBuffersProcessed);

		// For each processed buffer, remove it from the Source Queue, read next chunk of audio
		// data from disk, fill buffer with new data, and add it to the Source Queue
		while (iBuffersProcessed) {
			// Remove the Buffer from the Queue.  (uiBuffer contains the Buffer ID for the unqueued Buffer)
			while (1) {
				bool flag;
				u32 const *data_ptr;
				pthread_mutex_lock(&mxq2);
				flag = (WebRtc_available_read(ringbuffer2) >= 256);
				pthread_mutex_unlock(&mxq2);
				if (flag) {
					pthread_mutex_lock(&mxq2);
					WebRtc_ReadBuffer(ringbuffer2, (void**) &data_ptr, pData,
							256);
					pthread_mutex_unlock(&mxq2);
					if (data_ptr != pData)
						memmove(pData, data_ptr, 1024);
					break;
				} else {
					Sleep(32);
				}
			}

			uiBuffer = 0;
			alSourceUnqueueBuffers(uiSource, 1, &uiBuffer);

			// Read more audio data (if there is any)
			// Copy audio data to Buffer
			alBufferData(uiBuffer, ulFormat, pData, 1024, 8000);
			// Queue Buffer on the Source
			alSourceQueueBuffers(uiSource, 1, &uiBuffer);

			iBuffersProcessed--;
		}

		// Check the status of the Source.  If it is not playing, then playback was completed,
		// or the Source was starved of audio data, and needs to be restarted.
		alGetSourcei(uiSource, AL_SOURCE_STATE, &iState);
		if (iState != AL_PLAYING) {
			// If there are Buffers in the Source Queue then the Source was starved of audio
			// data, so needs to be restarted (because there is more audio data to play)
			alGetSourcei(uiSource, AL_BUFFERS_QUEUED, &iQueuedBuffers);
			if (iQueuedBuffers) {
				alSourcePlay(uiSource);
			} else {
				// Finished playing
				break;
			}
		}
	}

	alSourceStop(uiSource);
	alSourcei(uiSource, AL_BUFFER, 0);
	free(pData);
	pData = NULL;

#if 0
// Create instance of WaveLoader class
	pWaveLoader = new CWaves();
	if ((pWaveLoader)
			&& (SUCCEEDED(
							pWaveLoader->OpenWaveFile(ALFWaddMediaPath(TEST_WAVE_FILE), &WaveID)))) {
		pWaveLoader->GetWaveSize(WaveID, &ulDataSize);
		pWaveLoader->GetWaveFrequency(WaveID, &ulFrequency);
		pWaveLoader->GetWaveALBufferFormat(WaveID, &alGetEnumValue, &ulFormat);

		// Queue 250ms of audio data
		pWaveLoader->GetWaveFormatExHeader(WaveID, &wfex);
		//ulBufferSize = wfex.nAvgBytesPerSec >> 2;
		//ulBufferSize = wfex.nAvgBytesPerSec >> 3;
		ulBufferSize = 1024;

		// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
		ulBufferSize -= (ulBufferSize % wfex.nBlockAlign);

		if (ulFormat != 0) {
			pData = malloc(ulBufferSize);
			if (pData) {
				// Set read position to start of audio data
				pWaveLoader->SetWaveDataOffset(WaveID, 0);

				// Fill all the Buffers with audio data from the wavefile
				for (iLoop = 0; iLoop < 4; iLoop++) {
					if (SUCCEEDED(
									pWaveLoader->ReadWaveData(WaveID, pData,
											ulBufferSize, &ulBytesWritten))) {
						alBufferData(uiBuffers[iLoop], ulFormat, pData,
								ulBytesWritten, ulFrequency);
						alSourceQueueBuffers(uiSource, 1, &uiBuffers[iLoop]);
					}
				}

				// Start playing source
				alSourcePlay(uiSource);

				iTotalBuffersProcessed = 0;

				while (!ALFWKeyPress()) {
					Sleep( SERVICE_UPDATE_PERIOD);

					// Request the number of OpenAL Buffers have been processed (played) on the Source
					iBuffersProcessed = 0;
					alGetSourcei(uiSource, AL_BUFFERS_PROCESSED,
							&iBuffersProcessed);

					// Keep a running count of number of buffers processed (for logging purposes only)
					iTotalBuffersProcessed += iBuffersProcessed;
					ALFWprintf("Buffers Processed %d\r",
							iTotalBuffersProcessed);

					// For each processed buffer, remove it from the Source Queue, read next chunk of audio
					// data from disk, fill buffer with new data, and add it to the Source Queue
					while (iBuffersProcessed) {
						// Remove the Buffer from the Queue.  (uiBuffer contains the Buffer ID for the unqueued Buffer)
						uiBuffer = 0;
						alSourceUnqueueBuffers(uiSource, 1, &uiBuffer);

						// Read more audio data (if there is any)
						pWaveLoader->ReadWaveData(WaveID, pData, ulBufferSize,
								&ulBytesWritten);
						if (ulBytesWritten) {
							// Copy audio data to Buffer
							alBufferData(uiBuffer, ulFormat, pData,
									ulBytesWritten, ulFrequency);
							// Queue Buffer on the Source
							alSourceQueueBuffers(uiSource, 1, &uiBuffer);
						}

						iBuffersProcessed--;
					}

					// Check the status of the Source.  If it is not playing, then playback was completed,
					// or the Source was starved of audio data, and needs to be restarted.
					alGetSourcei(uiSource, AL_SOURCE_STATE, &iState);
					if (iState != AL_PLAYING) {
						// If there are Buffers in the Source Queue then the Source was starved of audio
						// data, so needs to be restarted (because there is more audio data to play)
						alGetSourcei(uiSource, AL_BUFFERS_QUEUED,
								&iQueuedBuffers);
						if (iQueuedBuffers) {
							alSourcePlay(uiSource);
						} else {
							// Finished playing
							break;
						}
					}
				}

				// Stop the Source and clear the Queue
				alSourceStop(uiSource);
				alSourcei(uiSource, AL_BUFFER, 0);

				// Release temporary storage
				free(pData);
				pData = NULL;
			} else {
				ALFWprintf("Out of memory\n");
			}
		} else {
			ALFWprintf("Unknown Audio Buffer format\n");
		}

		// Close Wave Handle
		pWaveLoader->DeleteWaveFile(WaveID);
	} else {
		ALFWprintf("Failed to load %s\n", ALFWaddMediaPath(TEST_WAVE_FILE));
	}
#endif
// Clean up buffers and sources
	alDeleteSources(1, &uiSource);
	alDeleteBuffers( NUMBUFFERS, uiBuffers);

	pthread_cancel(pid1);
	pthread_cancel(pid2);
	pthread_cancel(pid3);

	if (pWaveLoader)
		delete pWaveLoader;

	ALFWShutdownOpenAL();

	ALFWShutdown();

	return 0;
}
