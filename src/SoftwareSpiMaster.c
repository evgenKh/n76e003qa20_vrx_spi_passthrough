#if USE_SOFTWARE_SPI_MASTER

#include "SoftwareSpiMaster.h"


#include "N76E003.h"

// CONSTANTS ----------------------------------------------
#define CPOL                     1  // Set CPOL to 1 or 0
#define CPHA                     1  // Set CPHA to 1 or 0
#define CS_TOGGLE_BETWEEN_BYTES  0  // Set CS toggle
                                    //   0=false  1=true

// MACROS ------------------------------------------------- 
#if CPHA
   #define SCK_POST
   #if CPOL
      #define SCK_INIT 1
      #define SCK_PRE  SCK=0
      #define SCK_MID  SCK=1
   #else
      #define SCK_INIT 0
      #define SCK_PRE  SCK=1
      #define SCK_MID  SCK=0
   #endif
#else
   #define SCK_PRE
   #if CPOL
      #define SCK_INIT 1
      #define SCK_MID  SCK=0
      #define SCK_POST SCK=1
   #else
      #define SCK_INIT 0
      #define SCK_MID  SCK=1
      #define SCK_POST SCK=0
   #endif
#endif

#if CS_TOGGLE_BETWEEN_BYTES
   #define CS_TOGGLE CS=1;CS=0
#else
   #define CS_TOGGLE
#endif

// PIN DEFINITIONS ----------------------------------------
//sfr  PORT_0 = 0x80;
//sbit CS     = PORT_0 ^ 1;
//sbit SCK    = PORT_0 ^ 2;
//sbit MOSI   = PORT_0 ^ 3;
//sbit MISO   = PORT_0 ^ 4;

#define MISO 0
#define MOSI P1_1
#define SCK P1_5
#define CS P1_0
#define PIN_UNKNOWN_OUT P1_2

// BIT-ADDRESSABLE GLOBAL VARIABLES -----------------------
unsigned char bdata spiTmp;
sbit                spiTmp7 = spiTmp ^ 7;
sbit                spiTmp6 = spiTmp ^ 6;
sbit                spiTmp5 = spiTmp ^ 5;
sbit                spiTmp4 = spiTmp ^ 4;
sbit                spiTmp3 = spiTmp ^ 3;
sbit                spiTmp2 = spiTmp ^ 2;
sbit                spiTmp1 = spiTmp ^ 1;
sbit                spiTmp0 = spiTmp ^ 0;


// FUNCTION spiReadWriteByte ------------------------------
// 
// Data is transfered starting from spiData[N_OF_SPI_BYTES-1]
// to spiData[0] MSb first. The received data replaces the
// existing data from spiData[N_OF_SPI_BYTES-1] to spiData[0].
//
// NOTE: this function assumes that
//       SCK=SCK_INIT and CS=1
void spiReadWriteBlock(void)
{
   unsigned char data i = N_OF_SPI_BYTES-1;

   CS = 0;
   while(1)
   {
      spiTmp = spiData[i];
      SCK_PRE; MOSI=spiTmp7; SCK_MID; spiTmp7=MISO; SCK_POST; // bit 7
      SCK_PRE; MOSI=spiTmp6; SCK_MID; spiTmp6=MISO; SCK_POST; // bit 6
      SCK_PRE; MOSI=spiTmp5; SCK_MID; spiTmp5=MISO; SCK_POST; // bit 5
      SCK_PRE; MOSI=spiTmp4; SCK_MID; spiTmp4=MISO; SCK_POST; // bit 4
      SCK_PRE; MOSI=spiTmp3; SCK_MID; spiTmp3=MISO; SCK_POST; // bit 3
      SCK_PRE; MOSI=spiTmp2; SCK_MID; spiTmp2=MISO; SCK_POST; // bit 2
      SCK_PRE; MOSI=spiTmp1; SCK_MID; spiTmp1=MISO; SCK_POST; // bit 1
      SCK_PRE; MOSI=spiTmp0; SCK_MID; spiTmp0=MISO; SCK_POST; // bit 0
      spiData[i] = spiTmp;

      if (i == 0)
         break;
      i--;

      CS_TOGGLE;
   }
   CS = 1;
}

// MAIN ---------------------------------------------------
void LoopSpiMaster(void)
{
   // 0. Init SPI Pins
   CS  = 1;
   SCK = SCK_INIT;

   // 1. Program Loop...
   while (1)
   {
      spiData[2] = 0x40;
      spiData[1] = 0x41;
      spiData[0] = 0x42;
      spiReadWriteBlock();
   }
}

#endif // USE_SOFTWARE_SPI_MASTER