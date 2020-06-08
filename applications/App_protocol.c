/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-19     CCRFIDZY       the first version
 */
/*
-  �ļ�����: protocol.c
-  �ļ�����: �Ա�˾Э���ж�λ�Ķ�������Э��Ľ���
-  �汾����: v1.1 2012.04
-  ��������: __Rangers
*/
#include "App_protocol.h"
#include "drv_flash.h"
#include "base64.h"

#define MODULE_BITS             7
#define ERROR_BITS              8

#define TRUE                    0
#define FALSE                   1

#define ERROR_BIT               1<<(MODULE_BITS+ERROR_BITS)
#define ERROR_BRGIN(_module_id) ((_module_id)<<ERROR_BITS)
#define ERROR_PROTOCOL_BASE ERROR_BRGIN(1)       //Э�鲿�ֻ�ַ
#define ERROR_T(_module_error)  (ERROR_BIT | (_module_error))
#define MODULE_ERROR(_error_t)  ((_error_t)&((1<<ERROR_BITS)-1))
#define MODULE_ID(_error_t)     (((_error_t)& ~(ERROR_BIT))>>ERROR_BITS)

/*����ͨ��Э�鲿�ִ����ʾ*/
#define ERROR_UC_JUDGE_HEADER           ERROR_PROTOCOL_BASE+0//����Э��֡ͷ����
#define ERROR_UC_JUDGE_CMD              ERROR_PROTOCOL_BASE+1//����Э��ָ�����
#define ERROR_UC_JUDGE_ADDR             ERROR_PROTOCOL_BASE+2//����Э���ַ����
#define ERROR_UC_JUDGE_CRC              ERROR_PROTOCOL_BASE+3//����Э��CRCУ�����
#define ERROR_UC_JUDGE_ENDER            ERROR_PROTOCOL_BASE+4//����Э��֡β����
#define ERROR_UC_JUDGE_DATALEN          ERROR_PROTOCOL_BASE+5//DATA�������ݳ��ȴ���
#define ERROR_UC_JUDGE_DATATYPE         ERROR_PROTOCOL_BASE+6//DATA�����������ʹ���

/*flash����*/
#define ERROR_FLASH_OPERATE_FLASE       ERROR_PROTOCOL_BASE+10//FLASH����ʧ��
#define ERROR_FLASH_OPERATE_CANNOT      ERROR_PROTOCOL_BASE+11//FLASH������ֹ

/*433M����ͨ��Э�鲿�ִ����ʾ*/
#define ERROR_RF_JUDGE_CMD              ERROR_PROTOCOL_BASE+20//RF����Э��ָ�����
#define ERROR_RF_JUDGE_SUM              ERROR_PROTOCOL_BASE+21//RF����Э��У��ʹ���
#define ERROR_RF_JUDGE_DATALEN          ERROR_PROTOCOL_BASE+22//RF����Э�����ݳ��ȴ���

/*��������ͨ��Э�鲿�ִ����ʾ*/
#define ERROR_BT_JUDGE_CMD              ERROR_PROTOCOL_BASE+23//��������Э��ָ�����
#define ERROR_BT_JUDGE_SUM              ERROR_PROTOCOL_BASE+24//��������Э��У��ʹ���
#define ERROR_BT_JUDGE_DATALEN          ERROR_PROTOCOL_BASE+25//��������Э�����ݳ��ȴ���


static void APP_Updata(uint8_t addr,uint8_t len,uint8_t *data);
static uint8_t APP_Getdata(uint8_t addr,uint8_t *data);
/*----------------------------------------------------------------------------*/
/*-----------------------------���ݽ���---------------------------------------*/
/*----------------------------------------------------------------------------*/

#define CRC_Polynomial 0xB7


