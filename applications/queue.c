/*
-  文件名称: queue.c
-  文件描述: 用于定位阅读器中，对所有队列的处理，
-  版本日期: v1.1 2012.04.09
-  作者署名: __Rangers
*/

#include "queue.h"
#include "string.h"
/*****************************************************************************/
/*局部宏定义区*/
#define ETH_SEND_MAXLOW      2
#define ETH_SEND_LEN         ETH_SEND_MAXLOW*ETH_SEND_MAXLEN

/*局部宏定义——错误区*/
#define ERROR_QUEUE_BASE          ERROR_BRGIN(2)       //队列基址

#define ERROR_UCQUEUE_FULL        ERROR_QUEUE_BASE+0   //串口队列满
#define ERROR_UCQUEUE_NULL        ERROR_QUEUE_BASE+1   //串口队列空

#define ERROR_RFQUEUE_FULL        ERROR_QUEUE_BASE+2   //无线队列满
#define ERROR_RFQUEUE_NULL        ERROR_QUEUE_BASE+3   //无线队列空

#define ERROR_QUEUE_CHOICE        ERROR_QUEUE_BASE+4   //选择不存在

/*变量区*/
//static uint8_t uiRfRdCnt;
//static uint8_t uiRfWrCnt;
//static uint8_t uiRfBuf[RF_MAXLOW][RF_MAXLEN];

static uint8_t uiEthSendRdCnt;
static uint8_t uiEthSendWrCnt;
static uint8_t uiEthSendBuf[ETH_SEND_MAXLOW][ETH_SEND_MAXLEN];

/*****************************************************************************/

//队空
uint8_t Nulqueue(uint8_t choice)
{
  uint8_t error = 0;
  
  switch(choice)
  {
    case ETH_SEND_QUEUE:
    if(uiEthSendWrCnt == uiEthSendRdCnt)
    {
      error = 1;
    }
    break;
  default:
    error = 1;
    break;
  }
  return error;
}

//出队
void Dequeue(uint8_t choice , uint8_t* pbuf)
{
  switch(choice)
  {
  case ETH_SEND_QUEUE:
    memcpy(pbuf,uiEthSendBuf[uiEthSendRdCnt],ETH_SEND_MAXLEN);/*数据拷贝*/
    memset(uiEthSendBuf[uiEthSendRdCnt],0,ETH_SEND_MAXLEN);
    uiEthSendRdCnt++;
    uiEthSendRdCnt %= ETH_SEND_MAXLOW;
    break;
  default:
    break;
  }
}

//队满
uint8_t Fulqueue(uint8_t choice)
{
  uint8_t error = 0;
  
  switch(choice)
  {
		case ETH_SEND_QUEUE:
			if(uiEthSendWrCnt == uiEthSendRdCnt-1)//缓冲区满
			{
				error = 1;
			}
    break;
		default:
			error = 1;
		break;
  }
  return error;
}

//入队
void EnQueue(uint8_t choice , const uint8_t* pbuf)
{
  switch(choice)
  {
		case ETH_SEND_QUEUE:
			memcpy(uiEthSendBuf[uiEthSendWrCnt],pbuf,ETH_SEND_MAXLEN);/*数据拷贝*/
			uiEthSendWrCnt++;
			uiEthSendWrCnt %= ETH_SEND_MAXLOW;
			break;
		default:
			break;
  }
}

void ClearQueue(uint8_t choice)
{
  switch(choice)
  {
  case ETH_SEND_QUEUE:
    uiEthSendRdCnt = 0;
    uiEthSendWrCnt = 0;
    memset(uiEthSendBuf,0,ETH_SEND_LEN);
    break;
  default:
    break;
  }
}


