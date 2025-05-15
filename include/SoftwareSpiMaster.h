#ifndef SoftwareSpiMaster_H
#define SoftwareSpiMaster_H

#if USE_SOFTWARE_SPI_MASTER

/*
 **********************************************************
 * 8051 Bit-Banged SPI
 *
 * MAXIM INTEGRATED PRODUCTS
 *
 **********************************************************
 */

#include "N76E003.h"

// GLOBAL VARIABLES ---------------------------------------
#define N_OF_SPI_BYTES 4

// FUNCTION PROTOTYPES ------------------------------------
void spiReadWriteBlock(void);

unsigned char /*data*/ spiData[N_OF_SPI_BYTES];

void LoopSpiMaster(void);

#endif // #if USE_SOFTWARE_SPI_MASTER

#endif 