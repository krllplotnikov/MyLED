#include <stdint.h>
#include <math.h>

void WS2812B_HSV2RGB(float h, float s, float v, uint8_t* r, uint8_t* g, uint8_t* b);
void WS2812B_RGB2HSV(uint8_t r, uint8_t g, uint8_t b, float* h, float* s, float* v);
float WS2812B_Interpolation(float x1, float x2, float fx1, float fx2, float x);