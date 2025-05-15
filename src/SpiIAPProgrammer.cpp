#include "SpiIAPProgrammer.h"
#include "IAPMessage.h"
#include <SPI.h>
#include <string.h>
#include <string>

#if USE_SPI_CHIP_SELECT_MUX
# include "ChipSelectMuxDevice.h"
#endif // USE_SPI_CHIP_SELECT_MUX

#define INVALID_HEX_CHAR        'x'
uint8_t HexToDec(uint8_t h)
{
    if (h >= '0' && h <= '9')
        return h - '0';
    else if (h >= 'A' && h <= 'F')
        return h - 'A' + 0xA;
    else if (h >= 'a' && h <= 'z')
        return h - 'a' + 0xA;
    else
        return INVALID_HEX_CHAR;
}

void SpiIAPProgrammer::IAPSendMessage(SPIClass* spiClass, uint32_t baudrate, int8_t spiChipSelectPin,
  uint8_t cmd, 
  const IAPSessionParams& sessionPararms,
  uint8_t thisHexLineNumber, const char* intelHexStringLine)
{
  static IAPMessage msg;
  memset(&msg, 0, IAP_MESSAGE_SIZE);
  memset(msg.m_footer_6FF, 0xFF, 6);
  msg.m_cmd = cmd;
  msg.m_sessionParams = sessionPararms;
  msg.m_thisHexLineNumber = thisHexLineNumber;

  uint32_t len = 0;
  if(intelHexStringLine)
  {
    len = strlen(intelHexStringLine);
    if(len > IAP_HEX_STRING_MAX_SIZE -1)
    {
      printf("hex string too long(%d)", len);
      return;
    }
    memcpy(msg.m_intelHexString, intelHexStringLine, len);
  }
  
  char spiResponseBuf[IAP_MESSAGE_SIZE];

  printf("SPI sending: msg %X %d intelHex lines.\n", cmd, len);
  spiClass->beginTransaction(SPISettings(baudrate, MSBFIRST, SPI_MODE2));
  
  ChipSelect(spiChipSelectPin, true);
  
  spiClass->transferBytes((uint8_t*)&msg, (uint8_t*)spiResponseBuf, IAP_MESSAGE_SIZE);
  //spiClass.writeBytes((uint8_t*)&msg, IAP_MESSAGE_SIZE);
  spiClass->endTransaction();

  ChipSelect(spiChipSelectPin, false);

  spiResponseBuf[IAP_MESSAGE_SIZE-1] = '\0';
  if(spiResponseBuf[0] != 255u)
  {
    printf("SPI respond: %s\n", spiResponseBuf);
    //printf("SPI respond hex: %X %X %X %X %X %X\n", spiResponseBuf[0], spiResponseBuf[1], spiResponseBuf[2], spiResponseBuf[3], spiResponseBuf[4], spiResponseBuf[5]);
  }

  delay(200);
}

IAPSessionParams SpiIAPProgrammer::CalcSessionParams(const char *intelHexString)
{
  IAPSessionParams result{IAP_BOOTLOADER_VERSION, 0, 0};
  if(!intelHexString || !strlen(intelHexString))
  {
    //return result;
  }
  std::string hexStringCopy(intelHexString);

  uint16_t minAddres = 65535;
  uint16_t maxAddres = 0;

  char* line = strtok(&hexStringCopy[0], "\r\n");
  do
  {
     if(!line || strlen(line) < 9) continue;
     if(line[0] != ':') continue;
     if(line[7]=='0' && line[8]=='1') break;//end-of-data line
     if(line[7]=='0' && line[8]=='0')//line contains data
     {
       result.m_hexLinesCount++;
       uint16_t startAddr = 0;
       startAddr = (startAddr<<4) | HexToDec(line[3]);
       startAddr = (startAddr<<4) | HexToDec(line[4]);
       startAddr = (startAddr<<4) | HexToDec(line[5]);
       startAddr = (startAddr<<4) | HexToDec(line[6]);
       
       uint8_t byteCount = 0;
       byteCount = (byteCount<<4) | HexToDec(line[1]);
       byteCount = (byteCount<<4) | HexToDec(line[2]);
       uint16_t endAddr = startAddr + byteCount;

       minAddres = min(minAddres, startAddr);
       maxAddres = max(maxAddres, endAddr);
     }

  } while(line = strtok(nullptr, "\r\n"));

  if(minAddres > 0)
  {
    //TODO: not supported non zero start addressed!
    minAddres = 0;
  }
  result.m_programSizeBytes = maxAddres - minAddres;
  //printf("minAddr: %X, maxAddr: %X\n", minAddres, maxAddres);
  return result;
}

