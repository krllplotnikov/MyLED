#include "AddressStrip.h"
#include "WS2812B.h"
#include <stdint.h>
#include <string.h>

/*--------------------------------------------------------------------------------
                        Local Function Prototype
--------------------------------------------------------------------------------*/
//Run
static void RunColor(AddressStrip_st* strip);
static void RunEffect(AddressStrip_st* strip);
static void RunColorMusic(AddressStrip_st* strip);
static void RunStateOff(AddressStrip_st* strip);

//Preset
static void PresetRainbow(AddressStrip_st* strip, uint16_t start, uint16_t stop);
static void PresetHue(AddressStrip_st* strip, uint16_t start, uint16_t stop, uint16_t hue);
static void PresetRaindrop(AddressStrip_st* strip, uint16_t start, uint16_t stop);
static void PresetGradient(AddressStrip_st* strip, uint16_t startLed, uint16_t stopLed, uint16_t startHue, uint16_t stopHue);

//Animation
static void AnimationTrain(AddressStrip_st* strip, uint16_t start, uint16_t stop);
static void AnimationLoop(AddressStrip_st* strip, uint16_t start, uint16_t stop);

//Effect
static void EffectRainbowLoop(AddressStrip_st* strip);

//ColorMusic
static void ColorMusicSpectre(AddressStrip_st* strip);
static void ColorMusicRainbowTrain(AddressStrip_st* strip);

static void ColorMusicCalculateMagnitudes(AddressStrip_st* strip);



/*--------------------------------------------------------------------------------
                        Global Function Definition
--------------------------------------------------------------------------------*/
void AddressStrip_Init(AddressStrip_st* strip, WS2812B_st* ws2812b){
	strip->ws2812b = ws2812b;
	
	strip->numberLeds = ADDRESS_STRIP_DEFAULT_NUMDER_LEDS;
	strip->state = ADDRESS_STRIP_DEFAULT_STATE;
	strip->mode = ADDRESS_STRIP_DEFAULT_MODE;
	strip->brightness = ADDRESS_STRIP_DEFAULT_BRIGHTNESS;
	strip->hue = ADDRESS_STRIP_DEFAULT_HUE;
	strip->saturation = ADDRESS_STRIP_DEFAULT_SATURATION;
	strip->effect = ADDRESS_STRIP_DEFAULT_EFFECT;
	strip->effectSpeed = ADDRESS_STRIP_DEFAULT_EFFECT_SPEED;
	strip->colorMusic = ADDRESS_STRIP_DEFAULT_COLOR_MUSIC;
	strip->colorMusicSpeed = ADDRESS_STRIP_DEFAULT_COLOR_MUSIC_SPEED;
		
	strip->colorMusicSignalFlag = 1;
	
	strip->previousNumberLeds = 0;
	strip->previousMode = ADDRESS_STRIP_MODE_NONE;
	strip->previousEffect = ADDRESS_STRIP_EFFECT_NONE;
	strip->previousColorMusic = ADDRESS_STRIP_COLOR_MUSIC_NONE;
	
}

void AddressStrip_Run(AddressStrip_st* strip){
	if(strip->state == ADDRESS_STRIP_STATE_ON){
		if(strip->mode == ADDRESS_STRIP_MODE_COLOR){
			RunColor(strip);
		}	
		if(strip->mode == ADDRESS_STRIP_MODE_EFFECT){
			RunEffect(strip);
		}	
		if(strip->mode == ADDRESS_STRIP_MODE_COLOR_MUSIC){
			RunColorMusic(strip);
		}				
	}
	else if(strip->state == ADDRESS_STRIP_STATE_OFF){
		RunStateOff(strip);	
	}
	WS2812B_Run(strip->ws2812b);
}

void AddressStrip_SetNumberLeds(AddressStrip_st* strip, uint16_t numberLeds){
	strip->numberLeds = numberLeds;
	for(int i = 0; i < WS2812B_MAX_PIXELS; i++){
		WS2812B_SetColorHSV(strip->ws2812b, 0, 0, 0, i);
	}	
}

void AddressStrip_SetState(AddressStrip_st* strip, AddressStrip_State_et state){
	strip->previousState = strip->state;
	strip->state = state;
}

void AddressStrip_SetMode(AddressStrip_st* strip, uint8_t mode){
	strip->previousMode = strip->mode;
	strip->mode = mode;
}

