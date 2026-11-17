#ifndef INC_FFT_H_
#define INC_FFT_H_

#include <stdint.h>
#include <stdio.h>
#include <math.h>

#define  NUMBER_IS_2_POW_K(x)   ((!((x)&((x)-1)))&&((x)>1))  // x is pow(2, k), k=1,2, ...
#define  FT_DIRECT        -1    // Direct transform.
#define  FT_INVERSE        1    // Inverse transform.

typedef struct{
	float* rdat;
	float* idat;
	float* magnitudes;
	uint16_t size;
	int8_t ftFlag;
}FFT_st;

void FFT_Init(FFT_st* fft, float* rdat, float* idat, uint16_t size);
void FFT_Calculate(FFT_st* fft, int8_t ftFlag);
void FFT_Magnitude(FFT_st* fft, float* magnitudes);

#endif /* INC_FFT_H_ */
