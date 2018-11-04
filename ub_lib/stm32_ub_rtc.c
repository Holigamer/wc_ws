//--------------------------------------------------------------
// File     : stm32_ub_rtc.c
// Datum    : 28.09.2013
// Version  : 1.1
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.0
// Module   : RTC, PWR (event. EXTI, MISC)
// Funktion : RealTimeClock der CPU
//
// Hinweis  : Ein externer Quarz (LSE = 32,768kHz) muss
//            an PC14 und PC15 angeschlossen werden
//            (und zwei Kondensatoren mit 6,8pF an GND)
//
//            Wenn die RTC auch ohne Spannungsversorung
//            weiterlaufen soll, muss eine externe Batterie
//            an VBAT (Pin6) angeschlossen werden.
//            Am Discovery-Modul muss dazu der 0 Ohm (R26)
//            entfernt werden !
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_rtc.h"



//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
void P_RTC_Config(void);
void P_RTC_WAKEUP_ISR(void);


//--------------------------------------------------------------
// GLobale Variabeln
//--------------------------------------------------------------
RTC_TimeTypeDef   RTC_TimeStructure; // muss Global sein
RTC_DateTypeDef   RTC_DateStructure; // muss Global sein
RTC_InitTypeDef   RTC_InitStructure; // muss Global sein


//--------------------------------------------------------------
// Init und Start der RTC
// Return Wert :
//  -> RTC_UNDEFINED = RTC war noch nicht initialisiert
//  -> RTC_INIT_OK   = RTC war schon initialisiert
//                     (aber noch nicht eingestellt)
//  -> RTC_TIME_OK   = RTC war schon eingestellt
//--------------------------------------------------------------
RTC_STATUS_t UB_RTC_Init(void)
{
  RTC_STATUS_t ret_wert=RTC_UNDEFINED;
  uint32_t status;

  // Clock enable fuer PWR
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  // Zugriff auf RTC freigeben
  PWR_BackupAccessCmd(ENABLE);

  // Test vom Status-Register
  status=RTC_ReadBackupRegister(RTC_STATUS_REG);
  if(status==RTC_STATUS_TIME_OK) {
    // wenn Uhrzeit schon eingestellt ist
    ret_wert=RTC_TIME_OK;
    UB_RTC.status=RTC_TIME_OK;

    // warte bis synconisiert
    RTC_WaitForSynchro();
    #if RTC_USE_WAKEUP_ISR==1
      RTC_ClearITPendingBit(RTC_IT_WUT);
      EXTI_ClearITPendingBit(EXTI_Line22);
    #endif

    // RTC auslesen
    UB_RTC_GetClock(RTC_DEC);
  }
  else if(status==RTC_STATUS_INIT_OK) {
    // wenn RTC schon initialisiert ist
    ret_wert=RTC_INIT_OK;
    UB_RTC.status=RTC_INIT_OK;

    // warte bis synconisiert
    RTC_WaitForSynchro();
    #if RTC_USE_WAKEUP_ISR==1
      RTC_ClearITPendingBit(RTC_IT_WUT);
      EXTI_ClearITPendingBit(EXTI_Line22);
    #endif

    // RTC auslesen
    UB_RTC_GetClock(RTC_DEC);
  }
  else {
    // wenn RTC noch nicht initialisiert ist
    ret_wert=RTC_UNDEFINED;
    UB_RTC.status=RTC_UNDEFINED;

    // RTC Konfig und start
    P_RTC_Config();

    // RTC zuruecksetzen auf (0:00:00 / 1.1.00)
    UB_RTC.std=0;
    UB_RTC.min=0;
    UB_RTC.sek=0;
    UB_RTC.tag=1;
    UB_RTC.monat=1;
    UB_RTC.jahr=0;
    UB_RTC.wotag=1;
    UB_RTC_SetClock(RTC_DEC);

    UB_RTC.status=RTC_INIT_OK;
  }
  
  return(ret_wert);
}