void AddressStrip_SetBrightness(AddressStrip_st* strip, uint8_t brightness){
	strip->brightness = brightness;
}

void AddressStrip_SetHue(AddressStrip_st* strip, uint16_t hue){
	strip->hue = hue;
}

void AddressStrip_SetSaturation(AddressStrip_st* strip, uint8_t saturation){
	strip->saturation = saturation;
}

void AddressStrip_SetEffect(AddressStrip_st* strip, AddressStrip_Effect_et effect){
	strip->effect = effect;
}

void AddressStrip_SetEffectSpeed(AddressStrip_st* strip, uint8_t effectSpeed){
	strip->effectSpeed = effectSpeed;
}

void AddressStrip_SetColorMusic(AddressStrip_st* strip, AddressStrip_ColorMusic_et colorMusic){
	strip->colorMusic = colorMusic;
}

void AddressStrip_SetColorMusicSpeed(AddressStrip_st* strip, uint8_t colorMusicSpeed){
	strip->colorMusicSpeed = colorMusicSpeed;
}

uint8_t AddressStrip_IsColorMusicReady(AddressStrip_st* strip){
	return strip->colorMusicSignalFlag;
}

void AddressStrip_UpdateColorMusicSignal(AddressStrip_st* strip, int16_t* colorMusicSignal){
	if(strip->colorMusicSignalFlag){
		for (uint32_t i = 0; i < ADDRESS_STRIP_COLOR_MUSIC_SIGNAL_BUFFER_SIZE; i++) {
			strip->colorMusicSignal[i] = colorMusicSignal[i];
		}
		strip->colorMusicSignalFlag = 0;
	}
}



/*--------------------------------------------------------------------------------
                        Local Function Definition
--------------------------------------------------------------------------------*/
//Run
static void RunColor(AddressStrip_st* strip){
	for(int i = 0; i < strip->numberLeds; i++){
		WS2812B_SetColorHSV(strip->ws2812b, (float)strip->hue, (float)strip->saturation / 100, (float)strip->brightness / 100, i);
	}	
}

static void RunEffect(AddressStrip_st* strip){
	switch (strip->effect){
		case ADDRESS_STRIP_EFFECT_NONE: break;
		case ADDRESS_STRIP_EFFECT_RAINBOW_LOOP: EffectRainbowLoop(strip); break;
	}
}

static void RunColorMusic(AddressStrip_st* strip){
	switch (strip->colorMusic){
		case ADDRESS_STRIP_COLOR_MUSIC_NONE: break;
		case ADDRESS_STRIP_COLOR_MUSIC_SPECTRE: ColorMusicSpectre(strip); break;
		case ADDRESS_STRIP_COLOR_MUSIC_RAINBOW_TRAIN: ColorMusicRainbowTrain(strip); break;
	}	
}

static void RunStateOff(AddressStrip_st* strip){
	for(int i = 0; i < strip->numberLeds; i++){
		WS2812B_SetColorHSV(strip->ws2812b, 0, 0, 0, i);
	}	
}



//Preset
static void PresetRainbow(AddressStrip_st* strip, uint16_t start, uint16_t stop){
	if(stop > start){
		for(int i = start; i <= stop; i++){
			WS2812B_SetColorHSV(strip->ws2812b, WS2812B_Interpolation(0, stop - start, 0, 360, i), 1, (float)strip->brightness / 100, i);
		}		
	}
	else{
//TODO		
	}
}

static void PresetHue(AddressStrip_st* strip, uint16_t start, uint16_t stop, uint16_t hue){
	if(stop > start){
		for(int i = start; i <= stop; i++){
			WS2812B_SetColorHSV(strip->ws2812b, hue, 1, (float)strip->brightness / 100, i);
		}
	}
	else{
//TODO		
	}		
}

static void PresetRaindrop(AddressStrip_st* strip, uint16_t start, uint16_t stop){
	if(stop > start){
		for(int i = start; i <= stop; i++){
			float value = WS2812B_Interpolation(start, stop, 0.01, (float)strip->brightness / 100, i);
			WS2812B_SetValue(strip->ws2812b, value, i);
		}
	}
	else{
		for(int i = stop; i <= start; i++){
			float value = WS2812B_Interpolation(start, stop, 0.01, (float)strip->brightness / 100, i);
			WS2812B_SetValue(strip->ws2812b, value, i);
		}	
	}	
}

