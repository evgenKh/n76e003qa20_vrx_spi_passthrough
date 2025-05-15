#ifndef IAPMessage_H
#define IAPMessage_H

#if USE_NUVOTON
#include "numicro_8051.h"
#endif

#define IAP_HEX_STRING_MAX_SIZE 80
#define IAP_MESSAGE_SIZE (1+(1+1+2)+1+IAP_HEX_STRING_MAX_SIZE+6)
#define IAP_BOOTLOADER_VERSION 0

//Commands copied from isp_uart0.h except for CMD_RUN_LDROM added by me
#define CMD_START_SESSION    0xAE
#define CMD_RUN_APROM        0xAB
#define CMD_RUN_LDROM        0x69
#define CMD_NOP              0x70
#define CMD_GET_DEVICEID     0xB1
#define CMD_ERASE_ALL        0xA3
#define CMD_READ_CONFIG      0xA2
#define CMD_UPDATE_CONFIG    0xA1
#define CMD_UPDATE_APROM     0xA0

#if USE_NUVOTON
#define PACKED
#else
#define PACKED __attribute__ ((packed))

#endif

struct PACKED IAPSessionParams_t{
    uint8_t m_bootloaderVersion;
    uint8_t m_hexLinesCount;
    uint16_t m_programSizeBytes;
};

struct PACKED IAPMessage_t{
    uint8_t m_cmd;
    struct IAPSessionParams_t m_sessionParams;
    uint8_t m_thisHexLineNumber;//for CMD_UPDATE_APROM
    uint8_t m_intelHexString[IAP_HEX_STRING_MAX_SIZE];//should fit in 70characters
    uint8_t m_footer_6FF[6];//to make ADF3450 reject this and don't write anything IAP-related to its buffers.
};
#if USE_NUVOTON
typedef __xdata volatile struct IAPMessage_t IAPMessage;
typedef __xdata volatile struct IAPSessionParams_t IAPSessionParams;
#else
typedef struct IAPMessage_t IAPMessage;
typedef struct IAPSessionParams_t IAPSessionParams;
static_assert(sizeof(IAPMessage) == IAP_MESSAGE_SIZE);
#endif
#endif