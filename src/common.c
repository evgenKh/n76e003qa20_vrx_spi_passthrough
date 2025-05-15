/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* SPDX-License-Identifier: Apache-2.0                                                                     */
/* Copyright(c) 2023 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#if USE_NUVOTON
#include "numicro_8051.h"

BIT BIT_TMP;

#if defined __C51__
uint8_t data  TA_REG_TMP,BYTE_TMP,SFRS_TMP;

#elif defined __ICC8051__
uint8_t __data  TA_REG_TMP,BYTE_TMP,SFRS_TMP;

#elif defined __SDCC__
__data uint8_t TA_REG_TMP,BYTE_TMP,SFRS_TMP;
#endif

/**
 * @brief       Read Dataflash
 * @param       Dataflash start address
 * @return      Dataflash Value
 * @details     None
**/
#if defined __C51__
uint8_t Read_APROM_BYTE(uint16_t code *u16_addr)

#elif defined __ICC8051__
uint8_t Read_APROM_BYTE(const uint16_t __code *u16_addr)

#elif defined __SDCC__
uint8_t Read_APROM_BYTE(const uint16_t __code *u16_addr)

#endif
{
    uint8_t rdata;
    rdata = *u16_addr >> 8;
    return rdata;
}


/**
 * @brief       Software reset API
 * @param       Run in APROM/LDROM after reset
 *                       - \ref BOOT_APROM: Run in APROM after reset .
 *                       - \ref BOOT_LDROM: Run in LDROM after reset.
 * @return      None
 * @details     None
 */
void Software_Reset(uint8_t u8Bootarea)
{
    uint8_t boottemp;
    boottemp = (0x80|u8Bootarea);
    TA = 0xAA;
    TA = 0x55;
    CHPCON = boottemp;                   //software reset enable
}


/**
 * @brief       Software loop delay by HIRC, about 3ms
 * @param       None
 * @return      None
 * @details     None
 */

/*
void _delay_(void)
{
    uint8_t t1,t2;

    for (t2=0;t2<0x1A;t2++)
    {
       for (t1=0;t1<0x7f;t1++)
       {
          CALL_NOP;
       }
    }
}*/
#endif