//--------------------------------------------------------------
// RTC setzen
// vor dem Aufruf muss die aktuelle Uhrzeit und das Datum
// in der Struktur "UB_RTC" gespeichert werden
// format : [RTC_DEC, RTC_HEX]
//--------------------------------------------------------------
void UB_RTC_SetClock(RTC_FORMAT_t format)
{
  // Check auf Min+Max
  if(UB_RTC.std>23) UB_RTC.std=23;
  if(UB_RTC.min>59) UB_RTC.min=59;
  if(UB_RTC.sek>59) UB_RTC.sek=59;

  if(UB_RTC.tag<1) UB_RTC.tag=1;
  if(UB_RTC.tag>31) UB_RTC.tag=31;
  if(UB_RTC.monat<1) UB_RTC.monat=1;
  if(UB_RTC.monat>12) UB_RTC.monat=12;
  if(UB_RTC.jahr>99) UB_RTC.jahr=99;
  if(UB_RTC.wotag<1) UB_RTC.wotag=1;
  if(UB_RTC.wotag>7) UB_RTC.wotag=7;

  // Zeit einstellen
  RTC_TimeStructure.RTC_Hours = UB_RTC.std;
  RTC_TimeStructure.RTC_Minutes = UB_RTC.min;
  RTC_TimeStructure.RTC_Seconds = UB_RTC.sek;
  if(format==RTC_DEC) {
    RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);   
  }
  else {
    RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);
  }

  // Datum einstellen
  RTC_DateStructure.RTC_Date = UB_RTC.tag;
  RTC_DateStructure.RTC_Month = UB_RTC.monat;
  RTC_DateStructure.RTC_Year = UB_RTC.jahr;
  RTC_DateStructure.RTC_WeekDay = UB_RTC.wotag;
  if(format==RTC_DEC) {
    RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure); 
  }
  else {
    RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure); 
  }

  // Sonstige Settings
  RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
  RTC_InitStructure.RTC_SynchPrediv =  0xFF;
  RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
  RTC_Init(&RTC_InitStructure);  

  // Status-Register beschreiben
  // falls Settings vom User gemacht wurden
  if(UB_RTC.status!=RTC_UNDEFINED) {
    RTC_WriteBackupRegister(RTC_STATUS_REG, RTC_STATUS_TIME_OK); 
  }
}

RTC_t UB_RTC_CalculateTimeFromGmtTime(RTC_t gmt_time)
{
	RTC_t retTime = gmt_time;

	// Hier sind wir garantiert in Winterzeit, also nur eine Stunde addieren
	if( retTime.monat < 3 || retTime.monat > 10 ) {
		 UB_RTC_AddHour(&retTime, 1);
		 return retTime;
	}

	// Hier sind wir garantiert in Sommerzeit, also zwei Stunden addieren
	if( retTime.monat > 3 && retTime.monat < 10 ) {
		UB_RTC_AddHour(&retTime, 2);
		return retTime;
	}

	// Beginn der Sommerzeit im M�rz berechnen
	if( retTime.monat == 3 ) {
		int date = 0;
		if( retTime.jahr == 18 ) {
			date = 25;
		} else if( retTime.jahr == 19 ) {
			date = 31;
		} else if( retTime.jahr == 20 ) {
			date = 29;
		} else if( retTime.jahr == 21 ) {
			date = 28;
		} else if( retTime.jahr == 22 ) {
			date = 27;
		}
		if( retTime.tag < date ) {
			UB_RTC_AddHour(&retTime, 1);
			return retTime;
		}
		if( retTime.tag > date ) {
			UB_RTC_AddHour(&retTime, 2);
			return retTime;
		}
		if( retTime.tag == date ) {
			// ab 1 Uhr an dem Tag haben wir Sommerzeit
			if( retTime.std >= 1 ) {
				UB_RTC_AddHour(&retTime, 2);
				return retTime;
			}
			else {
				UB_RTC_AddHour(&retTime, 1);
				return retTime;
			}
		}
	}

	if( retTime.monat == 10 ) {
		int date = 0;
		if( retTime.jahr == 18 ) {
			date = 28;
		} else if( retTime.jahr == 19 ) {
			date = 27;
		} else if( retTime.jahr == 20 ) {
			date = 25;
		} else if( retTime.jahr == 21 ) {
			date = 31;
		} else if( retTime.jahr == 22 ) {
			date = 20;
		}

		if( retTime.tag < date ) {
			UB_RTC_AddHour(&retTime, 2);
			return retTime;
		}
		if( retTime.tag > date ) {
			UB_RTC_AddHour(&retTime, 1);
			return retTime;
		}
		if( retTime.tag == date ) {
			// ab 1 Uhr haben wir Winterzeit
			if( retTime.std >= 1 ) {
				UB_RTC_AddHour(&retTime, 1);
				return retTime;
			} else {
				UB_RTC_AddHour(&retTime, 2);
				return retTime;
			}
		}
	}

	return retTime;
}

