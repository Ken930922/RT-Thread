/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-19     CCRFIDZY       the first version
 */
#ifndef MYAPP_APP_PROTOCOL_H_
#define MYAPP_APP_PROTOCOL_H_

#include <rtthread.h>
#include "App_Core.h"
#include "string.h"
#include "App_LF125K.h"
#include "App_Eth.h"


#define Message_Head          0x02
#define Message_End           0x03

#define VERSION_Byte0    0x02    //�汾��FV20
#define VERSION_Byte1    0x02
#define VERSION_Byte2    0x00
#define VERSION_Byte3    0x01

#define DEV_TYPE         0x12       //�豸����
#define NATIVEDEF_ID     0x00000005 //Ĭ�ϱ���ID

#define UWBMODE_DEF      0x01 //UWBĬ��ģʽ
#define UWBHEIGH_DEF     0x00 //UWBĬ�ϸ߶�


/*�豸  ����*/
#define HEART             1      //  �豸 ����
#define UPLOAD		        2      //  �豸 �ϱ�
#define SET_REPLY         3      //  �豸 ����Ӧ��
#define SET_ERROR         4      //  �豸 ���ô���
#define READ_REPLY        5      //  �豸 ��ȡӦ��
#define READ_ERROR        6      //  �豸 ��ȡ����
#define TAG_UPLOAD        7      //  ��ǩ �ϱ�

/*д��ǩ������������*/
#define TAG_WRCONFIG      20     //  д������Ϣ

/*����ǩ������������*/
#define TAG_RDCONFIG      30     //  ��������Ϣ

/*д�ն˲�����������*/
#define WRCONFIG          60     //  д������Ϣ

/*���ն˲�����������*/
#define RDCONFIG          80     //  ��������Ϣ

/*��ȡ������չָ��*/
#define RD_CFGMSG         100    //  ��ȡ�豸���ò�����Ϣ

#define RESETVERSION    ((0x01<<5)+0x15)
#define ENTERING        ((0x02<<5)+0x15)
#define READING         ((0x03<<5)+0x15)
#define SETTING         ((0x04<<5)+0x15)
#define EXITING         ((0x05<<5)+0x15)
#define NON             (0xFF)



#define Config_Table_Len   6

#define Config_ID					  0x00
#define Config_Mode				  0x04
#define Config_HeartRate    0x05
/*************************************************************/
/**************************���ݽ���***************************/
/*************************************************************/
/*���ݰ�����*/
uint16_t uc_RDMsgPackage(uint8_t* data_buf,uint16_t data_len,uint8_t *status,uint8_t *TagID);
/*���� ���ݽ���*/
uint16_t bt_ProDataLearning(const uint8_t* data_buf);
/*************************************************************/
/**************************���ݴ��***************************/
/*************************************************************/
/*�豸�ϱ�֡���*/
uint8_t uc_UploadDataPackage(uint8_t* data_buf,uint8_t packnumber);

/*�������ݴ��*/
uint16_t uc_HeartDataPackage(uint8_t* data_buf, uint8_t packnumber);

/*��������Ӧ����*/
uint16_t uc_ReSetPackage(uint8_t* data_buf,uint8_t packnumber,uint8_t ConfigCode);

/*����ʧ��Ӧ����*/
uint16_t uc_ReCfgERRORPackage(uint8_t* data_buf,uint8_t packnumber,uint8_t ErrorCode);

/*�豸״̬��ѯӦ����*/
uint16_t uc_ReReadStatPackage( uint8_t* data_buf,uint8_t packnumber,uint8_t *Code);
/*�豸״̬��ѯ����Ӧ����*/


/*��ǩ���ݴ��*/
uint16_t uc_ReReadErrorPackage(uint8_t* data_buf,uint8_t packnumber,uint8_t ErrorCode);

/*��ǩ��������Ӧ����*/
uint16_t uc_ReSetTagPackage(uint8_t* data_buf,uint8_t* Tag_ID, uint8_t packnumber,uint8_t Code);

/*��ǩ����ʧ��Ӧ����*/
uint16_t uc_ReSetTagERRORPackage(uint8_t* data_buf,uint8_t* Tag_ID,uint8_t packnumber,uint8_t ErrorCode);

/*��ǩ�豸״̬��ѯӦ����*/
uint16_t uc_ReReadTagStatPackage( uint8_t* data_buf,uint8_t* Tag_ID,uint8_t packnumber,uint8_t *Code);

/*��ǩ�豸״̬��ѯ����Ӧ����*/
uint16_t uc_ReReadTagErrorPackage(uint8_t* data_buf,uint8_t packnumber,uint8_t ErrorCode);

/*��λ��Ϣ���*/
uint16_t Locationprotoc(const uint8_t* buf ,uint8_t buflen ,uint8_t* re_buf ,
                     uint8_t rebuflen);

/*��ǩ�ϱ�*/
uint8_t uc_TAGNormalDataPackage(uint8_t* data_buf ,const uint8_t* ble_buf,uint8_t packnumber);

void APP_flash_upload(uint8_t *table,uint8_t star,uint8_t len);

#endif /* MYAPP_APP_PROTOCOL_H_ */
