#if USE_IAP
#include "IAPReceiver.h"
#include "numicro_8051.h"
#include "IAPMessage.h"
#include "ihex_parser.h"
#include <string.h>

#define trig_IAPGO           TA=0xAA;TA=0x55;IAPTRG|=0x01

#define PAGE_ERASE_AP        0x22
#define BYTE_READ_AP         0x00
#define BYTE_PROGRAM_AP      0x21
#define BYTE_READ_ID         0x0C
#define PAGE_ERASE_CONFIG    0xE2
#define BYTE_READ_CONFIG     0xC0
#define BYTE_PROGRAM_CONFIG  0xE1
#define READ_UID             0x04
#define PAGE_SIZE            128

IAPSessionParams iapSessionParams;
// pointer physically in internal ram pointing to object in external ram
IAPMessage* __data iapMessage;
__data uint8_t  iapCmd;

__xdata uint8_t* __data intelHexString;
__data uint8_t  intelHexStringSize;

__data volatile uint16_t flash_address;
__data uint8_t  lastHexLineProgrammed;
__xdata char g_iapLogTextBuf[IAP_LOG_TEXT_BUF_SIZE];
__xdata char itoaTextBuf[4];
BIT  iapSessionStarted;

void IAPReceiverInit(void)
{
	set_CHPCON_IAPEN;//enable iap
	iapMessage = NULL;
	iapCmd = 0x00;
	g_iapLogTextBuf[0] = '\0';
	lastHexLineProgrammed = 0;
	iapSessionStarted = FALSE;
}

void IAPReceiverReset(void)
{
	iapCmd = 0;
	iapMessage = NULL;
}

bool IAPReceiverParseMessage(IAPMessage* inIapMessage, uint8_t inIapMessageSize)
{
	iapCmd = 0;
	iapMessage = NULL;
	if(inIapMessageSize < IAP_MESSAGE_SIZE)
		return false;
		
	//Optimizer messes this check
	//if(iapMessage->m_footer_6FF[0] != 0xFF)
	//	puts("bad footr");
	//	return false;

	iapMessage = inIapMessage;
	iapCmd = iapMessage->m_cmd;

	switch (iapCmd)
	{
	case CMD_START_SESSION:
		iapSessionParams = iapMessage->m_sessionParams;
		iapSessionStarted = TRUE;
		return true;
	case CMD_ERASE_ALL:
	case CMD_RUN_APROM:
	case CMD_RUN_LDROM:
	case CMD_NOP:
		return true;
	case CMD_UPDATE_APROM:
		intelHexString = iapMessage->m_intelHexString;
		//intelHexStringSize = min(inIapMessageSize - 1 - 6, IAP_HEX_STRING_MAX_SIZE - 1);
		ihex_reset_state();
		bool parseResult = ihex_parser(intelHexString, IAP_HEX_STRING_MAX_SIZE);
		if(!parseResult)
		{
			IAPLogWrite("ihex_parser err");
			IAPLogWrite(intelHexString);
		}
		return parseResult;
	}
	IAPLogWrite("UnknCmd:");
	IAPLogWrite(IAPitoa(iapCmd));
	return false;
}

bool IAPUpdateAprom(void)
{
	uint8_t   vo8temp;
	uint16_t  vo16temp;
	uint8_t i = 0;
	flash_address = address_lo;
	for(i = 0; i < byte_count; i++)
	{
		IAPCN = BYTE_PROGRAM_AP;          //program byte
		IAPAL = flash_address&0xff;
		IAPAH = (flash_address>>8)&0xff;
		IAPFD=data[i];
	#ifdef isp_with_wdt
		set_IAPTRG_IAPGO_WDCLR;
	#else
		trig_IAPGO;
	#endif

		IAPCN = BYTE_READ_AP;              //program byte verify
		vo8temp = data[i];
		if(IAPFD!=vo8temp)
		while(1);                          
		if (CHPCON==0x43)              //if error flag set, program error stop ISP
		while(1);

		//g_totalchecksum += vo8temp;
		flash_address++;
		vo16temp = iapSessionParams.m_programSizeBytes;
		if(flash_address==vo16temp)
		{
			//g_programflag=0;
			//g_timer0Over =1;
			IAPLogWrite("hexEarly");
			goto END_2;
		}
	} 
	END_2:
	
	if(iapMessage->m_thisHexLineNumber > 0 && lastHexLineProgrammed != iapMessage->m_thisHexLineNumber - 1)
	{
		IAPLogWrite("hexLins skipd.");
	}
	lastHexLineProgrammed = iapMessage->m_thisHexLineNumber;
	IAPLogWrite("Flashd line#.");
	IAPLogWrite(IAPitoa(lastHexLineProgrammed));

	return true;
}

