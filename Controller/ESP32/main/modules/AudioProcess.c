#include "AudioProcess.h"
#include "FFT.h"

#include <math.h>
#include <string.h>

#define PI_F 3.14159265358979f

/*
 * Octave band edges, Hz. Band i = [edge[i], edge[i + 1]).
 *   0:   40-80    sub-bass (kick drum body)
 *   1:   80-160   bass
 *   2:  160-320   low-mid
 *   3:  320-640   mid
 *   4:  640-1280  mid
 *   5: 1280-2560  high-mid (vocals, snare)
 *   6: 2560-5120  presence
 *   7: 5120-8000  brilliance (hi-hats); limited by Nyquist = sampleRate / 2
 */
static const float bandEdgesHz[AUDIO_PROCESS_BANDS_COUNT + 1] = {40, 80, 160, 320, 640, 1280, 2560, 5120, 8000};

typedef struct{
	float sampleRate;
	uint16_t binStart[AUDIO_PROCESS_BANDS_COUNT];   // first FFT bin of the band (inclusive)
	uint16_t binStop[AUDIO_PROCESS_BANDS_COUNT];    // last FFT bin of the band (exclusive)
	float peakDb[AUDIO_PROCESS_BANDS_COUNT];
	float floorDb[AUDIO_PROCESS_BANDS_COUNT];
	uint8_t started;                                // 0 until the first frame initialized peak/floor

	float window[AUDIO_PROCESS_FFT_SIZE];
	float re[AUDIO_PROCESS_FFT_SIZE];
	float im[AUDIO_PROCESS_FFT_SIZE];

	float block[AUDIO_PROCESS_FFT_SIZE];            // accumulated input samples, -1..1
	uint16_t blockFill;

	float bands[AUDIO_PROCESS_BANDS_COUNT];         // last computed frame
}AudioProcess_st;

static AudioProcess_st audio;

void AudioProcess_Init(float sampleRate){
	const uint16_t n = AUDIO_PROCESS_FFT_SIZE;
	const uint16_t maxBin = n / 2;   // bins above n/2 mirror the lower ones for a real signal

	audio.sampleRate = sampleRate;

	for(uint16_t i = 0; i < n; i++){
		audio.window[i] = 0.5f * (1.0f - cosf(2.0f * PI_F * i / (n - 1)));   // Hann
	}

	uint16_t previousStop = 1;   // skip bin 0 (DC)
	for(uint8_t b = 0; b < AUDIO_PROCESS_BANDS_COUNT; b++){
		uint16_t start = (uint16_t)lroundf(bandEdgesHz[b] * n / sampleRate);
		uint16_t stop  = (uint16_t)lroundf(bandEdgesHz[b + 1] * n / sampleRate);

		if(start < previousStop) start = previousStop;   // bands must not overlap
		if(stop > maxBin) stop = maxBin;
		if(stop <= start) stop = start + 1;               // at least one bin per band
		if(stop > maxBin + 1) stop = maxBin + 1;

		audio.binStart[b] = start;
		audio.binStop[b] = stop;
		previousStop = stop;
	}

	AudioProcess_Reset();
}

void AudioProcess_Reset(void){
	audio.blockFill = 0;
	audio.started = 0;   // peak/floor are initialized from the next frame
	for(uint8_t b = 0; b < AUDIO_PROCESS_BANDS_COUNT; b++){
		audio.peakDb[b] = 0.0f;
		audio.floorDb[b] = 0.0f;
		audio.bands[b] = 0.0f;
	}
}

uint8_t AudioProcess_AddSamples(const int16_t* samples, size_t count){
	uint8_t ready = 0;
	if(samples == NULL){
		return 0;
	}

	for(size_t i = 0; i < count; i++){
		audio.block[audio.blockFill++] = (float)samples[i] / 32768.0f;
		if(audio.blockFill >= AUDIO_PROCESS_FFT_SIZE){
			AudioProcess_ProcessBlock(audio.block, audio.bands);
			audio.blockFill = 0;
			ready = 1;
		}
	}
	return ready;
}

void AudioProcess_GetBands(float* bands){
	memcpy(bands, audio.bands, sizeof(audio.bands));
}

void AudioProcess_ProcessBlock(const float* samples, float* bands){
	const uint16_t n = AUDIO_PROCESS_FFT_SIZE;

	//1. DC removal: MEMS microphones have a constant offset that would leak into the low bins
	float mean = 0;
	for(uint16_t i = 0; i < n; i++){
		mean += samples[i];
	}
	mean /= n;

	//2. Window: reduces spectral leakage (a loud bass would otherwise "light up" every band)
	for(uint16_t i = 0; i < n; i++){
		audio.re[i] = (samples[i] - mean) * audio.window[i];
		audio.im[i] = 0;
	}

	//3. Forward FFT (FT_DIRECT: no 1/N scaling)
	FFT_st fft;
	FFT_Init(&fft, audio.re, audio.im, n);
	FFT_Calculate(&fft, FT_DIRECT);

	//4. Band power. Scale so that a full-scale sine gives 0 dBFS:
	//   |X[k]| of a sine with amplitude A and Hann window = A * N / 4
	const float scale = 4.0f / n;

	for(uint8_t b = 0; b < AUDIO_PROCESS_BANDS_COUNT; b++){
		float power = 0;
		for(uint16_t k = audio.binStart[b]; k < audio.binStop[b]; k++){
			float re = audio.re[k] * scale;
			float im = audio.im[k] * scale;
			power += re * re + im * im;   // sum of power over the band: octave bands of pink noise come out equal
		}

		//5. Logarithmic scale: loudness perception is logarithmic
		float db = 10.0f * log10f(power + 1e-12f);

		//6. Adaptive normalization (AGC) per band: peak follows loud parts, floor follows quiet parts
		if(!audio.started){
			audio.peakDb[b] = db;
			audio.floorDb[b] = db;
		}
		audio.peakDb[b] -= AUDIO_PROCESS_PEAK_DECAY_DB;
		if(db > audio.peakDb[b]) audio.peakDb[b] = db;

		if(db < audio.floorDb[b]){
			float fall = audio.floorDb[b] - db;
			if(fall > AUDIO_PROCESS_FLOOR_FALL_DB) fall = AUDIO_PROCESS_FLOOR_FALL_DB;
			audio.floorDb[b] -= fall;
		}
		else{
			audio.floorDb[b] += AUDIO_PROCESS_FLOOR_RISE_DB;
		}

		float range = audio.peakDb[b] - audio.floorDb[b];
		if(range < AUDIO_PROCESS_MIN_RANGE_DB) range = AUDIO_PROCESS_MIN_RANGE_DB;

		float value = (db - audio.floorDb[b]) / range;
		if(db < AUDIO_PROCESS_GATE_DB) value = 0;
		if(value < 0) value = 0;
		if(value > 1) value = 1;

		bands[b] = value;
	}

	audio.started = 1;
}
