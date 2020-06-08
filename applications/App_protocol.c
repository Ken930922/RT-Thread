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
-  文件名称: protocol.c
-  文件描述: 对本司协议中定位阅读器所有协议的解析
-  版本日期: v1.1 2012.04
-  作者署名: __Rangers
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
#define ERROR_PROTOCOL_BASE ERROR_BRGIN(1)       //协议部分基址
#define ERROR_T(_module_error)  (ERROR_BIT | (_module_error))
#define MODULE_ERROR(_error_t)  ((_error_t)&((1<<ERROR_BITS)-1))
#define MODULE_ID(_error_t)     (((_error_t)& ~(ERROR_BIT))>>ERROR_BITS)

/*串口通信协议部分错误标示*/
#define ERROR_UC_JUDGE_HEADER           ERROR_PROTOCOL_BASE+0//串口协议帧头错误
#define ERROR_UC_JUDGE_CMD              ERROR_PROTOCOL_BASE+1//串口协议指令错误
#define ERROR_UC_JUDGE_ADDR             ERROR_PROTOCOL_BASE+2//串口协议地址错误
#define ERROR_UC_JUDGE_CRC              ERROR_PROTOCOL_BASE+3//串口协议CRC校验错误
#define ERROR_UC_JUDGE_ENDER            ERROR_PROTOCOL_BASE+4//串口协议帧尾错误
#define ERROR_UC_JUDGE_DATALEN          ERROR_PROTOCOL_BASE+5//DATA部分数据长度错误
#define ERROR_UC_JUDGE_DATATYPE         ERROR_PROTOCOL_BASE+6//DATA部分数据类型错误

/*flash操作*/
#define ERROR_FLASH_OPERATE_FLASE       ERROR_PROTOCOL_BASE+10//FLASH操作失败
#define ERROR_FLASH_OPERATE_CANNOT      ERROR_PROTOCOL_BASE+11//FLASH操作禁止

/*433M无线通信协议部分错误标示*/
#define ERROR_RF_JUDGE_CMD              ERROR_PROTOCOL_BASE+20//RF无线协议指令错误
#define ERROR_RF_JUDGE_SUM              ERROR_PROTOCOL_BASE+21//RF无线协议校验和错误
#define ERROR_RF_JUDGE_DATALEN          ERROR_PROTOCOL_BASE+22//RF无线协议数据长度错误

/*蓝牙无线通信协议部分错误标示*/
#define ERROR_BT_JUDGE_CMD              ERROR_PROTOCOL_BASE+23//蓝牙无线协议指令错误
#define ERROR_BT_JUDGE_SUM              ERROR_PROTOCOL_BASE+24//蓝牙无线协议校验和错误
#define ERROR_BT_JUDGE_DATALEN          ERROR_PROTOCOL_BASE+25//蓝牙无线协议数据长度错误


