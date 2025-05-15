#ifndef IAPReceiver_H
#define IAPReceiver_H

#include "numicro_8051.h"
#include "IAPMessage.h"
#include <stdbool.h>

#define IAP_LOG_TEXT_BUF_SIZE 50

void IAPReceiverInit(void);
void IAPReceiverReset(void);

bool IAPReceiverParseMessage(IAPMessage* inIapMessage, uint8_t inIapMessageSize);
bool IAPReceiverExecuteMessage(void);
void IAPLogWrite(const char*);
void IAPLogFlushToMem(char* dstMem, uint8_t dstMemSize);
const char* IAPitoa(uint8_t val);

#endif