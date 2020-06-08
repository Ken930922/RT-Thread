#ifndef __QUEUE_H
#define __QUEUE_H

#include "App_Core.h"

#define ETH_SEND_QUEUE       0

#define ETH_SEND_MAXLEN      32

uint8_t Fulqueue(uint8_t choice);
uint8_t Nulqueue(uint8_t choice);
void Dequeue(uint8_t choice , uint8_t* pbuf);
void EnQueue(uint8_t choice , const uint8_t* pbuf);
void ClearQueue(uint8_t choice);

#endif


