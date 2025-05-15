#if USE_SW_SPI_SLAVE
#include "SoftwareSpiSlave.h"
#include "pins.h"

//If an interrupt service routine changes variables which are accessed by other functions these variables have to be declared volatile.
__xdata volatile uint8_t    g_spiSlaveRxBuf[SPISLAVE_BUFSIZE];
volatile BIT                spiSlaveIsByteReady;
volatile BIT                g_spiSlaveIsReceiveFinished;
__data volatile uint8_t     spiSlaveRxByteBuf;
__data volatile uint8_t     g_spiSlaveRxBufHead;
// pointer physically in internal ram pointing to object in external ram
__xdata volatile uint8_t* __data volatile spiSlaveRxBufHeadPtr;

#if USE_SPI_SLAVE_TX
__xdata volatile uint8_t    g_spiSlaveTxBuf[SPISLAVE_BUFSIZE];
volatile BIT                g_spiSlaveIsTransmitFinished;
volatile BIT                spiSlaveTransmitBitOnSckRising;
__data volatile uint8_t     spiSlaveTxByteBuf;
__data volatile uint8_t     g_spiSlaveTxBufHead;
// pointer physically in internal ram pointing to object in external ram
__xdata volatile uint8_t* __data volatile spiSlaveTxBufHeadPtr;
#endif

void SpiSlaveInitData(void)
{
    g_spiSlaveRxBufHead = 0;//Will write byte to this index of array. Also a size of received data
    g_spiSlaveIsReceiveFinished = FALSE;
#if USE_SPI_SLAVE_TX
    g_spiSlaveTxBufHead = 0;
    g_spiSlaveIsTransmitFinished = FALSE;
    spiSlaveTransmitBitOnSckRising = FALSE;
    g_spiSlaveTxBuf[0] = 0;
#endif
}

void SpiSlaveInitPins(void)
{
    //PIN_CH1/PIN_SPISLAVE_SCK
    P30_INPUT_MODE;
    P30_ST_ENABLE;

    //PIN_CH2/PIN_SPISLAVE_CS
    P04_INPUT_MODE;
    P04_ST_ENABLE;

    //PIN_CH3/PIN_SPISLAVE_MOSI
    P03_INPUT_MODE;
    P03_ST_ENABLE;

   //No pull up for inputs.
   //PIN_CH1 = 1;
   //PIN_CH2 = 1;
   //PIN_CH3 = 1;
   //PIN_S1 = 1; 

#if USE_SPI_SLAVE_TX
   PIN_SPI_SLAVE_MISO_QUASI_MODE;  //Can not set push-pull in case MISO==ICPDA
   PIN_SPI_SLAVE_MISO = 0;//turn on LED of boot start
#endif
}


void SpiSlaveShutdown(void)
{
   SpiSlaveCsListenStop();
   //PIN_CH1/PIN_SPISLAVE_SCK
   P30_INPUT_MODE;
   P04_INPUT_MODE;
   P03_INPUT_MODE; 

#if USE_SPI_SLAVE_TX
   PIN_SPI_SLAVE_MISO = 1;//turn off LED on bootloader exit
   PIN_SPI_SLAVE_MISO_INPUT_MODE;
#endif
}

void SpiSlaveCsListenStart(void)
{
#if SPISLAVE_USE_P04_BOTHEDGE_FOR_CS
   ENABLE_INT_PORT0;
   ENABLE_BIT4_BOTHEDGE_TRIG;
/*
   SFRS_TMP = SFRS;
   SFRS = 0;
   CLEAR_PIN_INTERRUPT_PIT4_FLAG;
   if (SFRS_TMP) ENABLE_SFR_PAGE1;
*/
   ENABLE_PIN_INTERRUPT;
#else
   SpiSlaveReceiveStart();
#endif
   ENABLE_GLOBAL_INTERRUPT;
}

void SpiSlaveCsListenStop(void)
{   
#if SPISLAVE_USE_P04_BOTHEDGE_FOR_CS
   DISABLE_PIN_INTERRUPT;
#endif
}