int UB_RTC_AddHour(RTC_t* time, int hours)
{
	time->std = time->std + hours;
	if( time->std > 23 ) {
		time->tag = time->tag + 1;
		time->std = time->std - 24;
	}
	if( time->monat == 1 || time->monat == 3 || time->monat == 5 || time->monat == 7
		|| time->monat == 8 || time->monat == 10 || time->monat == 12 )
	{
		if( time->tag > 31 ) {
			time->monat = time->monat + 1;
			time->tag = time->tag - 31;
		}
	}else if( time->monat == 4 || time->monat == 6 || time->monat == 9 ||
			  time->monat == 11 ){
		if( time->tag > 30 ) {
			time->monat = time->monat + 1;
			time->tag = time->tag - 30;
		}
	}else if( time->monat == 2 ) {
		int schaltjahr = 0;

		if( ((time->jahr + 2000) % 4) == 0 ) {
			schaltjahr = 1;
		}
		if( ((time->jahr + 2000) % 400) == 0 ) {
			schaltjahr = 1;
		}

		if( schaltjahr == 1 ) {
			if( time->tag > 29 ) {
				time->monat = time->monat + 1;
				time->tag = time->tag - 29;
			}
		} else {
			if( time->tag > 28 ) {
				time->monat = time->monat + 1;
				time->tag = time->tag - 28;
			}
		}
	}

	if( time->monat > 12 ) {
		time->jahr = time->jahr + 1;
		time->monat = time->monat - 12;
	}

	return 0;
}


//--------------------------------------------------------------
// RTC auslesen
// nach dem Aufruf steht die aktuelle Uhrzeit
// und das Datum in der Struktur "UB_RTC"
// format : [RTC_DEC, RTC_HEX]
//--------------------------------------------------------------
RTC_t UB_RTC_GetClock(RTC_FORMAT_t format)
{
  RTC_t time;
  // Zeit auslesen
  if(format==RTC_DEC) {
    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
  }
  else {
    RTC_GetTime(RTC_Format_BCD, &RTC_TimeStructure);
  }
  time.std = RTC_TimeStructure.RTC_Hours;
  time.min = RTC_TimeStructure.RTC_Minutes;
  time.sek = RTC_TimeStructure.RTC_Seconds;

  // Datum auslesen
  if(format==RTC_DEC) {
    RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
  }
  else {
    RTC_GetDate(RTC_Format_BCD, &RTC_DateStructure);
  }  
  time.tag = RTC_DateStructure.RTC_Date;
  time.monat = RTC_DateStructure.RTC_Month;
  time.jahr = RTC_DateStructure.RTC_Year;
  time.wotag = RTC_DateStructure.RTC_WeekDay;

  return time;
}