bool IAPReceiverExecuteMessage(void)
{
	switch (iapCmd)
	{
	case CMD_START_SESSION:
		IAPLogWrite("START_SES");
		return true;
	case CMD_ERASE_ALL:
	{
		IAPLogWrite("ERASE_ALL");
		if(!iapSessionStarted)
		{
			IAPLogWrite("NotInSession");
			return false;
		}
		set_IAPUEN_APUEN;
		IAPFD = 0xFF;          //Erase must set IAPFD = 0xFF
		IAPCN = PAGE_ERASE_AP;
		for(flash_address=0x0000;flash_address<iapSessionParams.m_programSizeBytes/PAGE_SIZE;flash_address++)
		{        
			IAPAL = LOBYTE(flash_address*PAGE_SIZE);
			IAPAH = HIBYTE(flash_address*PAGE_SIZE);
#ifdef isp_with_wdt
			set_IAPTRG_IAPGO_WDCLR;
#else
			trig_IAPGO;
#endif
		}
		IAPLogWrite("EraseOk");
		return true;
	}
	case CMD_RUN_APROM:
	IAPLogWrite("reboot to APROM");
		Software_Reset(BOOT_APROM);
		return true;
	case CMD_RUN_LDROM:
		Software_Reset(BOOT_LDROM);
		return true;
	case CMD_NOP:
		return true;
	case CMD_UPDATE_APROM:
		if(!iapSessionStarted)
		{
			IAPLogWrite("NotInSession");
			return false;
		}
		//puts("CMD_UPDATE_APROM");
		if(record_type!=0)
		{
			IAPLogWrite("All hexLines recv.");
			if(lastHexLineProgrammed != iapSessionParams.m_hexLinesCount - 1)
			{
				IAPLogWrite("Not all HexLins flashd");
			}

			return true;
		}
		else
		{
			return IAPUpdateAprom();
		}
	}
	return false;
}

void IAPLogWrite(const char* text)
{
	uint8_t lenDst = strlen(g_iapLogTextBuf);
	uint8_t lenSrc = strlen(text)+1;
	uint8_t copyLen = (lenSrc<lenDst) ? lenSrc : (IAP_LOG_TEXT_BUF_SIZE-lenDst);
	if(copyLen)
	{
		memcpy(g_iapLogTextBuf + lenDst, text, copyLen);
		//strncat(g_iapLogTextBuf, text, IAP_LOG_TEXT_BUF_SIZE-1-strlen(g_iapLogTextBuf));
	}
#if USE_UART0
	//puts(g_iapLogTextBuf);
#endif	
}
const char* IAPitoa(uint8_t val)
{
	itoaTextBuf[0] = '0'+(val/100);
	itoaTextBuf[1] = '0'+((val%100)/10);
	itoaTextBuf[2] = '0'+(val%10);
	itoaTextBuf[3] = '\0';
	return itoaTextBuf;
}
void IAPLogFlushToMem(char* dstMem, uint8_t dstMemSize)
{
#if 0
	char i=0;
	for(;i<dstMemSize-1;++i)
		dstMem[i]=i;
	dstMem[dstMemSize-1] = 0;
#else
	uint8_t len = strlen(g_iapLogTextBuf);
	if(len > dstMemSize-1)
	{
		len = dstMemSize-1;
		g_iapLogTextBuf[len] = '\0';
	}
	memcpy(dstMem, g_iapLogTextBuf, len+1);
	g_iapLogTextBuf[0] = '\0';

	uint8_t zeroes = dstMemSize - len;
	if(len < dstMemSize - 1)
	{
		memset(dstMem+len, 0, dstMemSize-len-1); 
	}
#if USE_UART0
	puts(dstMem);
#endif	
#endif
}
#endif