#ifndef __WC_FRONTEND_H
#define __WC_FRONTEND_H

#include "stm32_ub_ws2812.h"

typedef enum {
	WC_ELEMENT_ES,
	WC_ELEMENT_IST,
	WC_ELEMENT_FUENF_MINUTE,
	WC_ELEMENT_ZEHN_MINUTE,
	WC_ELEMENT_ZWANZIG_MINUTE,
	WC_ELEMENT_DREI_MINUTE,
	WC_ELEMENT_VIERTEL_MINUTE,
	WC_ELEMENT_VOR,
	WC_ELEMENT_FUNK,
	WC_ELEMENT_NACH,
	WC_ELEMENT_HALB,
	WC_ELEMENT_EL_STUNDE,
	WC_ELEMENT_F_STUNDE,	// For elF and Fuenf
	WC_ELEMENT_UENF_STUNDE,
	WC_ELEMENT_EIN_STUNDE,
	WC_ELEMENT_S_STUNDE,	// For einS
	WC_ELEMENT_ZWEI_STUNDE,
	WC_ELEMENT_DREI_STUNDE,
	WC_ELEMENT_VIER_STUNDE,
	WC_ELEMENT_SECHS_STUNDE,
	WC_ELEMENT_ACHT_STUNDE,
	WC_ELEMENT_SIEBEN_STUNDE,
	WC_ELEMENT_ZWOELF_STUNDE,
	WC_ELEMENT_ZEH_STUNDE,
	WC_ELEMENT_N_STUNDE,	// For zehN and Neun
	WC_ELEMENT_EUN_STUNDE,
	WC_ELEMENT_UHR,
	WC_ELEMENT_MIN_upRight,
	WC_ELEMENT_MIN_downRight,
	WC_ELEMENT_MIN_downLeft,
	WC_ELEMENT_MIN_upLeft
} T_WC_ELEMENT;

#define T_WC_ELEMENT_COUNT 31

typedef struct {
	T_WC_ELEMENT ELEMENT_NAME;
	int START_INDEX;
	int LENGTH;
	int ENABLED;
} T_WC_CONFIGURATION;

extern WS2812_HSV_t WC_Colour; // colour, which should be used

void WC_SetElement(T_WC_ELEMENT element, int enable);
void WC_SetColor(WS2812_HSV_t colour);
WS2812_HSV_t WC_GetColor( void );
void WC_SetBrightness(int brightness);
void WC_Refresh();	// Not to be called from IRQs, as it waits itself for an IRQ
void WC_DisableAll();
void WC_DisableAllElements();


#endif
