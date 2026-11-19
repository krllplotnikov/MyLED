#ifndef PERIPH_INIT_H_
#define PERIPH_INIT_H_

#include "driver/rmt_tx.h"
#include "driver/i2s.h"

#include "nvs_flash.h"
#include "esp_wifi.h"

#define NUM_SAMPLES 256 // I2S DMA buffer length, samples
#define SAMPLE_RATE 16000

void RMT_Init();
void I2S_Init();

void NVS_Init();
void WiFi_Init();


void RMT_Transmit(void* payload, size_t payloadBytes);
/*
 * Read up to "samples" 16-bit mono samples into buffer (blocks until data is available).
 * Returns the number of samples read (0 on error).
 */
size_t I2S_Read(int16_t* buffer, size_t samples);

#endif
