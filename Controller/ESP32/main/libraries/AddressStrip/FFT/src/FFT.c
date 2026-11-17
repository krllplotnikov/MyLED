#include "FFT.h"

void FFT_Init(FFT_st* fft, float* rdat, float* idat, uint16_t size){
	fft->rdat = rdat;
	fft->idat = idat;
	fft->size = size;
}

void FFT_Calculate(FFT_st* fft, int8_t ftFlag){
	if ((fft->rdat == NULL) || (fft->idat == NULL))
		return;
	if ((fft->size > 16384) || (fft->size < 1))
		return;
	if (!NUMBER_IS_2_POW_K(fft->size))
		return;
	if ((ftFlag != FT_DIRECT) && (ftFlag != FT_INVERSE))
		return;

	uint8_t LogN;
	uint32_t i, j, n, k, io, ie, in, nn;
	float ru, iu, rtp, itp, rtq, itq, rw, iw, sr;

	static const float Rcoef[14] = { -1.0000000000000000F, 0.0000000000000000F,
			0.7071067811865475F, 0.9238795325112867F, 0.9807852804032304F,
			0.9951847266721969F, 0.9987954562051724F, 0.9996988186962042F,
			0.9999247018391445F, 0.9999811752826011F, 0.9999952938095761F,
			0.9999988234517018F, 0.9999997058628822F, 0.9999999264657178F };
	static const float Icoef[14] = { 0.0000000000000000F, -1.0000000000000000F,
			-0.7071067811865474F, -0.3826834323650897F, -0.1950903220161282F,
			-0.0980171403295606F, -0.0490676743274180F, -0.0245412285229122F,
			-0.0122715382857199F, -0.0061358846491544F, -0.0030679567629659F,
			-0.0015339801862847F, -0.0007669903187427F, -0.0003834951875714F };

	uint8_t arrLogN[14] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
	uint16_t arrN[14] = { 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096,
			8192, 16384 };

	for (uint8_t i = 0; i < 14; i++) {
		if (fft->size == arrN[i])
			LogN = arrLogN[i];
	}

	nn = fft->size >> 1;
	ie = fft->size;
	for (n = 1; n <= LogN; n++) {
		rw = Rcoef[LogN - n];
		iw = Icoef[LogN - n];
		if (ftFlag == FT_INVERSE)
			iw = -iw;
		in = ie >> 1;
		ru = 1.0F;
		iu = 0.0F;
		for (j = 0; j < in; j++) {
			for (i = j; i < fft->size; i += ie) {
				io = i + in;
				rtp = fft->rdat[i] + fft->rdat[io];
				itp = fft->idat[i] + fft->idat[io];
				rtq = fft->rdat[i] - fft->rdat[io];
				itq = fft->idat[i] - fft->idat[io];
				fft->rdat[io] = rtq * ru - itq * iu;
				fft->idat[io] = itq * ru + rtq * iu;
				fft->rdat[i] = rtp;
				fft->idat[i] = itp;
			}

			sr = ru;
			ru = ru * rw - iu * iw;
			iu = iu * rw + sr * iw;
		}

		ie >>= 1;
	}

	for (j = i = 1; i < fft->size; i++) {
		if (i < j) {
			io = i - 1;
			in = j - 1;
			rtp = fft->rdat[in];
			itp = fft->idat[in];
			fft->rdat[in] = fft->rdat[io];
			fft->idat[in] = fft->idat[io];
			fft->rdat[io] = rtp;
			fft->idat[io] = itp;
		}

		k = nn;

		while (k < j) {
			j = j - k;
			k >>= 1;
		}

		j = j + k;
	}

	if (ftFlag == FT_DIRECT)
		return;

	rw = 1.0F / fft->size;

	for (i = 0; i < fft->size; i++) {
		fft->rdat[i] *= rw;
		fft->idat[i] *= rw;
	}

	return;	
}

void FFT_Magnitude(FFT_st* fft, float* magnitudes){
	for (uint32_t i = 0; i < fft->size; i++) {
		magnitudes[i] = sqrtf(fft->rdat[i] * fft->rdat[i] + fft->idat[i] * fft->idat[i]);
	}			
}