static void PresetGradient(AddressStrip_st* strip, uint16_t startLed, uint16_t stopLed, uint16_t startHue, uint16_t stopHue){
	if(stopLed > startLed){
		for(int led = startLed; led <= stopLed; led++){
			float hue = WS2812B_Interpolation(startLed, stopLed, startHue, stopHue, led);
			WS2812B_SetColorHSV(strip->ws2812b, hue, 1, (float)strip->brightness / 100, led);
		}
	}
	else{
//TODO		
	}
}



//Animation
static void AnimationLoop(AddressStrip_st* strip, uint16_t start, uint16_t stop){
	uint8_t r, g, b;
	uint8_t tr, tg, tb;
	
	if(stop > start){
		WS2812B_GetColorRGB(strip->ws2812b, &tr, &tg, &tb, stop);
		for(uint16_t i = stop; i > start; i--){
			WS2812B_GetColorRGB(strip->ws2812b, &r, &g, &b, i - 1);
			WS2812B_SetColorRGB(strip->ws2812b, r, g, b, i);
		}
		WS2812B_SetColorRGB(strip->ws2812b, tr, tg, tb, start);	
	}
	else{
		WS2812B_GetColorRGB(strip->ws2812b, &tr, &tg, &tb, stop);
		for(uint16_t i = stop; i < start; i++){
			WS2812B_GetColorRGB(strip->ws2812b, &r, &g, &b, i + 1);
			WS2812B_SetColorRGB(strip->ws2812b, r, g, b, i);
		}
		WS2812B_SetColorRGB(strip->ws2812b, tr, tg, tb, start);			
	}
}

static void AnimationTrain(AddressStrip_st* strip, uint16_t start, uint16_t stop){
	uint8_t r, g, b;
	if(stop > start){
		for(uint16_t i = stop; i > start; i--){
			WS2812B_GetColorRGB(strip->ws2812b, &r, &g, &b, i - 1);
			WS2812B_SetColorRGB(strip->ws2812b, r, g, b, i);
		}
		WS2812B_SetColorRGB(strip->ws2812b, 0, 0, 0, start);			
	}
	else{
		for(uint16_t i = stop; i < start; i++){
			WS2812B_GetColorRGB(strip->ws2812b, &r, &g, &b, i + 1);
			WS2812B_SetColorRGB(strip->ws2812b, r, g, b, i);
		}
		WS2812B_SetColorRGB(strip->ws2812b, 0, 0, 0, start);			
	}

}



//Effect
static void EffectRainbowLoop(AddressStrip_st* strip){
	static uint32_t tick = 0;
	
	if(strip->numberLeds != strip->previousNumberLeds || strip->effect != strip->previousEffect || strip->mode != strip->previousMode || strip->previousState != strip->state){
		PresetRainbow(strip, 0, strip->numberLeds - 1);
		strip->previousNumberLeds = strip->numberLeds;
		strip->previousEffect = strip->effect;
		strip->previousMode = strip->mode;
		strip->previousState = strip->state;
	}
	
	if(strip->ws2812b->userFunctions.getTick() - tick > strip->effectSpeed){
		AnimationLoop(strip, 0, strip->numberLeds - 1);
		
		tick = strip->ws2812b->userFunctions.getTick();
	}
}