void SpiSlaveReceiveStart(void)
{
    spiSlaveRxByteBuf = 0x01;//when this bit shifted out to carry, byte-buf is full.
    g_spiSlaveRxBufHead = 0;//Will write byte to this index of array. Also a size of received data
    spiSlaveRxBufHeadPtr = &g_spiSlaveRxBuf[0];//Will write byte to this ptr
    spiSlaveIsByteReady = FALSE;
    g_spiSlaveIsReceiveFinished = FALSE;

#if USE_SPI_SLAVE_TX
    spiSlaveTxBufHeadPtr = &g_spiSlaveTxBuf[0];//Will read byte from this ptr
    spiSlaveTxByteBuf = *spiSlaveTxBufHeadPtr;
    g_spiSlaveIsTransmitFinished = (g_spiSlaveTxBufHead == 0);
    spiSlaveTransmitBitOnSckRising = FALSE;
    //Set first bit of TX data BEFORE we got clock pulse, Even if txbuf is 'empty'
    
    __asm__ (
      "mov A, _spiSlaveTxByteBuf       ;slaveSpiByteBuf from RAM to Accumulator\n"
      "rlc A                           ;Shift Accumulator to left. Carry shifted-out is highest bit. \n"
      SPI_SLAVE_CARRY_TO_MISO_ASM
      "mov _spiSlaveTxByteBuf, A       ;Store back to RAM\n"
      );
#endif
#if SPISLAVE_USE_INT0_FALLING_FOR_SCK
   INT0_FALLING_EDGE_TRIG;
   CLEAR_INT0_INTERRUPT_FLAG;
   ENABLE_INT0_INTERRUPT;
#elif SPISLAVE_USE_P30_RISING_FOR_SCK
   ENABLE_INT_PORT3;
   ENABLE_BIT0_RISINGEDGE_TRIG;

   SFRS_TMP = SFRS;
   SFRS = 0;
   CLEAR_PIN_INTERRUPT_PIT0_FLAG;
   if (SFRS_TMP) ENABLE_SFR_PAGE1;

   ENABLE_PIN_INTERRUPT;
#else
   ENABLE_INT_PORT3;
   ENABLE_BIT0_FALLINGEDGE_TRIG;

   SFRS_TMP = SFRS;
   SFRS = 0;
   CLEAR_PIN_INTERRUPT_PIT0_FLAG;
   if (SFRS_TMP) ENABLE_SFR_PAGE1;

   ENABLE_PIN_INTERRUPT;
#endif
   ENABLE_GLOBAL_INTERRUPT;
}


void SpiSlaveReceiveStop(void)
{
#if SPISLAVE_USE_INT0_FALLING_FOR_SCK
   DISABLE_INT0_INTERRUPT;
#else
   DISABLE_PIN_INTERRUPT;
   SFRS_TMP = SFRS;              /* for SFRS page */

   SFRS = 0;
   clr_PIF0;
   //CLEAR_PIN_INTERRUPT_PIT0_FLAG;
   if (SFRS_TMP)                 /* for SFRS page */
   {
   ENABLE_SFR_PAGE1;
   }
#endif
   g_spiSlaveIsReceiveFinished = TRUE;
}


//Interrupt service routines

#if SPISLAVE_USE_P04_BOTHEDGE_FOR_CS
void PinInterrupt_ISR (void) __interrupt (7)
{
   //Clearing this interrupt flag on page 0
   SFRS_TMP = SFRS;
   SFRS = 0;
   PIF&=CLR_BIT4;
   SFRS = SFRS_TMP;

   if(!P04)
        SpiSlaveReceiveStart();//Falling edge
   else
        SpiSlaveReceiveStop();//Rising edge
}
#endif


