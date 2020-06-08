/*
-  �ļ�����: queue.c
-  �ļ�����: ���ڶ�λ�Ķ����У������ж��еĴ���
-  �汾����: v1.1 2012.04.09
-  ��������: __Rangers
*/

#include "queue.h"
#include "string.h"
/*****************************************************************************/
/*�ֲ��궨����*/
#define ETH_SEND_MAXLOW      2
#define ETH_SEND_LEN         ETH_SEND_MAXLOW*ETH_SEND_MAXLEN

/*�ֲ��궨�塪��������*/
#define ERROR_QUEUE_BASE          ERROR_BRGIN(2)       //���л�ַ

#define ERROR_UCQUEUE_FULL        ERROR_QUEUE_BASE+0   //���ڶ�����
#define ERROR_UCQUEUE_NULL        ERROR_QUEUE_BASE+1   //���ڶ��п�

#define ERROR_RFQUEUE_FULL        ERROR_QUEUE_BASE+2   //���߶�����
#define ERROR_RFQUEUE_NULL        ERROR_QUEUE_BASE+3   //���߶��п�

#define ERROR_QUEUE_CHOICE        ERROR_QUEUE_BASE+4   //ѡ�񲻴���

/*������*/
//static uint8_t uiRfRdCnt;
//static uint8_t uiRfWrCnt;
//static uint8_t uiRfBuf[RF_MAXLOW][RF_MAXLEN];

static uint8_t uiEthSendRdCnt;
static uint8_t uiEthSendWrCnt;
static uint8_t uiEthSendBuf[ETH_SEND_MAXLOW][ETH_SEND_MAXLEN];

/*****************************************************************************/

//�ӿ�
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

//����
void Dequeue(uint8_t choice , uint8_t* pbuf)
{
  switch(choice)
  {
  case ETH_SEND_QUEUE:
    memcpy(pbuf,uiEthSendBuf[uiEthSendRdCnt],ETH_SEND_MAXLEN);/*���ݿ���*/
    memset(uiEthSendBuf[uiEthSendRdCnt],0,ETH_SEND_MAXLEN);
    uiEthSendRdCnt++;
    uiEthSendRdCnt %= ETH_SEND_MAXLOW;
    break;
  default:
    break;
  }
}

//����
uint8_t Fulqueue(uint8_t choice)
{
  uint8_t error = 0;
  
  switch(choice)
  {
		case ETH_SEND_QUEUE:
			if(uiEthSendWrCnt == uiEthSendRdCnt-1)//��������
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

//���
void EnQueue(uint8_t choice , const uint8_t* pbuf)
{
  switch(choice)
  {
		case ETH_SEND_QUEUE:
			memcpy(uiEthSendBuf[uiEthSendWrCnt],pbuf,ETH_SEND_MAXLEN);/*���ݿ���*/
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


