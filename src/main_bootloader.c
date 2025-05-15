#if USE_MAIN_LDROM

#include "numicro_8051.h"

#if !USE_PRINTF
#define printf(...) PRINTF_NOT_ALLOWED_TO_SAVE_ROM_SPACE;
#endif

#include "pins.h"
#include "delay.h"
#include "IAPMessage.h"
#include "SoftwareSpiSlave.h"
#include "IAPReceiver.h"



#define BOOT_TIMEOUTED_FLAG TF2
uint32_t __data timer2CounterTmp;

void RestartBootTimer(uint16_t timeoutMs)
{
   SFRS_TMP = SFRS;
   SFRS = 0;//TH2, TL2 are on page 0 only!

   clr_T2CON_TF2;
   clr_T2CON_TR2;
   set_T2CON_CMRL2;   
   T2MOD = 0;
   set_T2MOD_T2DIV0;
   set_T2MOD_T2DIV1;
   set_T2MOD_T2DIV2;
   timer2CounterTmp = 65535ul-(16000ul/512ul*timeoutMs);
   //Will count up from TIMER2CT to 65535
   TH2 = HIBYTE(timer2CounterTmp);
   TL2 = LOBYTE(timer2CounterTmp);
   RCMP2H = 0xFF;
   RCMP2L = 0xFF;
   set_T2CON_TR2;//Start Timer2 

   SFRS = SFRS_TMP;
}

void main(void)
{
#if USE_CLOCK_FUNCTIONS
   MODIFY_HIRC(HIRC_16);//Set 16MHz clock
   FsysSelect(FSYS_HIRC);
#endif

#if USE_UART0
   P06_QUASI_MODE;//Rxd
   P07_QUASI_MODE;//Txd
   UART_Open(16000000,UART0_Timer1,9600);
   //ENABLE_UART0_PRINTF;
   DISABLE_UART0_INTERRUPT;
   //printf("\nHello!");
#endif

   SpiSlaveInitData();
   SpiSlaveInitPins();
   SpiSlaveCsListenStart();

   //Using Timer0 for boot timeout
   RestartBootTimer(600);//600ms for any valid IAP message.
   IAPReceiverInit();
   set_CHPCON_IAPEN;//enable iap
   volatile bool parseOk = false;
   volatile bool execOk = false;

   while (1)
   {
      if(g_spiSlaveIsReceiveFinished && g_spiSlaveRxBufHead)
      {
         SpiSlaveCsListenStop();
         //Switch led when buf is full.
         //PIN_LED = !PIN_LED;
         IAPReceiverReset();
         parseOk = IAPReceiverParseMessage((IAPMessage*)g_spiSlaveRxBuf, g_spiSlaveRxBufHead);
         if(!parseOk)
         {
            IAPLogWrite("Msg parse err.Len=");
            IAPLogWrite(IAPitoa(g_spiSlaveRxBufHead));
         }
         else
         {
            DISABLE_GLOBAL_INTERRUPT;
            execOk = 0;
            execOk = IAPReceiverExecuteMessage();
            if(!execOk)
            {
               IAPLogWrite("Msg exec err");
            }
            ENABLE_GLOBAL_INTERRUPT;

            //If any msg parsed successfully, restart timer with bigger value
            RestartBootTimer(2000);//Restarting timer as late as possible            
         }
#if USE_UART0
         //g_spiSlaveRxBuf[g_spiSlaveRxBufHead-1] = '\0';
         //puts(g_spiSlaveRxBuf);
#endif
         IAPReceiverReset();
         g_spiSlaveRxBufHead = 0;//Consume buffer
         
         //Flush txt log to SPi tx buf
         IAPLogFlushToMem(g_spiSlaveTxBuf, SPISLAVE_BUFSIZE);
         g_spiSlaveTxBufHead = SPISLAVE_BUFSIZE - 1;
         SpiSlaveCsListenStart();
      }
      SpiSlaveTick();
      if(TF2)
      {
#if USE_UART0
         puts("timeout Boot APROM");
#endif         
         break;
      }
   }
   
   SpiSlaveShutdown();//will turn off led and set pins to inputs.

   Software_Reset(BOOT_APROM);//Boot to main app
}

#endif // USE_LDROM