uint8_t CRC8_Soft( uint8_t *data, uint16_t len )
{
  uint8_t crcve = 0xFF;
  uint16_t k;
  uint32_t j;

  for( k = 0; k < len; k++ )
  {
    crcve ^= data[k];
    for( j = 0; j < 8; j++ )
    {
      if( crcve & 0x80 )
      {
        crcve <<= 1;
        crcve ^= CRC_Polynomial;
      }
      else
      {
        crcve <<= 1;
      }
    }
  }

  return crcve;
}

/*
*��������: bt_ProDataLearning
*    ����: ������ǩ�ϴ��ĸ澯������������Ϣ
*    ������Դ: �����������Ա�ǩ������
*��ڲ���: pbuf-�������
*���ڲ���: FALSE���� �� TRUE
*/
uint16_t bt_ProDataLearning(const uint8_t* data_buf)
{
  uint8_t temp;
  uint8_t err = TRUE;
  uint8_t LEN;

  if(data_buf[0]==0xA5)
  {
      LEN = data_buf[1];
      temp =  CRC8_Soft((uint8_t*)(data_buf + 1),LEN - 1);
      if(temp != data_buf[LEN])
      {
        return ERROR_T(ERROR_BT_JUDGE_CMD);
      }
  }
  else
  {
    return ERROR_T(ERROR_BT_JUDGE_SUM);
  }
  return err;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*-----------------------------���ݴ��---------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
*��������: uc_UploadDataPackage �豸�ϱ����
*��ڲ���:
           data_buf-��������
*          packnumber-��Ϣ��ˮ��
*���ڲ���:  ����
*/
uint8_t uc_UploadDataPackage(uint8_t* data_buf,uint8_t packnumber)
{
	uint8_t data[20];
	unsigned char basedata[30];
	uint16_t crc16;
	size_t baselen;
  //֡ͷ
  *(data_buf+0) = Message_Head;
	//����
  data[0] = 0x00;
  data[1] = 0x0B;
	//Э��汾
	data[2] = 0xCC;
  //֡����
  data[3]  = 0x00;
  //�Ķ�������
  data[4]  = DEV_TYPE;
  //�Ķ���ID
  data[5] = (uint8_t)(NATIVE_ID>>24);
  data[6] = (uint8_t)(NATIVE_ID>>16);
  data[7] = (uint8_t)(NATIVE_ID>>8);
  data[8] = (uint8_t)NATIVE_ID;

  data[9]  = packnumber;
  //�汾�� ��4�ֽ�
  //Ӳ���汾
  data[10]  =  VERSION_Byte0;
  data[11]  = VERSION_Byte1;
  //�̼��汾
  data[12]  = VERSION_Byte2;
  data[13]  = VERSION_Byte3;
  //Crc16
	crc16  = CRC16((char *)data,14);
	data[14] = (crc16>>8)&0xFF;
	data[15] = crc16&0xFF;
	//base64
	if( 0 != mbedtls_base64_encode(basedata,sizeof(basedata),&baselen,data,16))
		return 0;
	rt_memcpy(data_buf+1,basedata,baselen);
	//֡β
  *(data_buf+baselen+1) = Message_End;
	return baselen+2;
}
/*
*��������: uc_HeartDataPackage �豸�����źŴ��, ���� ��ѯ
*��ڲ���:
           data_buf-��������
*          packnumber-������ˮ��
*���ڲ���: ���ݳ���
*/
uint16_t uc_HeartDataPackage(uint8_t* data_buf, uint8_t packnumber)
{
	uint8_t data[30];
	unsigned char basedata[30];
	uint16_t crc16;
	size_t baselen;
  //֡ͷ
  *(data_buf+0) = Message_Head;
  //����
  data[0] = 0x00;
  data[1] = 0x0F;
	//Э��汾
	data[2] = 0xCC;
  //֡����
  data[3] = 0x01;
  //�Ķ�������
  data[4] = DEV_TYPE;
  //�Ķ���ID
  data[5] = (uint8_t)(NATIVE_ID>>24);
  data[6] = (uint8_t)(NATIVE_ID>>16);
  data[7] = (uint8_t)(NATIVE_ID>>8);
  data[8] = (uint8_t)NATIVE_ID;

  data[9] = packnumber;
  //�汾�� ��4�ֽ�
  //Ӳ���汾
  data[10] =  VERSION_Byte0;
  data[11] = VERSION_Byte1;
  //�̼��汾
  data[12] = VERSION_Byte2;
  data[13] = VERSION_Byte3;
  //Unixʱ���
  data[14] = 0;
  data[15] = 0;
  data[16] = 0;
  data[17] = 0;
  //Crc16
	crc16  = CRC16((char *)data,18);
	data[18] = (crc16>>8)&0xFF;
	data[19] = crc16&0xFF;
	//base64
	if( 0 != mbedtls_base64_encode(basedata,sizeof(basedata),&baselen,data,20))
		return 0;
	rt_memcpy(data_buf+1,basedata,baselen);
  //֡β
  *(data_buf+baselen+1) = Message_End;
  return baselen+2;
}

/*
*��������: uc_ReSetPackage �Ķ�������Ӧ��֡
*��       ��: �Ķ�������Ӧ��֡
*��ڲ���: data_buf-�������
*      data_len-�������ݳ���
*���ڲ���: ��
*/
uint16_t uc_ReSetPackage(uint8_t* data_buf,uint8_t packnumber,uint8_t ConfigCode)
{
	uint16_t crc16;
	uint8_t data[30];
	unsigned char basedata[30];
	size_t baselen;
	//֡ͷ
	*(data_buf+0) = Message_Head;
	//����
	data[0] = 0x00;
	data[1] = 0x08;
	//Э��汾
	data[2] = 0xCC;
	//֡����
	data[3] = 0x04;
	//�Ķ�������
	data[4] = DEV_TYPE;
	//�Ķ���ID
	data[5] = (uint8_t)(NATIVE_ID>>24);
  data[6] = (uint8_t)(NATIVE_ID>>16);
  data[7] = (uint8_t)(NATIVE_ID>>8);
  data[8] = (uint8_t)NATIVE_ID;
	//���
	data[9] = packnumber;
	//������ַ
	data[10] = ConfigCode;
	//Crc16
	crc16  = CRC16((char *)data,11);
	data[11] = (crc16>>8)&0xFF;
	data[12] = crc16&0xFF;
	//base64
	if( 0 != mbedtls_base64_encode(basedata,sizeof(basedata),&baselen,data,13))
		return 0;
	rt_memcpy(data_buf+1,basedata,baselen);
	//֡β
	*(data_buf+baselen+1) = Message_End;
	
	return baselen+2;
}
/*
*��������: uc_ReCfgERRORPackage ���ô�����Ϣ����
*��       ��: ������Ϣ����
*��ڲ���: data_buf-�������
*      data_len-�������ݳ���
*���ڲ���: ��
*/
uint16_t uc_ReCfgERRORPackage(uint8_t* data_buf,uint8_t packnumber,uint8_t ErrorCode)
{
	uint8_t data[30];
	unsigned char basedata[30];
	size_t baselen;
	uint16_t crc16;
  //֡ͷ
  data_buf[0] = Message_Head;
  //����
  data[0] = 0x00;
  data[1] = 0x08;
	//Э��汾
	data[2] = 0xCC;
  //֡����
  data[3] = 0x05;
  //�Ķ�������
  data[4] = DEV_TYPE;
  //�Ķ���ID
  data[5] = (uint8_t)(NATIVE_ID>>24);
  data[6] = (uint8_t)(NATIVE_ID>>16);
  data[7] = (uint8_t)(NATIVE_ID>>8);
  data[8] = (uint8_t)NATIVE_ID;
  //���
  data[9] = packnumber;
  //�������
  data[10] = ErrorCode;
  //Crc16
	crc16  = CRC16((char *)data,11);
	data[11] = (crc16>>8)&0xFF;
	data[12] = crc16&0xFF;

	//base64
	if( 0 != mbedtls_base64_encode(basedata,sizeof(basedata),&baselen,data,13))
		return 0;
	rt_memcpy(data_buf+1,basedata,baselen);
	//֡β
	*(data_buf+baselen+1) = Message_End;
	return baselen+2;

}
/*
*��������: uc_ReReadStatPackage �Ķ�����ȡӦ��
*��       ��: �Ķ�����ȡӦ��
*��ڲ���: data_buf-�������
*      data_len-�������ݳ���
*���ڲ���: ��
*/
uint16_t uc_ReReadStatPackage( uint8_t* data_buf,uint8_t packnumber,uint8_t *Code)
{
	uint16_t crc16;
  uint8_t data[30];
	unsigned char basedata[30];
	size_t baselen;  
	int i=0;
	//֡ͷ
	*(data_buf+0) = Message_Head;
	//����
	data[0] = 0x00;
	data[1] = 11;
	//Э��汾
	data[2] = 0xCC;
	//֡����
	data[3] = 0x07;
	//�Ķ�������
	data[4] = DEV_TYPE;
	//�Ķ���ID
	data[5] = (uint8_t)(NATIVE_ID>>24);
  data[6] = (uint8_t)(NATIVE_ID>>16);
  data[7] = (uint8_t)(NATIVE_ID>>8);
  data[8] = (uint8_t)NATIVE_ID;

	data[9] = packnumber;
	//������ַ
	data[10] = Code[0];
	//��������
	data[11] = Code[1];
	for(i=0;i<Code[1];i++)
	{
		//����ֵ
		data[12+i] = Code[2+i];
	}
	
	//Crc16
	crc16  = CRC16((char *)data,13+i);
	data[13+i] = (crc16>>8)&0xFF;
	data[14+i] = crc16&0xFF;
	//base64
	if( 0 != mbedtls_base64_encode(basedata,sizeof(basedata),&baselen,data,15+i))
		return 0;
	rt_memcpy(data_buf+1,basedata,baselen);
	//֡β
	*(data_buf+baselen+1) = Message_End;
	return baselen+2;

}
/*
*��������: uc_ReReadErrorPackage �Ķ�����ȡ����Ӧ��
*��       ��: ,������Ϣ����
*��ڲ���: data_buf-�������
*      data_len-�������ݳ���
*���ڲ���: ��
*/
uint16_t uc_ReReadErrorPackage(uint8_t* data_buf,uint8_t packnumber,uint8_t ErrorCode)
{
	uint8_t data[30];
	unsigned char basedata[30];
	size_t baselen;  
	uint16_t crc16;
  //֡ͷ
  *(data_buf+0) = Message_Head;
  //����
  data[0] = 0x00;
  data[1] = 0x08;
	//Э��汾
	data[2] = 0xCC;
  //֡����
  data[3] = 0x08;
  //�Ķ�������
  data[4] = DEV_TYPE;
  //�Ķ���ID
  data[5] = (uint8_t)(NATIVE_ID>>24);
  data[6] = (uint8_t)(NATIVE_ID>>16);
  data[7] = (uint8_t)(NATIVE_ID>>8);
  data[8] = (uint8_t)NATIVE_ID;
  //���
  data[9] = packnumber;
  //�������
  data[10] = ErrorCode;
   //Crc16
	crc16  = CRC16((char *)data,11);
	data[11] = (crc16>>8)&0xFF;
	data[12] = crc16&0xFF;
	//base64
	if( 0 != mbedtls_base64_encode(basedata,sizeof(basedata),&baselen,data,13))
		return 0;
	rt_memcpy(data_buf+1,basedata,baselen);
	//֡β
	*(data_buf+baselen+1) = Message_End;
	return baselen+2;
}

/*
*��������: uc_TAGNormalDataPackage ��ǩ�豸�ϱ�
*    ����: PDAģ��usartͨ��,��ǩ��Ϣ���ݴ���ϴ����ֳֻ�
*    ������Դ: ��ǩ���������ĸ澯��������Ϣ
*��ڲ���: data_buf-�������
           redata_buf-��������
*          redata_len-�������ݳ���
*���ڲ���: 1 ��ʾ�û����ݰ�
           2 ��ʾ��λ���ݰ�
           3 ��ʾ��Ч���ݰ�
*/
uint8_t uc_TAGNormalDataPackage(uint8_t* data_buf ,const uint8_t* ble_buf,uint8_t packnumber)
{
	uint16_t crc16;
	uint8_t data[64];
	unsigned char basedata[64];
	size_t baselen;  
	//֡ͷ
	*(data_buf+0) = Message_Head;
	//Э��汾
	data[2] = 0xCC;
	//֡����
	data[3] = 0x80;
	//�Ķ�������
	data[4] = DEV_TYPE;
	//�Ķ���ID
	data[5] = (uint8_t)(NATIVE_ID>>24);
  data[6] = (uint8_t)(NATIVE_ID>>16);
  data[7] = (uint8_t)(NATIVE_ID>>8);
  data[8] = (uint8_t)NATIVE_ID;
	//��ǩ����
	data[9] = 0x10;
	//��ǩid
	data[10] = *(ble_buf+4);
	data[11] = *(ble_buf+5);
	data[12] = *(ble_buf+6);
	data[13] = *(ble_buf+7);
	//���
	data[14] = packnumber;
	switch(*(ble_buf+3))
	{
			case NET_IF_FT_TD:
			{
				//����
				data[0] = 0x00;
				data[1] = 0x1D;
				//��Ϣ����
				data[15] = 0x0D;
				//��Ϣ����
				data[16] = 0xB2;
				//��Ϣ����
				memcpy(data+17,&ble_buf[8],14);
				crc16  = CRC16((char *)data,31);
				data[31] = (crc16>>8)&0xFF;
				data[32] = crc16&0xFF;
				//base64
				if( 0 != mbedtls_base64_encode(basedata,sizeof(basedata),&baselen,data,33))
					return 0;
			}break;
			case NET_IF_FT_TOF:
			{
				//����
				data[0] = 0x00;
				data[1] = 0x15;
				//��Ϣ����
				data[15] = 0x07;
				//��Ϣ����
				data[16] = 0xA0;
				//��Ϣ����
				data[17] = ble_buf[14];  //����
				data[18] = ble_buf[15];	
				data[19] = 0;						//��ѹ
				data[20] = 0;						//���ģʽ
				data[21] = 0;						//״ֵ̬
				data[22] = 0;						//�汾��
				crc16  = CRC16((char *)data,23);
				data[23] = (crc16>>8)&0xFF;
				data[24] = crc16&0xFF;
				
				if( 0 != mbedtls_base64_encode(basedata,sizeof(basedata),&baselen,data,25))
					return 0;
			}break;
			case NET_IF_FT_TAG:
			{
				uint8_t i=0;
				//����
				data[0] = 0x00;
				data[1] = 0x13+ble_buf[1];
				//��Ϣ����
				data[15] = ble_buf[1];
				//��Ϣ����
				data[16] = 0x04;
				//��Ϣ����
				for(i=0;i<=ble_buf[1]-6;i++)
					data[17+i] = ble_buf[8+i];
				crc16  = CRC16((char *)data,17+i);
				data[17+i] = (crc16>>8)&0xFF;
				data[18+i] = crc16&0xFF;
				if( 0 != mbedtls_base64_encode(basedata,sizeof(basedata),&baselen,data,19+i))
					return 0;
			}break;
			default:return 0;
	}
	rt_memcpy(data_buf+1,basedata,baselen);
	//֡β
	*(data_buf+baselen+1) = Message_End;  
	return baselen+2;
}
/*
*��������: uc_ReSetTagPackage ��ǩ����Ӧ��
*    ����: Ӧ����������
*��ڲ���:  databuf  �����������
        id_buf   ����ID��Ϣ
        rebuf    ������������
        relen    �����������ݳ���
*���ڲ���: FALSE���� �� TRUE
*/
uint16_t uc_ReSetTagPackage(uint8_t* data_buf,uint8_t* Tag_ID,uint8_t packnumber,uint8_t Code)
{
	uint8_t data[30];
	unsigned char basedata[30];
	size_t baselen;  
	uint16_t crc16;
	//֡ͷ
	*(data_buf+0) = Message_Head;
	//����
	data[0] = 0x00;
	data[1] = 0x0E;
	//Э��汾
	data[2] = 0xCC;
	//֡����
	data[3] = 0x04;
	//�Ķ�������
	data[4] = DEV_TYPE;
	//�Ķ���ID
	data[5] = (uint8_t)(NATIVE_ID>>24);
  data[6] = (uint8_t)(NATIVE_ID>>16);
  data[7] = (uint8_t)(NATIVE_ID>>8);
  data[8] = (uint8_t)NATIVE_ID;
	//��ǩ����
	data[9] = Tag_ID[0];
	//��ǩid
	data[10] = Tag_ID[1];
	data[11] = Tag_ID[2];
	data[12] = Tag_ID[3];
	data[13] = Tag_ID[4];
	//���
	data[14] = packnumber;
	//������ַ
	data[15] = Code;
	//Crc16
	crc16  = CRC16((char *)data,16);
	data[16] = (crc16>>8)&0xFF;
	data[17] = crc16&0xFF;
	//base64
	if( 0 != mbedtls_base64_encode(basedata,sizeof(basedata),&baselen,data,18))
		return 0;
	rt_memcpy(data_buf+1,basedata,baselen);
	//֡β
	*(data_buf+baselen+1) = Message_End;
	return baselen+2;
}
/*
*��������: uc_ReTAGCfgERRORPackage ��ǩ���ô���Ӧ��֡
*��       ��: ,������Ϣ����
*��ڲ���: data_buf-�������
*      data_len-�������ݳ���
*���ڲ���: ��
*/
uint16_t uc_ReSetTagERRORPackage(uint8_t* data_buf,uint8_t* Tag_ID,uint8_t packnumber,uint8_t ErrorCode)
{
	uint8_t data[30];
	unsigned char basedata[30];
	size_t baselen;  
	uint16_t crc16;
	//֡ͷ
	*(data_buf+0) = Message_Head;
	//����
	data[0] = 0x00;
	data[1] = 0x0E;
	//Э��汾
	data[2] = 0xCC;
	//֡����
	data[3] = 0x04;
	//�Ķ�������
	data[4] = DEV_TYPE;
	//�Ķ���ID
	data[5] = (uint8_t)(NATIVE_ID>>24);
  data[6] = (uint8_t)(NATIVE_ID>>16);
  data[7] = (uint8_t)(NATIVE_ID>>8);
  data[8] = (uint8_t)NATIVE_ID;
	//��ǩ����
	data[9] = Tag_ID[0];
	//��ǩid
	data[10] = Tag_ID[1];
	data[11] = Tag_ID[2];
	data[12] = Tag_ID[3];
	data[13] = Tag_ID[4];
	//���
	data[14] = packnumber;
	//������ַ
	data[15] = ErrorCode;
	//Crc16
	crc16  = CRC16((char *)data,16);
	data[16] = (crc16>>8)&0xFF;
	data[17] = crc16&0xFF;
	//base64
	if( 0 != mbedtls_base64_encode(basedata,sizeof(basedata),&baselen,data,18))
		return 0;
	rt_memcpy(data_buf+1,basedata,baselen);
	//֡β
	*(data_buf+baselen+1) = Message_End;
	return baselen+2;
}
/*
*��������: uc_ReReadTagStatPackage��ǩ��ȡӦ��֡
*    ����: Ӧ���ȡ��Ϣ����
*��ڲ���: data_buf �����������
           id_buf   ����ID��Ϣ
           data     ��������
           data_len �������ݳ���
           rebuf    ������������
*          relen    �����������ݳ���
*���ڲ���: FALSE���� �� TRUE
*/
uint16_t uc_ReReadTagStatPackage( uint8_t* data_buf,uint8_t* Tag_ID,uint8_t packnumber,uint8_t *Code)
{
	uint16_t crc16;
	uint8_t data[30];
	unsigned char basedata[30];
	size_t baselen;  
	uint8_t i;
	//֡ͷ
	*(data_buf+0) = Message_Head;
	//Э��汾
	data[2] = 0xCC;
	//֡����
	data[3] = 0x04;
	//�Ķ�������
	data[4] = DEV_TYPE;
	//�Ķ���ID
	data[5] = (uint8_t)(NATIVE_ID>>24);
  data[6] = (uint8_t)(NATIVE_ID>>16);
  data[7] = (uint8_t)(NATIVE_ID>>8);
  data[8] = (uint8_t)NATIVE_ID;
	//��ǩ����
	data[9] = Tag_ID[0];
	//��ǩid
	data[10] = Tag_ID[1];
	data[11] = Tag_ID[2];
	data[12] = Tag_ID[3];
	data[13] = Tag_ID[4];
	//���
	data[14] = packnumber;
	//������ַ
	data[15] = Code[0];
	//��������
	data[16] = Code[1];
	for(i=0;i<Code[1];i++)
	{
		//����ֵ
		data[17+i] = Code[2+i];
	}
	//����
	data[0] = 0x00;
	data[1] = 0x0F+i;
	
	//Crc16
	crc16  = CRC16((char *)data,18+i);
	data[17+i] = (crc16>>8)&0xFF;
	data[18+i] = crc16&0xFF;
	//base64
	if( 0 != mbedtls_base64_encode(basedata,sizeof(basedata),&baselen,data,19+i))
		return 0;
	rt_memcpy(data_buf+1,basedata,baselen);
	//֡β
	*(data_buf+baselen+1) = Message_End;
	return baselen+2;
}
/*
*��������: uc_ReReadTagErrorPackage ��ǩ��ȡ����Ӧ��
*    ����: ��ǩ��ȡ��Ϣʧ������
*��ڲ���:          data_buf �����������
           id_buf   ����ID��Ϣ
           data     ��������
           data_len �������ݳ���
           rebuf    ������������
*          relen    �����������ݳ���
*���ڲ���: FALSE���� �� TRUE
*/
uint16_t uc_ReReadTagErrorPackage(uint8_t* data_buf,uint8_t packnumber,uint8_t ErrorCode)
{
	uint16_t crc16;
	uint8_t data[30];
	unsigned char basedata[30];
	size_t baselen;  
  //֡ͷ
  *(data_buf+0) = Message_Head;
  //����
  data[0] = 0x00;
  data[1] = 0x09;
	//Э��汾
	data[2] = 0xCC;
  //֡����
  data[3]  = 0x05;
  //�Ķ�������
  data[4] = DEV_TYPE;
  //�Ķ���ID
  data[5] = (uint8_t)(NATIVE_ID>>24);
  data[6] = (uint8_t)(NATIVE_ID>>16);
  data[7] = (uint8_t)(NATIVE_ID>>8);
  data[8] = (uint8_t)NATIVE_ID;
  //���
  data[9] = packnumber;
  //�������
  data[10] = ErrorCode;
   //Crc16
	crc16  = CRC16((char *)data,11);
	data[11] = (crc16>>8)&0xFF;
	data[12] = crc16&0xFF;
	//base64
	if( 0 != mbedtls_base64_encode(basedata,sizeof(basedata),&baselen,data,13))
		return 0;
	rt_memcpy(data_buf+1,basedata,baselen);
	//֡β
	*(data_buf+baselen+1) = Message_End;
	return baselen+2;
}


/*
*��������: uc_RDMsgPackage ���ݽ���
*    ����: ������̫�����ݰ�
*��ڲ���: data_buf �����������
           data     ��������
           data_len �������ݳ���
           rebuf    ������������
*          relen    �����������ݳ���
*���ڲ���: FALSE���� �� TRUE
*/
uint16_t uc_RDMsgPackage(uint8_t* data_buf,uint16_t data_len,uint8_t *status,uint8_t *TagID)
{
	uint8_t er= 0;
	uint8_t real_data[64];
	size_t real_len=64;		
	uint32_t ID=0;
	 /*У��ͷ*/
	if(*(data_buf+0) != Message_Head)
    return 0;
	 /*У��β*/
	data_len = data_len-1;
  if(*(data_buf+data_len) != Message_End)
    return ERROR_T(ERROR_UC_JUDGE_ENDER);
	if(0 != mbedtls_base64_decode(real_data,sizeof(real_data),&real_len,data_buf+1,(size_t)data_len-1))
		return 0;
	/*У���豸ID��*/
	ID = real_data[4];
	ID = real_data[5] + (ID<<8);
	ID = real_data[6] + (ID<<8);
	ID = real_data[7] + (ID<<8);
	
  if(ID != NATIVE_ID)
     return 0;
	
	switch(real_data[2])
	{
		case 0x02 : //Ӧ������֡
		{
			er = HEART;
		}break;
		case 0x03 : //��վ����֡
		{
			er = WRCONFIG;
			APP_Updata(real_data[9],real_data[10],(real_data+11));
			*status = real_data[9];
		}break;
		case 0x06 : //��վ��ȡ֡
		{
			er = RDCONFIG;
			*(status) =  real_data[9];
			*(status+1) = APP_Getdata(real_data[9],status+3);
		}break;
		case 0x81 : //��ǩ����֡
		{
			er = TAG_WRCONFIG;
			TagID[0] = real_data[9];
			TagID[1] = real_data[10];
			TagID[2] = real_data[11];
			TagID[3] = real_data[12];
			APP_Updata(real_data[14],real_data[15],(real_data+16));
			*status = real_data[14];
		}break;
		case 0x84 : //��ǩ��ȡ֡
		{
			er = TAG_RDCONFIG;
			TagID[0] = real_data[9];
			TagID[1] = real_data[10];
			TagID[2] = real_data[11];
			TagID[3] = real_data[12];
			*(status) =  real_data[14];
			*(status+1) = APP_Getdata(real_data[14],status+3);
		}break;
		default:
			return 0;
	}
	return er;
}


static void APP_Updata(uint8_t addr,uint8_t len,uint8_t *data)
{
	uint8_t i=0;
	uint8_t Config_Table[Config_Table_Len];
	for(i=0;i<len;i++)
	{
		Config_Table[addr+i] = *(data+i);
	}
	APP_flash_upload(Config_Table,ConfigTable_Add,Config_Table_Len);
}

void APP_flash_upload(uint8_t *table,uint8_t star,uint8_t len)
{
	uint8_t i;
	uint8_t FLASH_DATA[100];
	stm32_flash_read(Memery_Start_Address,FLASH_DATA,100);
	for(i=0;i<len;i++)
	{
		FLASH_DATA[star+i] = table[i];
	}
	stm32_flash_erase(Memery_Start_Address,100);
	stm32_flash_write(Memery_Start_Address,FLASH_DATA,100);
}


static uint8_t APP_Getdata(uint8_t addr,uint8_t *data)
{
	uint8_t Config_Table[Config_Table_Len];
	stm32_flash_read(Memery_Start_Address+ConfigTable_Add,Config_Table,Config_Table_Len);
	switch(addr)
	{
		case Config_HeartRate:
		{
			*(data) = Config_Table[addr];
			return 1;
		}
		case Config_ID:
		{
			*(data) = Config_Table[addr];
			*(data+1) = Config_Table[addr+1];
			*(data+2) = Config_Table[addr+2];
			*(data+3) = Config_Table[addr+3];
			return 4;
		}
		case Config_Mode:
		{
			*(data) = Config_Table[addr];
		}
		default:return 0;
	}
}