//ColorMusic
static void ColorMusicSpectre(AddressStrip_st* strip){
	static uint32_t tick = 0;
	
	uint16_t segment1Start = 0;
	uint16_t segment1Stop  = strip->numberLeds / 6 - 1;
	
	uint16_t segment2Start = segment1Stop + 1;
	uint16_t segment2Stop  = segment2Start + segment1Stop;

	uint16_t segment3Start = segment2Stop + 1;
	uint16_t segment3Stop  = segment3Start + segment1Stop;

	uint16_t segment4Start = segment3Stop + 1;
	uint16_t segment4Stop  = segment4Start + segment1Stop;

	uint16_t segment5Start = segment4Stop + 1;
	uint16_t segment5Stop  = segment5Start + segment1Stop;

	uint16_t segment6Start = segment5Stop + 1;
	uint16_t segment6Stop  = strip->numberLeds - 1;
	
	if(strip->numberLeds != strip->previousNumberLeds || strip->colorMusic != strip->previousColorMusic){
		PresetGradient(strip, segment1Start, segment1Stop, 0, 60);
		PresetGradient(strip, segment2Start, segment2Stop, 120, 180);
		PresetGradient(strip, segment3Start, segment3Stop, 240, 350);
		PresetGradient(strip, segment4Start, segment4Stop, 350, 240);
		PresetGradient(strip, segment5Start, segment5Stop, 180, 120);
		PresetGradient(strip, segment6Start, segment6Stop, 60, 0);
		
		PresetRaindrop(strip, segment1Stop, segment1Start);
		PresetRaindrop(strip, segment6Start, segment6Stop);
		PresetRaindrop(strip, segment2Start, segment2Stop);
		PresetRaindrop(strip, segment5Stop, segment5Start);
		PresetRaindrop(strip, segment3Start, segment3Stop);
		PresetRaindrop(strip, segment4Stop, segment4Start);	
									
		strip->previousNumberLeds = strip->numberLeds;
		strip->previousColorMusic = strip->colorMusic;		
	}	
	
	if(strip->ws2812b->userFunctions.getTick() - tick > strip->colorMusicSpeed){
		AnimationTrain(strip, segment1Stop, segment1Start);
		AnimationTrain(strip, segment2Start, segment2Stop);
		AnimationTrain(strip, segment3Start, segment3Stop);
		AnimationTrain(strip, segment4Stop, segment4Start);
		AnimationTrain(strip, segment5Stop, segment5Start);
		AnimationTrain(strip, segment6Start, segment6Stop);	
			
		tick = strip->ws2812b->userFunctions.getTick();
	}
	
	if(!strip->colorMusicSignalFlag){
		ColorMusicCalculateMagnitudes(strip);
		
		if(strip->colorMusicMagnitude[1] > 7){
			PresetGradient(strip, segment1Start, segment1Stop, 0, 60);
			PresetGradient(strip, segment6Start, segment6Stop, 60, 0);
			
			PresetRaindrop(strip, segment1Stop, segment1Start);
			PresetRaindrop(strip, segment6Start, segment6Stop);
		}
		if(strip->colorMusicMagnitude[5] > 5){
			PresetGradient(strip, segment2Start, segment2Stop, 120, 180);
			PresetGradient(strip, segment5Start, segment5Stop, 180, 120);
			
			PresetRaindrop(strip, segment2Start, segment2Stop);
			PresetRaindrop(strip, segment5Stop, segment5Start);
		}
		if(strip->colorMusicMagnitude[10] > 3){	
			PresetGradient(strip, segment3Start, segment3Stop, 240, 350);
			PresetGradient(strip, segment4Start, segment4Stop, 350, 240);
			
			PresetRaindrop(strip, segment3Start, segment3Stop);
			PresetRaindrop(strip, segment4Stop, segment4Start);			
		}
		strip->colorMusicSignalFlag = 1;
	}	
}

static void ColorMusicRainbowTrain(AddressStrip_st* strip){
	static uint16_t hue = 0;
	static uint32_t tick = 0;
	
	if(strip->numberLeds != strip->previousNumberLeds || strip->colorMusic != strip->previousColorMusic){
		PresetHue(strip, 0, 5, hue);
		PresetRaindrop(strip, 0, 5);
		strip->previousNumberLeds = strip->numberLeds;
		strip->previousColorMusic = strip->colorMusic;
	}

	if(strip->ws2812b->userFunctions.getTick() - tick > strip->colorMusicSpeed){
		AnimationTrain(strip, 0, strip->numberLeds);
		tick = strip->ws2812b->userFunctions.getTick();
	}
	
	if(!strip->colorMusicSignalFlag){
		ColorMusicCalculateMagnitudes(strip);
		
		if(strip->colorMusicMagnitude[3] > 7){
			PresetHue(strip, 0, 5, hue);
			PresetRaindrop(strip, 0, 5);
			AnimationTrain(strip, 0, strip->numberLeds);
			hue += 6;
			if(hue > 360){
				hue = 0;
			}
		}
		strip->colorMusicSignalFlag = 1;
	}
}



static void ColorMusicCalculateMagnitudes(AddressStrip_st* strip){	
	FFT_st fft = {0};
	float idat[ADDRESS_STRIP_COLOR_MUSIC_SIGNAL_BUFFER_SIZE] = {0};
			
	FFT_Init(&fft, strip->colorMusicSignal, idat, ADDRESS_STRIP_COLOR_MUSIC_SIGNAL_BUFFER_SIZE);
	FFT_Calculate(&fft, FT_INVERSE);
	FFT_Magnitude(&fft, strip->colorMusicMagnitude);
}