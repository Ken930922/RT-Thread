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

#define VERSION_Byte0    0x02    //版本号FV20
#define VERSION_Byte1    0x02
#define VERSION_Byte2    0x00
#define VERSION_Byte3    0x01

#define DEV_TYPE         0x12       //设备类型
#define NATIVEDEF_ID     0x00000005 //默认本机ID

#define UWBMODE_DEF      0x01 //UWB默认模式
#define UWBHEIGH_DEF     0x00 //UWB默认高度


/*设备  控制*/
#define HEART             1      //  设备 心跳
#define UPLOAD		        2      //  设备 上报
#define SET_REPLY         3      //  设备 设置应答
#define SET_ERROR         4      //  设备 设置错误
#define READ_REPLY        5      //  设备 读取应答
#define READ_ERROR        6      //  设备 读取错误
#define TAG_UPLOAD        7      //  标签 上报

/*写标签参数配置命令*/
#define TAG_WRCONFIG      20     //  写配置信息

/*读标签参数配置命令*/
#define TAG_RDCONFIG      30     //  读配置信息

/*写终端参数配置命令*/
#define WRCONFIG          60     //  写配置信息

/*读终端参数配置命令*/
#define RDCONFIG          80     //  读配置信息

/*读取参数扩展指令*/
#define RD_CFGMSG         100    //  读取设备配置参数信息

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
/**************************数据解析***************************/
/*************************************************************/
/*数据包解析*/
uint16_t uc_RDMsgPackage(uint8_t* data_buf,uint16_t data_len,uint8_t *status,uint8_t *TagID);
/*无线 数据解析*/
uint16_t bt_ProDataLearning(const uint8_t* data_buf);
/*************************************************************/
/**************************数据打包***************************/
/*************************************************************/
/*设备上报帧打包*/
uint8_t uc_UploadDataPackage(uint8_t* data_buf,uint8_t packnumber);

/*心跳数据打包*/
uint16_t uc_HeartDataPackage(uint8_t* data_buf, uint8_t packnumber);

/*配置命令应答打包*/
uint16_t uc_ReSetPackage(uint8_t* data_buf,uint8_t packnumber,uint8_t ConfigCode);

/*配置失败应答打包*/
uint16_t uc_ReCfgERRORPackage(uint8_t* data_buf,uint8_t packnumber,uint8_t ErrorCode);

/*设备状态查询应答打包*/
uint16_t uc_ReReadStatPackage( uint8_t* data_buf,uint8_t packnumber,uint8_t *Code);
/*设备状态查询错误应答打包*/


/*标签数据打包*/
uint16_t uc_ReReadErrorPackage(uint8_t* data_buf,uint8_t packnumber,uint8_t ErrorCode);

/*标签配置命令应答打包*/
uint16_t uc_ReSetTagPackage(uint8_t* data_buf,uint8_t* Tag_ID, uint8_t packnumber,uint8_t Code);

/*标签配置失败应答打包*/
uint16_t uc_ReSetTagERRORPackage(uint8_t* data_buf,uint8_t* Tag_ID,uint8_t packnumber,uint8_t ErrorCode);

/*标签设备状态查询应答打包*/
uint16_t uc_ReReadTagStatPackage( uint8_t* data_buf,uint8_t* Tag_ID,uint8_t packnumber,uint8_t *Code);

/*标签设备状态查询错误应答打包*/
uint16_t uc_ReReadTagErrorPackage(uint8_t* data_buf,uint8_t packnumber,uint8_t ErrorCode);

/*定位信息打包*/
uint16_t Locationprotoc(const uint8_t* buf ,uint8_t buflen ,uint8_t* re_buf ,
                     uint8_t rebuflen);

/*标签上报*/
uint8_t uc_TAGNormalDataPackage(uint8_t* data_buf ,const uint8_t* ble_buf,uint8_t packnumber);

void APP_flash_upload(uint8_t *table,uint8_t star,uint8_t len);

#endif /* MYAPP_APP_PROTOCOL_H_ */
