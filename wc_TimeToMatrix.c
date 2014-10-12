#include "wc_TimeToMatrix.h"

// Function uses rtc time to set the wc_frontend
void SetWordMatrix( RTC_t time ){
	// Disable all old active LED elements
	WC_DisableAll();

	int hourToShow = time.std % 12;
	int minuteToshow = time.min;
	if ( minuteToshow >= 25 )
		++hourToShow;

	// Set minute LED elements
	// Set single LED dots
	if( minuteToshow % 5 != 0 )
		WC_SetElement( WC_ELEMENT_MIN_1, 1 );
	if( minuteToshow % 5 > 1 )
		WC_SetElement( WC_ELEMENT_MIN_2, 1 );
	if( minuteToshow % 5 > 2 )
		WC_SetElement( WC_ELEMENT_MIN_3, 1 );
	if( minuteToshow % 5 > 3 )
		WC_SetElement( WC_ELEMENT_MIN_4, 1 );

	// set FUENF
	if ( ( minuteToshow >= 5 && minuteToshow < 10 ) // fuenf nach voll
			|| ( minuteToshow >= 25 && minuteToshow < 30 )	// fuenf vor halb
			|| ( minuteToshow >= 35 && minuteToshow < 40 )	// fuenf nach halb
			|| ( minuteToshow >= 55 ) )	// fuenf vor voll
		WC_SetElement( WC_ELEMENT_FUENF_MINUTE, 1 );

	// set ZEHN
	if ( ( minuteToshow >= 10 && minuteToshow < 15 )	// zehn nach voll
			|| ( minuteToshow >= 50 && minuteToshow < 55 ) )	// zehn vor voll
		WC_SetElement( WC_ELEMENT_ZEHN_MINUTE, 1 );

	// set ZWANZIG
	if ( ( minuteToshow >= 20 && minuteToshow < 25 )	// zwanzig nach voll
			|| ( minuteToshow >= 40 && minuteToshow < 45 ) )	// zwanzig vor voll
		WC_SetElement( WC_ELEMENT_ZWANZIG, 1 );

	// set VIERTEL
	if ( ( minuteToshow >= 15 && minuteToshow < 20 )
			|| ( minuteToshow >= 45 && minuteToshow < 50 ) )
		WC_SetElement( WC_ELEMENT_VIERTEL, 1 );

	// set HALB
	if ( minuteToshow >= 25 && minuteToshow < 40 )
		WC_SetElement( WC_ELEMENT_HALB, 1 );

	// Set VOR/NACH and UHR
	if ( ( minuteToshow >= 5 && minuteToshow < 25 )	// Nach voll
			|| ( minuteToshow >= 35 && minuteToshow < 40 ) )	// Nach halb
		WC_SetElement( WC_ELEMENT_NACH, 1 );
	if ( ( minuteToshow >= 25 && minuteToshow < 30 )	// vor halb
			|| minuteToshow >= 40 )	// vor voll
		WC_SetElement( WC_ELEMENT_VOR, 1 );
	if ( minuteToshow < 5 )
		WC_SetElement( WC_ELEMENT_UHR, 1 );


	// Set hour LED elements
	switch ( hourToShow )
	{
	case 1:
		WC_SetElement( WC_ELEMENT_EINS_STUNDE, 1 );
		break;
	case 2:
		WC_SetElement( WC_ELEMENT_ZWEI_STUNDE, 1 );
		break;
	case 3:
		WC_SetElement( WC_ELEMENT_DREI_STUNDE, 1 );
		break;
	case 4:
		WC_SetElement( WC_ELEMENT_VIER_STUNDE, 1 );
		break;
	case 5:
		WC_SetElement( WC_ELEMENT_FUENF_STUNDE, 1 );
		break;
	case 6:
		WC_SetElement( WC_ELEMENT_SECHS_STUNDE, 1 );
		break;
	case 7:
		WC_SetElement( WC_ELEMENT_SIEBEN_STUNDE, 1 );
		break;
	case 8:
		WC_SetElement( WC_ELEMENT_ACHT_STUNDE, 1 );
		break;
	case 9:
		WC_SetElement( WC_ELEMENT_NEUN_STUNDE, 1 );
		break;
	case 10:
		WC_SetElement( WC_ELEMENT_ZEHN_STUNDE, 1 );
		break;
	case 11:
		WC_SetElement( WC_ELEMENT_ELF_STUNDE, 1 );
		break;
	case 12:
		WC_SetElement( WC_ELEMENT_ZWOELF_STUNDE, 1 );
		break;
	}
}
