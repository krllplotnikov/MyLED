#include "WS2812B.h"
#include "WS2812B_Utils.h"

void WS2812B_Init(WS2812B_st* ws2812b, WS2812B_UserFunctions_st* userFunctions){
	ws2812b->userFunctions.sendData = userFunctions->sendData;
	ws2812b->userFunctions.getTick = userFunctions->getTick;
}

void WS2812B_Run(WS2812B_st* ws2812b){
	ws2812b->userFunctions.sendData(ws2812b, ws2812b->pixelColorRGB, WS2812B_MAX_PIXELS);
}



void WS2812B_SetColorRGB(WS2812B_st* ws2812b, uint8_t r, uint8_t g, uint8_t b,  uint16_t index){
	ws2812b->pixelColorRGB[index].r = r;
	ws2812b->pixelColorRGB[index].g = g;
	ws2812b->pixelColorRGB[index].b = b;
	
	WS2812B_RGB2HSV(r, g, b, &ws2812b->pixelColorHSV[index].h, &ws2812b->pixelColorHSV[index].s, &ws2812b->pixelColorHSV[index].v);
}

void WS2812B_GetColorRGB(WS2812B_st* ws2812b, uint8_t* r, uint8_t* g, uint8_t* b,  uint16_t index){
	*r = ws2812b->pixelColorRGB[index].r;
	*g = ws2812b->pixelColorRGB[index].g;
	*b = ws2812b->pixelColorRGB[index].b;
}

void WS2812B_SetRed(WS2812B_st* ws2812b, uint8_t r, uint16_t index){
	ws2812b->pixelColorRGB[index].r = r;
	
	WS2812B_RGB2HSV(ws2812b->pixelColorRGB[index].r, ws2812b->pixelColorRGB[index].g, ws2812b->pixelColorRGB[index].b, &ws2812b->pixelColorHSV[index].h, &ws2812b->pixelColorHSV[index].s, &ws2812b->pixelColorHSV[index].v);
}

void WS2812B_SetGreen(WS2812B_st* ws2812b, uint8_t g, uint16_t index){
	ws2812b->pixelColorRGB[index].g = g;
	
	WS2812B_RGB2HSV(ws2812b->pixelColorRGB[index].r, ws2812b->pixelColorRGB[index].g, ws2812b->pixelColorRGB[index].b, &ws2812b->pixelColorHSV[index].h, &ws2812b->pixelColorHSV[index].s, &ws2812b->pixelColorHSV[index].v);	
}

void WS2812B_SetBlue(WS2812B_st* ws2812b, uint8_t b, uint16_t index){
	ws2812b->pixelColorRGB[index].b = b;
	
	WS2812B_RGB2HSV(ws2812b->pixelColorRGB[index].r, ws2812b->pixelColorRGB[index].g, ws2812b->pixelColorRGB[index].b, &ws2812b->pixelColorHSV[index].h, &ws2812b->pixelColorHSV[index].s, &ws2812b->pixelColorHSV[index].v);	
}



void WS2812B_SetColorHSV(WS2812B_st* ws2812b, float h, float s, float v,  uint16_t index){
	ws2812b->pixelColorHSV[index].h = h;
	ws2812b->pixelColorHSV[index].s = s;
	ws2812b->pixelColorHSV[index].v = v;
		
	WS2812B_HSV2RGB(h, s, v, &ws2812b->pixelColorRGB[index].r, &ws2812b->pixelColorRGB[index].g, &ws2812b->pixelColorRGB[index].b);
}

void WS2812B_GetColorHSV(WS2812B_st* ws2812b, float* h, float* s, float* v,  uint16_t index){
	*h = ws2812b->pixelColorHSV[index].h;
	*s = ws2812b->pixelColorHSV[index].s;
	*v = ws2812b->pixelColorHSV[index].v;
}

void WS2812B_SetHue(WS2812B_st* ws2812b, float h, uint16_t index){
	ws2812b->pixelColorHSV[index].h = h;
	
	WS2812B_HSV2RGB(ws2812b->pixelColorHSV[index].h, ws2812b->pixelColorHSV[index].s, ws2812b->pixelColorHSV[index].v, &ws2812b->pixelColorRGB[index].r, &ws2812b->pixelColorRGB[index].g, &ws2812b->pixelColorRGB[index].b);
}

void WS2812B_SetSaturation(WS2812B_st* ws2812b, float s, uint16_t index){
	ws2812b->pixelColorHSV[index].s = s;
	
	WS2812B_HSV2RGB(ws2812b->pixelColorHSV[index].h, ws2812b->pixelColorHSV[index].s, ws2812b->pixelColorHSV[index].v, &ws2812b->pixelColorRGB[index].r, &ws2812b->pixelColorRGB[index].g, &ws2812b->pixelColorRGB[index].b);
}

void WS2812B_SetValue(WS2812B_st* ws2812b, float v, uint16_t index){
	ws2812b->pixelColorHSV[index].v = v;
	
	WS2812B_HSV2RGB(ws2812b->pixelColorHSV[index].h, ws2812b->pixelColorHSV[index].s, ws2812b->pixelColorHSV[index].v, &ws2812b->pixelColorRGB[index].r, &ws2812b->pixelColorRGB[index].g, &ws2812b->pixelColorRGB[index].b);	
}