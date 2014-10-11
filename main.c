/*****************************************
 *  INCLUDES
 *****************************************/
#include "stdint.h"
#include "system_stm32f4xx.h"
// UB Libs
#include "ub_lib/stm32_ub_systick.h"
#include "ub_lib/stm32_ub_led.h"
#include "ub_lib/stm32_ub_dig_out.h"
#include "ub_lib/stm32_ub_dig_in.h"
#include "ub_lib/stm32_ub_rtc.h"
// Own Stuff
#include "DCF77.h"
#include "wc_frontend.h"


/*****************************************
 *  USED PINS
 *****************************************/
//PB7 - PowerOn Pin of DCF77 module
//PD3 - DataIn for DCF77 module

/*****************************************
 *  GLOBALS
 *****************************************/

volatile uint8_t gNewDcfSampleAvailable = 0;
volatile BitAction gDcfInputState = Bit_RESET;

/*****************************************
 *  MAIN
 *****************************************/
int main(void)
{
	SystemInit();

	// Init of UB libs
	UB_Systick_Init();
	UB_TIMER2_Init_FRQ( 100 );


	UB_Led_Init();
	UB_DigIn_Init();
	UB_DigOut_Init();

	UB_WS2812_Init();

	UB_Led_On( LED_GREEN );
	UB_Systick_Pause_ms( 100 );
	// Start timers and therefore cyclic actions in the call backs below
	UB_TIMER2_Start();

	UB_DigOut_Lo(DOUT_PB7);	// Set PB7 low to start DCF module

	WC_SetColor(WS2812_HSV_COL_WHITE);
	WC_SetElement(WC_ELEMENT_ES, 1);
	WC_Refresh();

	while(1) {


	}


}


/*****************************************
 *  Timer 2 Callback @100Hz
 *  - Used for: DCF77 sampling
 *****************************************/
void UB_TIMER2_ISR_CallBack( void )
{
	gDcfInputState = UB_DigIn_Read( DIN_PD3 );
	if ( gDcfInputState == Bit_SET )
		UB_Led_On( LED_ORANGE );
	else
		UB_Led_Off( LED_ORANGE );
		RTC_t newTime;
		dcf77_SignalState_t dcf77state = Dcf77_ProcessSignal( gDcfInputState );
		if ( dcf77state == dcf77_TimeRxSuccess )
			newTime = Dcf77_GetTime();
		else if ( dcf77state == dcf77_RxStateUnkown )
			UB_Led_On( LED_RED );
		else if ( dcf77state == dcf77_RxStateGood )
			UB_Led_Off( LED_RED );
}

// Other non used timer callbacks
void UB_TIMER3_ISR_CallBack( void )
{

}

void UB_TIMER4_ISR_CallBack( void )
{

}

void UB_TIMER5_ISR_CallBack( void )
{

}