static void APP_Updata(uint8_t addr,uint8_t len,uint8_t *data);
static uint8_t APP_Getdata(uint8_t addr,uint8_t *data);
/*----------------------------------------------------------------------------*/
/*-----------------------------数据解析---------------------------------------*/
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
*功能名称: bt_ProDataLearning
*    功能: 解析标签上传的告警心跳等数据信息
*    数据来源: 蓝牙接收来自标签的数据
*入口参数: pbuf-入口数据
*出口参数: FALSE代号 或 TRUE
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
/*-----------------------------数据打包---------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
*功能名称: uc_UploadDataPackage 设备上报打包
*入口参数:
           data_buf-出口数据
*          packnumber-消息流水号
*出口参数:  长度
*/
uint8_t uc_UploadDataPackage(uint8_t* data_buf,uint8_t packnumber)
{
	uint8_t data[20];
	unsigned char basedata[30];
	uint16_t crc16;
	size_t baselen;
  //帧头
  *(data_buf+0) = Message_Head;
	//长度
  data[0] = 0x00;
  data[1] = 0x0B;
	//协议版本
	data[2] = 0xCC;
  //帧类型
  data[3]  = 0x00;
  //阅读器类型
  data[4]  = DEV_TYPE;
  //阅读器ID
  data[5] = (uint8_t)(NATIVE_ID>>24);
  data[6] = (uint8_t)(NATIVE_ID>>16);
  data[7] = (uint8_t)(NATIVE_ID>>8);
  data[8] = (uint8_t)NATIVE_ID;

  data[9]  = packnumber;
  //版本号 共4字节
  //硬件版本
  data[10]  =  VERSION_Byte0;
  data[11]  = VERSION_Byte1;
  //固件版本
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
	//帧尾
  *(data_buf+baselen+1) = Message_End;
	return baselen+2;
}
/*
*功能名称: uc_HeartDataPackage 设备心跳信号打包, 保活 查询
*入口参数:
           data_buf-出口数据
*          packnumber-数据流水号
*出口参数: 数据长度
*/
uint16_t uc_HeartDataPackage(uint8_t* data_buf, uint8_t packnumber)
{
	uint8_t data[30];
	unsigned char basedata[30];
	uint16_t crc16;
	size_t baselen;
  //帧头
  *(data_buf+0) = Message_Head;
  //长度
  data[0] = 0x00;
  data[1] = 0x0F;
	//协议版本
	data[2] = 0xCC;
  //帧类型
  data[3] = 0x01;
  //阅读器类型
  data[4] = DEV_TYPE;
  //阅读器ID
  data[5] = (uint8_t)(NATIVE_ID>>24);
  data[6] = (uint8_t)(NATIVE_ID>>16);
  data[7] = (uint8_t)(NATIVE_ID>>8);
  data[8] = (uint8_t)NATIVE_ID;

  data[9] = packnumber;
  //版本号 共4字节
  //硬件版本
  data[10] =  VERSION_Byte0;
  data[11] = VERSION_Byte1;
  //固件版本
  data[12] = VERSION_Byte2;
  data[13] = VERSION_Byte3;
  //Unix时间戳
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
  //帧尾
  *(data_buf+baselen+1) = Message_End;
  return baselen+2;
}

