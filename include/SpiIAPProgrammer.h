#ifndef SpiIAPProgrammer_H
#define SpiIAPProgrammer_H

#include <Arduino.h>
#include "IAPMessage.h"

#if USE_SPI_CHIP_SELECT_MUX
#include "Enums.h"
class ChipSelectMuxDevice;
#endif // USE_SPI_CHIP_SELECT_MUX

class SPIClass;
class SpiIAPProgrammer
{
    public:
    bool Program(SPIClass* spiClass, int8_t spiChipSelectPin, const char* intelHexString, uint32_t spiBaudRate);
    void PowerCycleReceiver();
    void ChipSelect(int8_t spiChipSelectPin, bool selected);
    
    static const char* const k_hexStringsSimplePassthrough;

#if USE_SPI_CHIP_SELECT_MUX
    ChipSelectMuxDevice* m_chipSelectMuxDevice = nullptr;
    ChipSelectMuxId m_csMuxId = ChipSelectMuxId::CS_MUX_NONE;
#endif // USE_SPI_CHIP_SELECT_MUX

    private:
    IAPSessionParams CalcSessionParams(const char *intelHexString);

    void IAPSendMessage(SPIClass* spiClass, uint32_t baudrate, int8_t spiChipSelectPin,
        uint8_t cmd, 
        const IAPSessionParams& sessionPararms,
        uint8_t thisHexLineNumber, const char* intelHexStringLine);

    

};

#endif