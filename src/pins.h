#ifndef pins_H
#define pins_H

#if USE_NUVOTON

//hardware immutable pins
#define PIN_CH1 P30
#define PIN_CH2 P04
#define PIN_CH3 P03
#define PIN_S1 P14

#if USE_SPI_SLAVE_TX
#   if USE_S1_AS_SPISLAVE_MISO
//S1 is P1.4
#       define PIN_SPI_SLAVE_MISO P14
#       define PIN_SPI_SLAVE_MISO_INPUT_MODE P14_INPUT_MODE
#       define PIN_SPI_SLAVE_MISO_QUASI_MODE P14_QUASI_MODE
//Hack to "pass" pin number to asm
#       define SPI_SLAVE_CARRY_TO_MISO_ASM "mov P1.4, C   ;Carry bit to MISO pin\n"
#   elif USE_ICPDA_AS_SPISLAVE_MISO
//ICPDA is P1.6
#       define PIN_SPI_SLAVE_MISO P16
#       define PIN_SPI_SLAVE_MISO_INPUT_MODE P16_INPUT_MODE
#       define PIN_SPI_SLAVE_MISO_QUASI_MODE P16_QUASI_MODE
//Hack to "pass" pin number to asm
#       define SPI_SLAVE_CARRY_TO_MISO_ASM "mov P1.6, C   ;Carry bit to MISO pin\n"
#   else
#       error "define USE_S1_AS_SPISLAVE_MISO or USE_ICPDA_AS_SPISLAVE_MISO"
#   endif
#endif

#define PIN_SPIMASTER_MOSI P11
#define PIN_SPIMASTER_SCK P15
#define PIN_SPIMASTER_CS P10
#define PIN_MUXOUT_IN P12


#define PIN_SPIMASTER_MOSI_PUSHPULL_MODE P11_PUSHPULL_MODE
#define PIN_SPIMASTER_SCK_PUSHPULL_MODE P15_PUSHPULL_MODE
#define PIN_SPIMASTER_CS_PUSHPULL_MODE P10_PUSHPULL_MODE
#define PIN_MUXOUT_IN_INPUT_MODE P12_INPUT_MODE

#define PIN_ICPDA P16
#define PIN_UART0_RXD P07
#define PIN_UART0_TXD P06
//hardware immutable pins end

#define PIN_SPI_SLAVE_SCK P30
#define PIN_SPI_SLAVE_MOSI P03

#endif
#endif