//--------------------------------------------------------------
// WakeUp Interrupt einstellen
//--------------------------------------------------------------
#if RTC_USE_WAKEUP_ISR==1
void UB_RTC_SetWakeUpInterrupt(RTC_WAKEUP_t wakeup)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  EXTI_InitTypeDef  EXTI_InitStructure;
  uint32_t wakeup_time=0x7FF;

  if(wakeup==RTC_WAKEUP_STOP) {
    // WakeUp-Interrupt abschalten
    RTC_WakeUpCmd(DISABLE);
    // disable Interrupt
    RTC_ITConfig(RTC_IT_WUT, DISABLE);

    // NVIC disable
    NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);

    // ext Interrupt 22 disable
    EXTI_ClearITPendingBit(EXTI_Line22);
    EXTI_InitStructure.EXTI_Line = EXTI_Line22;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = DISABLE;
    EXTI_Init(&EXTI_InitStructure);
  }
  else {
    // WakeUp-Interrupt einschalten

    // NVIC init
    NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); 

    // ext Interrupt 22 einstellen (fuer WakeUp)
    EXTI_ClearITPendingBit(EXTI_Line22);
    EXTI_InitStructure.EXTI_Line = EXTI_Line22;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // zum einstellen muss Wakup disabled sein
    RTC_WakeUpCmd(DISABLE);
    
    if(wakeup==RTC_WAKEUP_30s) wakeup_time=0xEFFF;
    if(wakeup==RTC_WAKEUP_10s) wakeup_time=0x4FFF;
    if(wakeup==RTC_WAKEUP_5s) wakeup_time=0x27FF;
    if(wakeup==RTC_WAKEUP_1s) wakeup_time=0x7FF;
    if(wakeup==RTC_WAKEUP_500ms) wakeup_time=0x3FF;
    if(wakeup==RTC_WAKEUP_250ms) wakeup_time=0x1FF;
    if(wakeup==RTC_WAKEUP_125ms) wakeup_time=0xFF;

    // Teiler 16 => 32,768kHz:16 => 2048 Hz
    RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);
    // WakeUp Counter einstellen
    RTC_SetWakeUpCounter(wakeup_time);

    // enable Interrupt
    RTC_ITConfig(RTC_IT_WUT, ENABLE);

    // enable Wakeup
    RTC_WakeUpCmd(ENABLE);
    
  }
}
#endif



//--------------------------------------------------------------
// diese Funktion wird zyklisch aufgerufen,
// bei einem WakeUp-Interrupt
// (je nachdem welche WakeUp-Time eingestellt ist)
//--------------------------------------------------------------
void WC_OneMinute_ISR();
#if RTC_USE_WAKEUP_ISR==1
void P_RTC_WAKEUP_ISR(void)
{
	WC_OneMinute_ISR();
}
#endif







//--------------------------------------------------------------
// interne Funktion
// RTC Settings einstellen
//--------------------------------------------------------------
void P_RTC_Config(void)
{
  // Clock auf LSE einstellen
  RCC_LSEConfig(RCC_LSE_ON);

  // warten bis Clock eingestellt
  while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);

  // Clock enable fuer LSE
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

  // enable RTC
  RCC_RTCCLKCmd(ENABLE);

  // warte bis synconisiert
  RTC_WaitForSynchro();

  // Status-Register beschreiben
  RTC_WriteBackupRegister(RTC_STATUS_REG, RTC_STATUS_INIT_OK);
}


//--------------------------------------------------------------
// WakeUp-Interrupt
// wird zyklisch aufgerufen, wenn er aktiviert ist
//--------------------------------------------------------------
#if RTC_USE_WAKEUP_ISR==1
void RTC_WKUP_IRQHandler(void)
{
  if(RTC_GetITStatus(RTC_IT_WUT) != RESET)
  {
    // Interrupt Flags loeschen
    RTC_ClearITPendingBit(RTC_IT_WUT);
    EXTI_ClearITPendingBit(EXTI_Line22);

    // User-Funktion ausf�hren
    P_RTC_WAKEUP_ISR();
  }
}
#endif