#if SPISLAVE_USE_INT0_FALLING_FOR_SCK
void INT0_ISR(void) __interrupt (0)          // Vector @  0x03
#else
void PinInterrupt_ISR (void) __interrupt (7)
#endif
{
#if SPISLAVE_USE_INT0_FALLING_FOR_SCK
   //clr_TCON_IE0;          //clr int flag wait next falling edge
   //CLEAR_INT0_INTERRUPT_FLAG;
#else
   SFRS_TMP = SFRS;
   SFRS = 0;
   CLEAR_PIN_INTERRUPT_PIT0_FLAG;
   if (SFRS_TMP) ENABLE_SFR_PAGE1;
#endif
   

  __asm__ (
     "mov A, _spiSlaveRxByteBuf   ;slaveSpiByteBuf from RAM to Accumulator\n"
     "mov C, P0.3                   ;MOSI pin to Carry bit\n"
     "rlc A                         ;Shift Accumulator to left. Carry shifted-in to lowest bit. \n"
     "mov _spiSlaveRxByteBuf, A   ;Store back to RAM\n"
     "mov _spiSlaveIsByteReady, C ;Highest bit shifted-out to Carry\n" 
     );
     
     
   if(spiSlaveIsByteReady)
   {
      //Store byte from RAM to XRAM
      //Should be skipped by jnc if byte not ready. But still safe if not skipped
      //slaveSpiRxBuf[slaveSpiRxBufHead] = slaveSpiByteBuf;
      *spiSlaveRxBufHeadPtr = spiSlaveRxByteBuf;
      spiSlaveRxByteBuf = 0x01;
      //But increments must be skipped!
      ++spiSlaveRxBufHeadPtr;//inc dptr
      ++g_spiSlaveRxBufHead;

      g_spiSlaveIsReceiveFinished |= (g_spiSlaveRxBufHead == SPISLAVE_BUFSIZE);
      
#if USE_SPI_SLAVE_TX
      g_spiSlaveIsTransmitFinished |= g_spiSlaveIsReceiveFinished;
      g_spiSlaveIsTransmitFinished |= (g_spiSlaveTxBufHead == 0);
      if(!g_spiSlaveIsTransmitFinished)
      {
         ++spiSlaveTxBufHeadPtr;//inc dptr
         --g_spiSlaveTxBufHead;
         spiSlaveTxByteBuf = *spiSlaveTxBufHeadPtr;
      }
#endif
   }//if g_spiSlaveIsByteReady

   
   #if USE_SPI_SLAVE_TX
   if(!g_spiSlaveIsReceiveFinished)
   {
      spiSlaveTransmitBitOnSckRising = TRUE;
   }
   #endif


   /*
      __asm__ (
         "jnc 0002$                             ;if(!slaveSpiByteReady) goto label_byte_not_ready\n"
         "                                      ;if C is set, then store A to XRAM directly\n"
         "mov dpl, _g_spiSlaveRxBufHeadPtr      ;reconstructing 16-bit DPTR from 2 parts\n"
         "mov dph, _g_spiSlaveRxBufHeadPtr+1    ;\n"
         "mov A, _g_spiSlaveRxByteBuf              ;slaveSpiByteBuf from RAM to Accumulator\n"
         "movx @dptr, A                         ;*slaveSpiByteBufWriteHead = slaveSpiByteBuf\n"
         "inc dptr                              ;slaveSpiByteBufWriteHead++\n"
         "mov _g_spiSlaveRxBufHeadPtr, dpl    ;store DPTR back to ram\n"
         "mov _g_spiSlaveRxBufHeadPtr+1, dph  ;\n"
         
         "mov _g_spiSlaveRxByteBuf, #0x01           ;slaveSpiByteBuf=0x01\n"

         "mov A, _g_spiSlaveRxBufHead             ;\n"
         "inc A                                 ;slaveSpiRxBufHead++\n"
         "mov _g_spiSlaveRxBufHead, A             ;\n"

         "cjne A, #0x40, 0001$      ;if(slaveSpiRxBufHead != SPISLAVE_BUFSIZE) goto buffer_not_full\n"
         "setb _g_spiSlaveIsReceiveFinished          ;_slaveSpiReceiveFinished = TRUE\n"
         "0001$: ;buffer_not_full\n"
         "0002$: ;label_byte_not_ready\n"

         ); */

#if !SPISLAVE_USE_P04_BOTHEDGE_FOR_CS
   if(P04)
   {
      SpiSlaveReceiveStop();
   }
#endif
}

inline void SpiSlaveTick(void)
{
     //Writing next bit of TX on rising edge
#if USE_SPI_SLAVE_TX
   if(spiSlaveTransmitBitOnSckRising && PIN_SPI_SLAVE_SCK)
   {
      __asm__ (        
         "mov A, _spiSlaveTxByteBuf       ;slaveSpiByteBuf from RAM to Accumulator\n"
         "rlc A                           ;Shift Accumulator to left. Carry shifted-out is highest bit. \n"
         SPI_SLAVE_CARRY_TO_MISO_ASM
         "mov _spiSlaveTxByteBuf, A       ;Store back to RAM\n"
         );
      spiSlaveTransmitBitOnSckRising = FALSE;
   }

#endif
}


#endif