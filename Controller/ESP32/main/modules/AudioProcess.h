#ifndef AUDIO_PROCESS_H_
#define AUDIO_PROCESS_H_

/*
 * Audio processing for AddressStrip color music:
 *   I2S samples -> DC removal -> Hann window -> FFT -> 8 octave bands (power) ->
 *   dB -> adaptive normalization (per band AGC) -> 8 floats 0..1
 *
 * The result is passed to AddressStrip_UpdateColorMusicBands().
 */

#include <stdint.h>
#include <stddef.h>

#define AUDIO_PROCESS_BANDS_COUNT        8      // must be equal to ADDRESS_STRIP_COLOR_MUSIC_BANDS
#define AUDIO_PROCESS_FFT_SIZE           512    // 16 kHz / 512 = 31.25 Hz per bin, 32 ms per frame (~31 frames/s)

#ifndef AUDIO_PROCESS_MIN_RANGE_DB
#define AUDIO_PROCESS_MIN_RANGE_DB       30.0f  // minimal distance floor..peak, so silence/noise is not stretched to 0..1
#endif
#ifndef AUDIO_PROCESS_PEAK_DECAY_DB
#define AUDIO_PROCESS_PEAK_DECAY_DB      0.1f   // peak falls by this per frame (~3 dB/s): adapts to a quieter song
#endif
#ifndef AUDIO_PROCESS_FLOOR_RISE_DB
#define AUDIO_PROCESS_FLOOR_RISE_DB      0.05f  // floor rises by this per frame when the band is above it
#endif
#ifndef AUDIO_PROCESS_FLOOR_FALL_DB
#define AUDIO_PROCESS_FLOOR_FALL_DB      0.5f   // floor falls by up to this per frame when the band is below it
                                                // (rise/(rise+fall) ~ 9% => floor sits near the 9th percentile)
#endif
#ifndef AUDIO_PROCESS_GATE_DB
#define AUDIO_PROCESS_GATE_DB            -90.0f // below this (dBFS) the band is treated as silence
#endif

/*
 * Initialize the module for the given sample rate (Hz).
 * Band edges are recalculated for the rate; the top edge is limited by sampleRate / 2.
 */
void AudioProcess_Init(float sampleRate);

/*
 * Add raw 16-bit PCM samples (any count). Samples are accumulated into an
 * AUDIO_PROCESS_FFT_SIZE block; every complete block is analyzed.
 * Returns 1 if at least one new frame of bands was computed (read it with AudioProcess_GetBands).
 */
uint8_t AudioProcess_AddSamples(const int16_t* samples, size_t count);

/*
 * Copy the last computed frame: AUDIO_PROCESS_BANDS_COUNT values 0..1.
 */
void AudioProcess_GetBands(float* bands);

/*
 * Analyze one full block: AUDIO_PROCESS_FFT_SIZE mono samples in -1..1.
 * bands - output, AUDIO_PROCESS_BANDS_COUNT values 0..1.
 */
void AudioProcess_ProcessBlock(const float* samples, float* bands);

/*
 * Reset the accumulated samples and the adaptive normalization
 * (e.g. when color music mode is turned on after a pause).
 */
void AudioProcess_Reset(void);

#endif