/*
*功能名称: uc_ReSetPackage 阅读器设置应答帧
*功       能: 阅读器设置应答帧
*入口参数: data_buf-入口数据
*      data_len-出口数据长度
*出口参数: 无
*/
uint16_t uc_ReSetPackage(uint8_t* data_buf,uint8_t packnumber,uint8_t ConfigCode)
{
	uint16_t crc16;
	uint8_t data[30];
	unsigned char basedata[30];
	size_t baselen;
	//帧头
	*(data_buf+0) = Message_Head;
	//长度
	data[0] = 0x00;
	data[1] = 0x08;
	//协议版本
	data[2] = 0xCC;
	//帧类型
	data[3] = 0x04;
	//阅读器类型
	data[4] = DEV_TYPE;
	//阅读器ID
	data[5] = (uint8_t)(NATIVE_ID>>24);
  data[6] = (uint8_t)(NATIVE_ID>>16);
  data[7] = (uint8_t)(NATIVE_ID>>8);
  data[8] = (uint8_t)NATIVE_ID;
	//序号
	data[9] = packnumber;
	//参数地址
	data[10] = ConfigCode;
	//Crc16
	crc16  = CRC16((char *)data,11);
	data[11] = (crc16>>8)&0xFF;
	data[12] = crc16&0xFF;
	//base64
	if( 0 != mbedtls_base64_encode(basedata,sizeof(basedata),&baselen,data,13))
		return 0;
	rt_memcpy(data_buf+1,basedata,baselen);
	//帧尾
	*(data_buf+baselen+1) = Message_End;
	
	return baselen+2;
}
/*
*功能名称: uc_ReCfgERRORPackage 设置错误信息反馈
*功       能: 错误信息反馈
*入口参数: data_buf-入口数据
*      data_len-出口数据长度
*出口参数: 无
*/
uint16_t uc_ReCfgERRORPackage(uint8_t* data_buf,uint8_t packnumber,uint8_t ErrorCode)
{
	uint8_t data[30];
	unsigned char basedata[30];
	size_t baselen;
	uint16_t crc16;
  //帧头
  data_buf[0] = Message_Head;
  //长度
  data[0] = 0x00;
  data[1] = 0x08;
	//协议版本
	data[2] = 0xCC;
  //帧类型
  data[3] = 0x05;
  //阅读器类型
  data[4] = DEV_TYPE;
  //阅读器ID
  data[5] = (uint8_t)(NATIVE_ID>>24);
  data[6] = (uint8_t)(NATIVE_ID>>16);
  data[7] = (uint8_t)(NATIVE_ID>>8);
  data[8] = (uint8_t)NATIVE_ID;
  //序号
  data[9] = packnumber;
  //错误代号
  data[10] = ErrorCode;
  //Crc16
	crc16  = CRC16((char *)data,11);
	data[11] = (crc16>>8)&0xFF;
	data[12] = crc16&0xFF;

	//base64
	if( 0 != mbedtls_base64_encode(basedata,sizeof(basedata),&baselen,data,13))
		return 0;
	rt_memcpy(data_buf+1,basedata,baselen);
	//帧尾
	*(data_buf+baselen+1) = Message_End;
	return baselen+2;

}
/*
*功能名称: uc_ReReadStatPackage 阅读器读取应答
*功       能: 阅读器读取应答
*入口参数: data_buf-入口数据
*      data_len-出口数据长度
*出口参数: 无
*/
uint16_t uc_ReReadStatPackage( uint8_t* data_buf,uint8_t packnumber,uint8_t *Code)
{
	uint16_t crc16;
  uint8_t data[30];
	unsigned char basedata[30];
	size_t baselen;  
	int i=0;
	//帧头
	*(data_buf+0) = Message_Head;
	//长度
	data[0] = 0x00;
	data[1] = 11;
	//协议版本
	data[2] = 0xCC;
	//帧类型
	data[3] = 0x07;
	//阅读器类型
	data[4] = DEV_TYPE;
	//阅读器ID
	data[5] = (uint8_t)(NATIVE_ID>>24);
  data[6] = (uint8_t)(NATIVE_ID>>16);
  data[7] = (uint8_t)(NATIVE_ID>>8);
  data[8] = (uint8_t)NATIVE_ID;

	data[9] = packnumber;
	//参数地址
	data[10] = Code[0];
	//参数长度
	data[11] = Code[1];
	for(i=0;i<Code[1];i++)
	{
		//参数值
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
	//帧尾
	*(data_buf+baselen+1) = Message_End;
	return baselen+2;

}
/*
*功能名称: uc_ReReadErrorPackage 阅读器读取错误应答
*功       能: ,错误信息反馈
*入口参数: data_buf-入口数据
*      data_len-出口数据长度
*出口参数: 无
*/
uint16_t uc_ReReadErrorPackage(uint8_t* data_buf,uint8_t packnumber,uint8_t ErrorCode)
{
	uint8_t data[30];
	unsigned char basedata[30];
	size_t baselen;  
	uint16_t crc16;
  //帧头
  *(data_buf+0) = Message_Head;
  //长度
  data[0] = 0x00;
  data[1] = 0x08;
	//协议版本
	data[2] = 0xCC;
  //帧类型
  data[3] = 0x08;
  //阅读器类型
  data[4] = DEV_TYPE;
  //阅读器ID
  data[5] = (uint8_t)(NATIVE_ID>>24);
  data[6] = (uint8_t)(NATIVE_ID>>16);
  data[7] = (uint8_t)(NATIVE_ID>>8);
  data[8] = (uint8_t)NATIVE_ID;
  //序号
  data[9] = packnumber;
  //错误代号
  data[10] = ErrorCode;
   //Crc16
	crc16  = CRC16((char *)data,11);
	data[11] = (crc16>>8)&0xFF;
	data[12] = crc16&0xFF;
	//base64
	if( 0 != mbedtls_base64_encode(basedata,sizeof(basedata),&baselen,data,13))
		return 0;
	rt_memcpy(data_buf+1,basedata,baselen);
	//帧尾
	*(data_buf+baselen+1) = Message_End;
	return baselen+2;
}

/*
*功能名称: uc_TAGNormalDataPackage 标签设备上报
*    功能: PDA模块usart通信,标签信息数据打包上传给手持机
*    数据来源: 标签发送上来的告警心跳等信息
*入口参数: data_buf-入口数据
           redata_buf-出口数据
*          redata_len-出口数据长度
*出口参数: 1 表示用户数据包
           2 表示定位数据包
           3 表示无效数据包
*/
uint8_t uc_TAGNormalDataPackage(uint8_t* data_buf ,const uint8_t* ble_buf,uint8_t packnumber)
{
	uint16_t crc16;
	uint8_t data[64];
	unsigned char basedata[64];
	size_t baselen;  
	//帧头
	*(data_buf+0) = Message_Head;
	//协议版本
	data[2] = 0xCC;
	//帧类型
	data[3] = 0x80;
	//阅读器类型
	data[4] = DEV_TYPE;
	//阅读器ID
	data[5] = (uint8_t)(NATIVE_ID>>24);
  data[6] = (uint8_t)(NATIVE_ID>>16);
  data[7] = (uint8_t)(NATIVE_ID>>8);
  data[8] = (uint8_t)NATIVE_ID;
	//标签类型
	data[9] = 0x10;
	//标签id
	data[10] = *(ble_buf+4);
	data[11] = *(ble_buf+5);
	data[12] = *(ble_buf+6);
	data[13] = *(ble_buf+7);
	//序号
	data[14] = packnumber;
	switch(*(ble_buf+3))
	{
			case NET_IF_FT_TD:
			{
				//长度
				data[0] = 0x00;
				data[1] = 0x1D;
				//消息长度
				data[15] = 0x0D;
				//消息类型
				data[16] = 0xB2;
				//消息内容
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
				//长度
				data[0] = 0x00;
				data[1] = 0x15;
				//消息长度
				data[15] = 0x07;
				//消息类型
				data[16] = 0xA0;
				//消息内容
				data[17] = ble_buf[14];  //距离
				data[18] = ble_buf[15];	
				data[19] = 0;						//电压
				data[20] = 0;						//测距模式
				data[21] = 0;						//状态值
				data[22] = 0;						//版本号
				crc16  = CRC16((char *)data,23);
				data[23] = (crc16>>8)&0xFF;
				data[24] = crc16&0xFF;
				
				if( 0 != mbedtls_base64_encode(basedata,sizeof(basedata),&baselen,data,25))
					return 0;
			}break;
			case NET_IF_FT_TAG:
			{
				uint8_t i=0;
				//长度
				data[0] = 0x00;
				data[1] = 0x13+ble_buf[1];
				//消息长度
				data[15] = ble_buf[1];
				//消息类型
				data[16] = 0x04;
				//消息内容
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
	//帧尾
	*(data_buf+baselen+1) = Message_End;  
	return baselen+2;
}
/*
*功能名称: uc_ReSetTagPackage 标签设置应答
*    功能: 应答配置命令
*入口参数:  databuf  ——入口数据
        id_buf   ——ID信息
        rebuf    ——出口数据
        relen    ——出口数据长度
*出口参数: FALSE代号 或 TRUE
*/
uint16_t uc_ReSetTagPackage(uint8_t* data_buf,uint8_t* Tag_ID,uint8_t packnumber,uint8_t Code)
{
	uint8_t data[30];
	unsigned char basedata[30];
	size_t baselen;  
	uint16_t crc16;
	//帧头
	*(data_buf+0) = Message_Head;
	//长度
	data[0] = 0x00;
	data[1] = 0x0E;
	//协议版本
	data[2] = 0xCC;
	//帧类型
	data[3] = 0x04;
	//阅读器类型
	data[4] = DEV_TYPE;
	//阅读器ID
	data[5] = (uint8_t)(NATIVE_ID>>24);
  data[6] = (uint8_t)(NATIVE_ID>>16);
  data[7] = (uint8_t)(NATIVE_ID>>8);
  data[8] = (uint8_t)NATIVE_ID;
	//标签类型
	data[9] = Tag_ID[0];
	//标签id
	data[10] = Tag_ID[1];
	data[11] = Tag_ID[2];
	data[12] = Tag_ID[3];
	data[13] = Tag_ID[4];
	//序号
	data[14] = packnumber;
	//参数地址
	data[15] = Code;
	//Crc16
	crc16  = CRC16((char *)data,16);
	data[16] = (crc16>>8)&0xFF;
	data[17] = crc16&0xFF;
	//base64
	if( 0 != mbedtls_base64_encode(basedata,sizeof(basedata),&baselen,data,18))
		return 0;
	rt_memcpy(data_buf+1,basedata,baselen);
	//帧尾
	*(data_buf+baselen+1) = Message_End;
	return baselen+2;
}
/*
*功能名称: uc_ReTAGCfgERRORPackage 标签设置错误应答帧
*功       能: ,错误信息反馈
*入口参数: data_buf-入口数据
*      data_len-出口数据长度
*出口参数: 无
*/
uint16_t uc_ReSetTagERRORPackage(uint8_t* data_buf,uint8_t* Tag_ID,uint8_t packnumber,uint8_t ErrorCode)
{
	uint8_t data[30];
	unsigned char basedata[30];
	size_t baselen;  
	uint16_t crc16;
	//帧头
	*(data_buf+0) = Message_Head;
	//长度
	data[0] = 0x00;
	data[1] = 0x0E;
	//协议版本
	data[2] = 0xCC;
	//帧类型
	data[3] = 0x04;
	//阅读器类型
	data[4] = DEV_TYPE;
	//阅读器ID
	data[5] = (uint8_t)(NATIVE_ID>>24);
  data[6] = (uint8_t)(NATIVE_ID>>16);
  data[7] = (uint8_t)(NATIVE_ID>>8);
  data[8] = (uint8_t)NATIVE_ID;
	//标签类型
	data[9] = Tag_ID[0];
	//标签id
	data[10] = Tag_ID[1];
	data[11] = Tag_ID[2];
	data[12] = Tag_ID[3];
	data[13] = Tag_ID[4];
	//序号
	data[14] = packnumber;
	//参数地址
	data[15] = ErrorCode;
	//Crc16
	crc16  = CRC16((char *)data,16);
	data[16] = (crc16>>8)&0xFF;
	data[17] = crc16&0xFF;
	//base64
	if( 0 != mbedtls_base64_encode(basedata,sizeof(basedata),&baselen,data,18))
		return 0;
	rt_memcpy(data_buf+1,basedata,baselen);
	//帧尾
	*(data_buf+baselen+1) = Message_End;
	return baselen+2;
}
/*
*功能名称: uc_ReReadTagStatPackage标签读取应答帧
*    功能: 应答读取信息命令
*入口参数: data_buf ——入口数据
           id_buf   ——ID信息
           data     ——数据
           data_len ——数据长度
           rebuf    ——出口数据
*          relen    ——出口数据长度
*出口参数: FALSE代号 或 TRUE
*/
uint16_t uc_ReReadTagStatPackage( uint8_t* data_buf,uint8_t* Tag_ID,uint8_t packnumber,uint8_t *Code)
{
	uint16_t crc16;
	uint8_t data[30];
	unsigned char basedata[30];
	size_t baselen;  
	uint8_t i;
	//帧头
	*(data_buf+0) = Message_Head;
	//协议版本
	data[2] = 0xCC;
	//帧类型
	data[3] = 0x04;
	//阅读器类型
	data[4] = DEV_TYPE;
	//阅读器ID
	data[5] = (uint8_t)(NATIVE_ID>>24);
  data[6] = (uint8_t)(NATIVE_ID>>16);
  data[7] = (uint8_t)(NATIVE_ID>>8);
  data[8] = (uint8_t)NATIVE_ID;
	//标签类型
	data[9] = Tag_ID[0];
	//标签id
	data[10] = Tag_ID[1];
	data[11] = Tag_ID[2];
	data[12] = Tag_ID[3];
	data[13] = Tag_ID[4];
	//序号
	data[14] = packnumber;
	//参数地址
	data[15] = Code[0];
	//参数长度
	data[16] = Code[1];
	for(i=0;i<Code[1];i++)
	{
		//参数值
		data[17+i] = Code[2+i];
	}
	//长度
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
	//帧尾
	*(data_buf+baselen+1) = Message_End;
	return baselen+2;
}
/*
*功能名称: uc_ReReadTagErrorPackage 标签读取错误应答
*    功能: 标签读取信息失败命令
*入口参数:          data_buf ——入口数据
           id_buf   ——ID信息
           data     ——数据
           data_len ——数据长度
           rebuf    ——出口数据
*          relen    ——出口数据长度
*出口参数: FALSE代号 或 TRUE
*/
uint16_t uc_ReReadTagErrorPackage(uint8_t* data_buf,uint8_t packnumber,uint8_t ErrorCode)
{
	uint16_t crc16;
	uint8_t data[30];
	unsigned char basedata[30];
	size_t baselen;  
  //帧头
  *(data_buf+0) = Message_Head;
  //长度
  data[0] = 0x00;
  data[1] = 0x09;
	//协议版本
	data[2] = 0xCC;
  //帧类型
  data[3]  = 0x05;
  //阅读器类型
  data[4] = DEV_TYPE;
  //阅读器ID
  data[5] = (uint8_t)(NATIVE_ID>>24);
  data[6] = (uint8_t)(NATIVE_ID>>16);
  data[7] = (uint8_t)(NATIVE_ID>>8);
  data[8] = (uint8_t)NATIVE_ID;
  //序号
  data[9] = packnumber;
  //错误代号
  data[10] = ErrorCode;
   //Crc16
	crc16  = CRC16((char *)data,11);
	data[11] = (crc16>>8)&0xFF;
	data[12] = crc16&0xFF;
	//base64
	if( 0 != mbedtls_base64_encode(basedata,sizeof(basedata),&baselen,data,13))
		return 0;
	rt_memcpy(data_buf+1,basedata,baselen);
	//帧尾
	*(data_buf+baselen+1) = Message_End;
	return baselen+2;
}


/*
*功能名称: uc_RDMsgPackage 数据解析
*    功能: 解析以太网数据包
*入口参数: data_buf ——入口数据
           data     ——数据
           data_len ——数据长度
           rebuf    ——出口数据
*          relen    ——出口数据长度
*出口参数: FALSE代号 或 TRUE
*/
uint16_t uc_RDMsgPackage(uint8_t* data_buf,uint16_t data_len,uint8_t *status,uint8_t *TagID)
{
	uint8_t er= 0;
	uint8_t real_data[64];
	size_t real_len=64;		
	uint32_t ID=0;
	 /*校验头*/
	if(*(data_buf+0) != Message_Head)
    return 0;
	 /*校验尾*/
	data_len = data_len-1;
  if(*(data_buf+data_len) != Message_End)
    return ERROR_T(ERROR_UC_JUDGE_ENDER);
	if(0 != mbedtls_base64_decode(real_data,sizeof(real_data),&real_len,data_buf+1,(size_t)data_len-1))
		return 0;
	/*校验设备ID号*/
	ID = real_data[4];
	ID = real_data[5] + (ID<<8);
	ID = real_data[6] + (ID<<8);
	ID = real_data[7] + (ID<<8);
	
  if(ID != NATIVE_ID)
     return 0;
	
	switch(real_data[2])
	{
		case 0x02 : //应答心跳帧
		{
			er = HEART;
		}break;
		case 0x03 : //基站设置帧
		{
			er = WRCONFIG;
			APP_Updata(real_data[9],real_data[10],(real_data+11));
			*status = real_data[9];
		}break;
		case 0x06 : //基站读取帧
		{
			er = RDCONFIG;
			*(status) =  real_data[9];
			*(status+1) = APP_Getdata(real_data[9],status+3);
		}break;
		case 0x81 : //标签设置帧
		{
			er = TAG_WRCONFIG;
			TagID[0] = real_data[9];
			TagID[1] = real_data[10];
			TagID[2] = real_data[11];
			TagID[3] = real_data[12];
			APP_Updata(real_data[14],real_data[15],(real_data+16));
			*status = real_data[14];
		}break;
		case 0x84 : //标签读取帧
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