void SpiIAPProgrammer::PowerCycleReceiver()
{
}

void SpiIAPProgrammer::ChipSelect(int8_t spiChipSelectPin, bool selected)
{
  if(spiChipSelectPin != -1)
  {
    delay(1);
    digitalWrite(spiChipSelectPin, !selected);
    delay(1);
  }
  
#if USE_SPI_CHIP_SELECT_MUX
  else if(m_chipSelectMuxDevice)
  {
    delay(1);
    if(selected)
      m_chipSelectMuxDevice->SetSelectedCS(m_csMuxId);
    else
      m_chipSelectMuxDevice->SetSelectedCS(ChipSelectMuxId::CS_MUX_NONE);
    delay(1);
  }
#endif // USE_SPI_CHIP_SELECT_MUX
}

bool SpiIAPProgrammer::Program(SPIClass *spiClass, int8_t spiCSPin, const char *intelHexString, uint32_t spiBaudRate)
{
  IAPSessionParams sessionParams = CalcSessionParams(intelHexString);
  //Use baudrate up to 128000 if you don't need SPI respond from bootloader. 20000 is reliable
  //If you need text response, 16000 seems ok, but use 10000 should be more reliable
  const uint32_t baudrate = 10000;
  IAPSendMessage(spiClass, baudrate, spiCSPin, CMD_START_SESSION, sessionParams, 0xFF, nullptr);
  IAPSendMessage(spiClass, baudrate, spiCSPin, CMD_START_SESSION, sessionParams, 0xFF, nullptr);
  IAPSendMessage(spiClass, baudrate, spiCSPin, CMD_START_SESSION, sessionParams, 0xFF, nullptr);
  IAPSendMessage(spiClass, baudrate, spiCSPin, CMD_START_SESSION, sessionParams, 0xFF, nullptr);
  IAPSendMessage(spiClass, baudrate, spiCSPin, CMD_START_SESSION, sessionParams, 0xFF, nullptr);
  IAPSendMessage(spiClass, baudrate, spiCSPin, CMD_ERASE_ALL, sessionParams, 0xFF, nullptr);
  delay(1000);

  std::string hexStringCopy(intelHexString);

  uint8_t lineNum = 0;
  char* line = strtok(&hexStringCopy[0], "\r\n");
  do
  {
     if(!line) continue;
     printf("strtok: %s", line);
     IAPSendMessage(spiClass, baudrate, spiCSPin, 
                    CMD_UPDATE_APROM, sessionParams, lineNum, line);
      
    lineNum++;
  }while(line = strtok(nullptr, "\r\n"));

  IAPSendMessage(spiClass, baudrate, spiCSPin, CMD_NOP, sessionParams, 0xFF, nullptr);//to get last response
  IAPSendMessage(spiClass, baudrate, spiCSPin, CMD_RUN_APROM, sessionParams, 0xFF, nullptr);

  return true;
}

const char* const SpiIAPProgrammer::k_hexStringsSimplePassthrough =
":16006200E493A3E493F58222AF8243078075C7AA75C7558F9F229C\n"
":03000000020006F5\n"
":03005F0002000399\n"
":0300030002007880\n"
":2000780053B3FD43B40253B3DF43B42053B3FE43B40143B30453B4FB43AC0153ADFEA2AF39\n"
":200098009200C2AF75C7AA75C755759101A20092AF43AC01A2AF9200C2AF75C7AA75C75529\n"
":2000B800759100A20092AF43B11053B2EFA2AF9200C2AF75C7AA75C755759101A20092AF92\n"
":2000D80043B110A2AF9200C2AF75C7AA75C755759100A20092AF43B10853B2F7A2AF920075\n"
":2000F800C2AF75C7AA75C755759101A20092AF43B108A2AF9200C2AF75C7AA75C755759149\n"
":2001180000A20092AFA2849202A28492015003200210A2839291A2019290A2B09295A201C3\n"
":080138009202A201929080E105\n"
":06003500E478FFF6D8FD9F\n"
":200013007900E94400601B7A00900144780175A000E493F2A308B8000205A0D9F4DAF2754E\n"
":02003300A0FF2C\n"
":20003B007800E84400600A790175A000E4F309D8FC7800E84400600C7900900001E4F0A3C3\n"
":04005B00D8FCD9FAFA\n"
":0D000600758120120140E5826003020003B5\n"
":0401400075820022A2\n"
":00000001FF\n";

