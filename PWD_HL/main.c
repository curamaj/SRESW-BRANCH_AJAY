#include <stdio.h>
#include <string.h>
#include "IO_Driver.h"
#include "IO_UART.h"
#include "IO_PWD.h"
#include "IO_PWM.h"
#include "IO_RTC.h"
#include "IO_DIO.h"
#include "IO_ADC.h"

#include "APDB.h"
#include "serial.h" //try to use IO_UART stuff before using serial manager stuff rusty has.

// figure out a way to read the 55505 sensor, and figure out what kind of output it has.
// Seems like the output is pulse, so we need to set up a pulse measurement input.
// could be a complex output too, try the pulse one first.

APDB appl_db =
          { 0                      /* ubyte4 versionAPDB        */
          , {0}                    /* BL_T_DATE flashDate       */
                                   /* BL_T_DATE buildDate                   */
          , { (ubyte4) (((((ubyte4) RTS_TTC_FLASH_DATE_YEAR) & 0x0FFF) << 0) |
                        ((((ubyte4) RTS_TTC_FLASH_DATE_MONTH) & 0x0F) << 12) |
                        ((((ubyte4) RTS_TTC_FLASH_DATE_DAY) & 0x1F) << 16) |
                        ((((ubyte4) RTS_TTC_FLASH_DATE_HOUR) & 0x1F) << 21) |
                        ((((ubyte4) RTS_TTC_FLASH_DATE_MINUTE) & 0x3F) << 26)) }
          , 0                      /* ubyte4 nodeType           */
          , 0                      /* ubyte4 startAddress       */
          , 0                      /* ubyte4 codeSize           */ 
          , 0                      /* ubyte4 legacyAppCRC       */
          , 0                      /* ubyte4 appCRC             */
          , 1                      /* ubyte1 nodeNr             */
          , 0                      /* ubyte4 CRCInit            */
          , 0                      /* ubyte4 flags              */
          , 0                      /* ubyte4 hook1              */
          , 0                      /* ubyte4 hook2              */
          , 0                      /* ubyte4 hook3              */
          , APPL_START             /* ubyte4 mainAddress        */
          , {0, 1}                 /* BL_T_CAN_ID canDownloadID */
          , {0, 2}                 /* BL_T_CAN_ID canUploadID   */
          , 0                      /* ubyte4 legacyHeaderCRC    */
          , 0                      /* ubyte4 version            */
          , 500                    /* ubyte2 canBaudrate        */
          , 0                      /* ubyte1 canChannel         */
          , {0}                    /* ubyte1 reserved[8*4]      */
          , 0                      /* ubyte4 headerCRC          */
          };


ubyte4 timestamp = 0;

void UART_Write(ubyte1 *buffer, ubyte1 length){
	ubyte1 actualwrittenbytes;
	IO_UART_Write(IO_UART_RS232, buffer, length, &actualwrittenbytes); //length will be the return number of sprintf (i.e. actuall bytes that are to be written, not length of whole buffer)
	do {
		IO_UART_GetTxStatus(IO_UART_RS232, &actualwrittenbytes);
		IO_UART_Task();
    }while (actualwrittenbytes != 0);			
}


void timeDelayInSeconds(ubyte2 seconds){
	ubyte4 timestamp = 0;
	IO_RTC_StartTime(&timestamp);
	while (IO_RTC_GetTimeUS(timestamp) < seconds*1000000){} //delay is in seconds!
}

void pumpControlMinToMaxSpeed(){
	
	ubyte1 storagebuffer[100]; //character storage buffer, this should keep being cleared up in order for the storage buffer to be clear for new input.		
	IO_PWM_Init( IO_PWM_05, 100, TRUE, FALSE, 0, FALSE, NULL ); // keep a 100HZ frequency, keep set duty right next to this when actually implementing code
//	IO_DO_Init(IO_DO_02); 
	IO_DO_Set(IO_DO_02,TRUE);
//	timeDelayInSeconds(5);
	ubyte1 counter = 0;
	for (ubyte2 changingduty = 7865; changingduty <= 60948; changingduty = changingduty+656){
		IO_PWM_SetDuty (IO_PWM_05, changingduty, NULL); // keep altering duty cycle in for loop.
//		IO_DO_Set(IO_DO_02,TRUE);
		ubyte2 sizeofdata = sprintf((char *)storagebuffer, "%i. Changing Duty is: %u. \n", counter, changingduty);
		counter++;
		UART_Write(storagebuffer, sizeofdata);
		timeDelayInSeconds(1);						
	}
}
//104 is pwm signal, variable
//143 is pwm 4A at 100%

void pumpControllerByPotentiometer(){

	ubyte1 storagebuffer[100]; //character storage buffer, this should keep being cleared up in order for the storage buffer to be clear for new input.	
	ubyte2 potvalue = 0;
	bool fresh = FALSE;
	ubyte1 counter = 0;
	IO_PWM_Init( IO_PWM_05, 100, TRUE, FALSE, 0, FALSE, NULL ); // keep a 100HZ frequency, keep set duty right next to this when actually implementing code
	//IO_DO_Init(IO_DO_02); 
	IO_DO_Set(IO_DO_02,TRUE);
	IO_ADC_ChannelInit( IO_ADC_5V_04, IO_ADC_RESISTIVE, 0, 0, 0, NULL );	
	timeDelayInSeconds(5);
	while (1){
		IO_ADC_Get(IO_ADC_5V_04, &potvalue, &fresh);
	//	IO_PWM_SetDuty (IO_PWM_05, changingduty, NULL); // keep altering duty cycle in for loop.
	//	IO_DO_Set(IO_DO_02,TRUE);
		ubyte2 sizeofdata = sprintf((char *)storagebuffer, "%i. Resistance Value: %u, Fresh(1=T,0=F): %i \n", counter, potvalue, fresh);
		counter++;
		if (counter == 200){
			counter = 0;
		}
		UART_Write(storagebuffer, sizeofdata);
		timeDelayInSeconds(1);						
	}
	
}

void systemInit(){ //call this function to initialize everything.
	IO_Driver_Init( NULL ); 
	IO_UART_Init(IO_UART_RS232, 115200, 8, IO_UART_PARITY_NONE, 1); //initialize RS232 Serial Communication.
	
	 
}



void main(void)
{
	//SerialManager* serialMan = SerialManager_new();
	systemInit(); 
	IO_Driver_TaskBegin();
	//pumpControllerByPotentiometer();
	IO_DO_Init(IO_DO_02);IO_DO_Set(IO_DO_02, FALSE);
	timeDelayInSeconds(3);
	pumpControlMinToMaxSpeed();
    IO_Driver_TaskEnd();
}









































