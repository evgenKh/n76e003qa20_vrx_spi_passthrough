#if USE_MAIN_APROM

#include "numicro_8051.h"

#if !USE_PRINTF
#define printf(...) PRINTF_NOT_ALLOWED_TO_SAVE_ROM;
#endif


#include "pins.h"
#include "delay.h"
#include "SoftwareSpiSlave.h"

#if USE_MAIN_SIMPLE_PASS_THROUGH
BIT currentCsValue;
BIT previousCsValue;

void main(void)
{
   PIN_SPIMASTER_MOSI_PUSHPULL_MODE;
   PIN_SPIMASTER_SCK_PUSHPULL_MODE;
   PIN_SPIMASTER_CS_PUSHPULL_MODE;
   PIN_MUXOUT_IN_INPUT_MODE;

   //PIN_CH1/PIN_SPISLAVE_SCK
   P30_INPUT_MODE;
   P30_ST_ENABLE;

   //PIN_CH2/PIN_SPISLAVE_CS
   P04_INPUT_MODE;
   P04_ST_ENABLE;

   //PIN_CH3/PIN_SPISLAVE_MOSI
   P03_INPUT_MODE;
   P03_ST_ENABLE;

   //Pull up for inputs?
   //PIN_CH1 = 1;
   //PIN_CH2 = 1;
   //PIN_CH3 = 1;
   //PIN_S1 = 1;    
   previousCsValue = PIN_CH2;
   while (1)
   {
      currentCsValue = PIN_CH2;

      if(!currentCsValue || !previousCsValue)//If CS is low, or if became high only on this cycle.
      {
         //in and out are on different ports, should be no problems with "load-edit-save"
         PIN_SPIMASTER_MOSI = PIN_CH3;
         PIN_SPIMASTER_CS = currentCsValue;
         PIN_SPIMASTER_SCK = PIN_CH1;

         previousCsValue = currentCsValue;
      }
      PIN_SPIMASTER_CS = currentCsValue;
   }
}
#endif // USE_MAIN_SIMPLE_PASS_THROUGH

#endif // USE_APROM