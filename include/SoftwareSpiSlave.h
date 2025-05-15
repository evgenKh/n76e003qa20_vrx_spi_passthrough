#ifndef SoftwareSpiSlave_H
#define SoftwareSpiSlave_H

#if USE_SW_SPI_SLAVE

#include "numicro_8051.h"
#include "IAPMessage.h"

#define STR(x) #x
#define XSTR(s) STR(s)

#define SPISLAVE_BUFSIZE IAP_MESSAGE_SIZE
#define SPISLAVE_USE_INT0_FALLING_FOR_SCK 1
#define SPISLAVE_USE_P30_RISING_FOR_SCK 0
#define SPISLAVE_USE_P30_FALLING_FOR_SCK 0
#define SPISLAVE_USE_P04_BOTHEDGE_FOR_CS 1

//If an interrupt service routine changes variables which are accessed by other functions these variables have to be declared volatile.
extern __xdata volatile uint8_t    g_spiSlaveRxBuf[SPISLAVE_BUFSIZE];
extern volatile BIT                g_spiSlaveIsReceiveFinished;
extern __data volatile uint8_t     g_spiSlaveRxBufHead;

#if USE_SPI_SLAVE_TX
extern __xdata volatile uint8_t    g_spiSlaveTxBuf[SPISLAVE_BUFSIZE];
extern __data volatile uint8_t     g_spiSlaveTxBufHead;
#endif

void SpiSlaveInitData(void);
void SpiSlaveInitPins(void);

void SpiSlaveShutdown(void);

void SpiSlaveCsListenStart(void);
void SpiSlaveCsListenStop(void);

void SpiSlaveReceiveStart(void);
void SpiSlaveReceiveStop(void);

inline void SpiSlaveTick(void);

//MUST declare ISRs in main.c or in this header!
void PinInterrupt_ISR (void) __interrupt (7);
void INT0_ISR(void) __interrupt (0); 

#endif // USE_SW_SPI_SLAVE
#endif