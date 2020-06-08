/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-19     CCRFIDZY       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include "App_LF125K.h"
#include "string.h"
/*---PWM--------*/
#define PWM_DEV_NAME        "pwm1"  /* PWM设备名称 */
#define PWM_DEV_CHANNEL      1     /* PWM通道 */
struct rt_device_pwm *pwm_dev;          /* PWM设备句柄 */

/*---定时器--------*/
#define HWTIMER_DEV_NAME   "timer11"     /* 定时器名称 */
rt_device_t hw_dev;             /* 定时器设备句柄 */
/* 定时器的控制块 */
//static rt_timer_t timer1;
/*---LF125K--------*/
#define IDLE                0x00  // 空闲
#define HIGH                0x01  // 高电平
#define LOW                 0x02  // 低电平
#define END                 0x03  // 完成

#define SEND_RF_Frequency           0x10                        //默认无线发射频点
#define LF_RES                      224                         //默认低频的RES 参数,调节距离
#define POS_TYPE                    0x69                        //定位器类型头
#define LOCAT_TIM                   0x000F                      //默认定位时间间隔(10ms  单位)
#define LOCAT_TIMH                  ((LOCAT_TIM>>8)&0xFF)       //默认定位时间
#define LOCAT_TIML                  (LOCAT_TIM&0xFF)

typedef union __UION_LF_SEND_MSG
{
  struct __LF_SEND_MSG
  {
    // 普通前导     0xAA
      uint8_t  nor_data8;
      uint8_t  nor_status;
      uint8_t  nor_bit;
    // 曼切斯特前导 0x96
      uint8_t  man_data8;
      uint8_t  man_status;
      uint8_t  man_bit;
    // 4字节数据    4type
      uint8_t  type4_status;
      uint8_t  type4_bit;
      uint32_t type4_data32;
  }un_lf_data;
  uint8_t un_lf_msg[sizeof(struct __LF_SEND_MSG)];
}un_LFMsg;
un_LFMsg unLfMsg;


/*---------全局变量-------------------------*/
uint8_t TagSet_TimerOutFlog;         //标签配置超时计时标志
uint8_t LfConfigBuf[20];
uint8_t LocTimOutCnt,LocTimOutBas;
uint32_t                      SENDA_DATA32;
uint32_t                      SENDB_DATA32;
uint32_t                      SENDC_DATA32;
/*---------函数-------------------------*/
void Locationprotoc(const uint8_t* buf ,uint8_t buflen ,uint8_t* re_buf , uint8_t rebuflen);
unsigned char CRC8(uint8_t * pData, uint16_t len);
/* 定时器 1 超时函数 */
static rt_err_t timeout11(rt_device_t dev, rt_size_t size)
{
    if (TagSet_TimerOutFlog)
    {
      if(BSP_LfConfigTagMsg(LfConfigBuf))
      {

      }
    }
    else
    {
      if(LfSendMsg())
      {

      }
    }
    return 0;
}
/* 定时器 1 超时函数 */
static void timeout1(void *parameter)
{
    //超出500ms
    if (TagSet_TimerOutFlog)
    {
        TagSet_TimerOutFlog = 0;
    }
}

void LF125K_Init(void)
{
    rt_uint32_t period = 8000;     /* 频率为12KHz，单位为纳秒ns */
    rt_uint32_t pulse = 4000;      /* PWM脉冲宽度值的增减方向 */
    rt_uint8_t  LocateA_ID[2];            // 定位器A ID号
    rt_uint8_t  LocateA_Buf[4];           // 定位器A 数据缓存
    rt_hwtimerval_t timeout_s;
    rt_uint32_t freq = 2000;             /* 计数频率 */
    rt_hwtimer_mode_t mode;                 /* 定时器模式 */
    LocTimOutCnt = LOCAT_TIMH;//定位时间
    LocTimOutBas = LOCAT_TIML;//定位时间基数*10ms
    /* step 1.1、查找 PWM 设备 */
    pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);
    if (pwm_dev == RT_NULL)
    {
       rt_kprintf("pwm sample run failed! can't find %s device!\n", PWM_DEV_NAME);
    }
    /* step 1.2、设置 PWM 周期和脉冲宽度默认值 */
    rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL, period, pulse);
    /* step 1.3、使能 PWM 设备的输出通道 */
    rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);

    /* 查找定时器设备 */
    hw_dev = rt_device_find(HWTIMER_DEV_NAME);
    if (hw_dev == RT_NULL)
    {
       rt_kprintf("hwtimer sample run failed! can't find %s device!\n", HWTIMER_DEV_NAME);
    }
    /* 以读写方式打开设备 */
    if (rt_device_open(hw_dev, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
       rt_kprintf("open %s device failed!\n", HWTIMER_DEV_NAME);
    }
    /* 设置超时回调函数 */
     rt_device_set_rx_indicate(hw_dev, timeout11);
     /* 设置计数频率(默认1Mhz或支持的最小计数频率) */
     rt_device_control(hw_dev, HWTIMER_CTRL_FREQ_SET, &freq);
     /* 设置模式为周期性定时器 */
     mode = HWTIMER_MODE_PERIOD;
     rt_device_control(hw_dev, HWTIMER_CTRL_MODE_SET, &mode);
     /* 设置定时器超时值为5s并启动定时器 */
    timeout_s.sec = 0;      /* 秒 */
    timeout_s.usec = 2500;     /* 微秒 */

    if (rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s)) != sizeof(timeout_s))
    {
        rt_kprintf("set timeout value failed\n");
    }
    rt_device_close(hw_dev); //定时器开启后 将不能动态申请内存


    /* 创建定时器 1  周期定时器 */
    rt_timer_create("timer1", timeout1,
                             RT_NULL, 500,
                             RT_TIMER_FLAG_PERIODIC);

    LocateA_ID[0] = 0x00;
    LocateA_ID[1] = 0x09;
    Locationprotoc(LocateA_ID,2,LocateA_Buf,4);
    SENDA_DATA32 =SENDB_DATA32=SENDC_DATA32=(rt_uint32_t)(LocateA_Buf[0]<<24) + (rt_uint32_t)(LocateA_Buf[1]<<16) + (rt_uint32_t)(LocateA_Buf[2]<<8 ) + (rt_uint32_t)(LocateA_Buf[3]);//设置 定位器需要发送的定位数据

}

uint8_t LfSendMsg(void)
{
  uint8_t err = 0x00;
  static uint8_t status = 0x00,init = 0x00,wakestatus = 0x00;
  static uint8_t channel;
  static uint8_t  tmp_loaccnt;
  static uint32_t tmp_loacbas;
  rt_hwtimerval_t timeout_s;      /* 定时器超时值 */
  timeout_s.sec = 0;
  switch(status)//
  {
      case 0x00:   // 唤醒头：设置定时器的定时时长为 5ms
        if(!wakestatus)
        {
          channel = 0x03/*ChoiceChannel()*/;  //单轴，对应PCB上中间一路输出
          rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);
          timeout_s.usec = 2500;
          rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
          tmp_loaccnt = LocTimOutCnt;
          if(!tmp_loaccnt)
              tmp_loaccnt = 1;
          tmp_loacbas = LocTimOutBas*1000;
          if(!tmp_loacbas)
              tmp_loacbas = 1000;// 10ms 單位
          memset(unLfMsg.un_lf_msg,0,sizeof(un_LFMsg));
          unLfMsg.un_lf_data.nor_data8 = 0xAA;
          unLfMsg.un_lf_data.man_data8  = 0x96;
          wakestatus = 0x01;
          if(channel == 3)
            {unLfMsg.un_lf_data.type4_data32 = SENDA_DATA32;break;}
          else if(channel == 4)
            {unLfMsg.un_lf_data.type4_data32 = SENDB_DATA32;break;}
          else if(channel == 2)
            {unLfMsg.un_lf_data.type4_data32 = SENDC_DATA32;break;}
          else
            {break;}
        }
        else
        {
          rt_pwm_disable(pwm_dev, PWM_DEV_CHANNEL);
          timeout_s.usec = 150;
          rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
          wakestatus = 0x00;
          status = 0x01;
          break;
        }
      case 0x01:  // 0xAA
        if((unLfMsg.un_lf_data.nor_bit < 8)&&(unLfMsg.un_lf_data.nor_status == IDLE))
        {
          if(unLfMsg.un_lf_data.nor_data8&0X80)
          {
            unLfMsg.un_lf_data.nor_status = HIGH;
          }
          else
          {
            unLfMsg.un_lf_data.nor_status = LOW;
          }
          unLfMsg.un_lf_data.nor_data8 <<= 1;
        }
        if(unLfMsg.un_lf_data.nor_status == HIGH)
        {
          rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);
          timeout_s.usec = 150;
          rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
          unLfMsg.un_lf_data.nor_status = END;
        }
        if(unLfMsg.un_lf_data.nor_status == LOW)
        {
          rt_pwm_disable(pwm_dev, PWM_DEV_CHANNEL);
          timeout_s.usec = 150;
          rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
          unLfMsg.un_lf_data.nor_status = END;
        }
        if(unLfMsg.un_lf_data.nor_bit > 8-1)
        {
          status = 0x02;
        }
        else
        {
          if(unLfMsg.un_lf_data.nor_status == END)
          {
            unLfMsg.un_lf_data.nor_status = IDLE;
            unLfMsg.un_lf_data.nor_bit ++;
          }
          break;
        }
      case 0x02:  // x096
        if((unLfMsg.un_lf_data.man_bit < 8)&&(unLfMsg.un_lf_data.man_status == IDLE))
        {
          if(unLfMsg.un_lf_data.man_data8&0X80){unLfMsg.un_lf_data.man_status = HIGH;}
          else              {unLfMsg.un_lf_data.man_status = LOW; }
          unLfMsg.un_lf_data.man_data8 <<= 1;
        }
        if(unLfMsg.un_lf_data.man_status == HIGH)
        {
          if(!init){
            rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);
            timeout_s.usec = 150;
            rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
            init = 0x01;
          }
          else{
            rt_pwm_disable(pwm_dev, PWM_DEV_CHANNEL);
            timeout_s.usec = 150;
            rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
            init = 0x00;
            unLfMsg.un_lf_data.man_status = END;
          }
        }
        if(unLfMsg.un_lf_data.man_status == LOW)
        {
          if(!init){
            rt_pwm_disable(pwm_dev, PWM_DEV_CHANNEL);
            timeout_s.usec = 150;
            rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
            init = 0x01;
          }
          else{
            rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);
            timeout_s.usec = 150;
            rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
            init = 0x00;
            unLfMsg.un_lf_data.man_status = END;
          }
        }
        if(unLfMsg.un_lf_data.man_bit > 8-1)
        {
          if(!init){
            rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);
            timeout_s.usec = 20;
            rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
            init = 0x01;
            break;
          }
          else{
            status = 0x03;
            init = 0x00;
          }
        }
        else{
          if(unLfMsg.un_lf_data.man_status == END){unLfMsg.un_lf_data.man_status = IDLE;unLfMsg.un_lf_data.man_bit ++;}
          break;
        }
      case 0x03:
        if((unLfMsg.un_lf_data.type4_bit < 32)&&(unLfMsg.un_lf_data.type4_status == IDLE))
        {
          if(unLfMsg.un_lf_data.type4_data32&0X80000000){unLfMsg.un_lf_data.type4_status = HIGH;}
          else                       {unLfMsg.un_lf_data.type4_status = LOW; }
          unLfMsg.un_lf_data.type4_data32 <<= 1;
        }
        if(unLfMsg.un_lf_data.type4_status == HIGH)
        {
          if(!init){
            rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);
            timeout_s.usec = 150;
            rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
            init = 0x01;
          }
          else{
            rt_pwm_disable(pwm_dev, PWM_DEV_CHANNEL);
            timeout_s.usec = 160;
            rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
            init = 0x00;
            unLfMsg.un_lf_data.type4_status = END;
          }
        }
        if(unLfMsg.un_lf_data.type4_status == LOW)
        {
          if(!init){
            rt_pwm_disable(pwm_dev, PWM_DEV_CHANNEL);
            timeout_s.usec = 150;
            rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
            init = 0x01;
          }
          else{
            rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);
            timeout_s.usec = 160;
            rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
            init = 0x00;
            unLfMsg.un_lf_data.type4_status = END;
          }
        }
        if(unLfMsg.un_lf_data.type4_bit > 32-1){status = 0x04;}
        else{
          if(unLfMsg.un_lf_data.type4_status == END){unLfMsg.un_lf_data.type4_status = IDLE;unLfMsg.un_lf_data.type4_bit ++;}
          break;
        }
      case 0x04:
        //  设置定时器的定时时长为 200ms
        rt_pwm_disable(pwm_dev, PWM_DEV_CHANNEL);
        srand(rt_tick_get());
        timeout_s.usec = 5*tmp_loacbas+(rand()%100)*50; //设置定时器的定时时長
        rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
        err = 0x01;
        status = wakestatus = init = 0x00;
        break;
      default:
        break;
  }
  return err;
}
/*
*********************************************************************************************************
* 函数名：BSP_LfConfigTagMsg()
*
* 描述  ：配置标签，通过LF 发送数据包
*
* 输入  ：*pbuf(发送的六字节数据包)
*
* 输出  ：1：发送完成 0：发送未完成
*
* 说明  ：
*********************************************************************************************************
*/
uint8_t BSP_LfConfigTagMsg(uint8_t* pbuf)
{
    uint8_t  err          = 0x00;
    static uint8_t status = 0x00;
    static uint8_t init   = 0x00;

//    static uint8_t channel = 0x04;
    static uint8_t i = 0;
    // 唤醒头       5ms
    static uint8_t wake = 0x00;
    // 普通前导     0xAA
    static uint8_t nor_data8;
    static uint8_t nor_status = IDLE;
    static uint8_t nor_bit    = 0x00;
    // 曼切斯特前导 0x96
    static uint8_t man_data8;
    static uint8_t man_status = IDLE;
    static uint8_t man_bit    = 0x00;
    // 6字节数据    6type
    static uint8_t  type6_data[6];
    static uint8_t  type6_status = IDLE;
    static uint8_t  type6_bit    = 0x00;
    rt_hwtimerval_t timeout_s;      /* 定时器超时值 */
    timeout_s.sec = 0;
    switch(status)//
    {
        case 0x00:    // 唤醒头：设置定时器的定时时长为 5ms
                  if(!wake)
                  {
//                    channel = 0x04/*ChoiceChannel()*/;  //单轴，对应PCB上中间一路输出
                    rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);
                    timeout_s.usec = 2500;
                    rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
                    nor_data8 = 0xAA;
                    man_data8  = 0x96;
                    memcpy(type6_data, pbuf, 6);
                    nor_status = IDLE;
                    nor_bit    = 0x00;
                    man_status = IDLE;
                    man_bit    = 0x00;
                    type6_status = IDLE;
                    type6_bit  = 0x00;
                    wake = 0x01;
                  }
                  else
                  {
                    rt_pwm_disable(pwm_dev, PWM_DEV_CHANNEL);
                    timeout_s.usec = 150;
                    rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
                    wake = 0x00;
                    status = 0x01;
                  }
        break;
        case 0x01:  // 0xAA
                  if((nor_bit < 8)&&(nor_status == IDLE))
                  {
                    if(nor_data8&0X80)
                    {
                      nor_status = HIGH;
                    }
                    else
                    {
                      nor_status = LOW;
                    }
                    nor_data8 <<= 1;
                  }
                  if(nor_status == HIGH)
                  {
                    rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);
                    timeout_s.usec = 150;
                    rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
                    nor_status = END;
                  }
                  if(nor_status == LOW)
                  {
                    rt_pwm_disable(pwm_dev, PWM_DEV_CHANNEL);
                    timeout_s.usec = 150;
                    rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
                    nor_status = END;
                  }
                  if(nor_bit > 8-1)
                  {
                    status = 0x02;
                  }
                  else
                  {
                    if(nor_status == END)
                    {
                      nor_status = IDLE;nor_bit ++;
                    }
                      break;
                  }
        case 0x02:  // x096
                  if((man_bit < 8)&&(man_status == IDLE))
                  {
                    if(man_data8&0X80)
                    {
                      man_status = HIGH;
                    }
                    else
                    {
                      man_status = LOW;
                    }
                    man_data8 <<= 1;
                  }
                  if(man_status == HIGH)
                  {
                    if(!init)
                    {
                      rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);
                      timeout_s.usec = 150;
                      rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
                      init = 0x01;
                    }
                    else
                    {
                      rt_pwm_disable(pwm_dev, PWM_DEV_CHANNEL);
                      timeout_s.usec = 150;
                      rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
                      init = 0x00;
                      man_status = END;
                    }
                  }
                  if(man_status == LOW)
                  {
                    if(!init)
                    {
                        rt_pwm_disable(pwm_dev, PWM_DEV_CHANNEL);
                        timeout_s.usec = 150;
                        rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
                        init = 0x01;
                    }
                    else
                    {
                      rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);
                      timeout_s.usec = 150;
                      rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
                      init = 0x00;
                      man_status = END;
                    }
                  }
                  if(man_bit > 8-1)
                  {
                    if(!init)
                    {
                        rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);
                        timeout_s.usec = 20;
                        rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
                        init = 0x01;
                        break;
                    }
                    else
                    {
                      status = 0x03;
                      init = 0x00;
                    }
                  }
                  else
                  {
                    if(man_status == END)
                    {
                      man_status = IDLE;
                      man_bit ++;
                    }
                    break;
                  }
        case 0x03:
                  if((type6_bit < 8)&&(type6_status == IDLE)) //六字节数据包
                  {
                    if(type6_data[i]&0X80)
                    {
                      type6_status = HIGH;
                    }
                    else
                    {
                      type6_status = LOW;
                    }
                    type6_data[i] <<= 1;
                  }
                  if(type6_status == HIGH)
                  {
                    if(!init)
                    {
                        rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);
                        timeout_s.usec = 150;
                        rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
                        init = 0x01;
                    }
                    else
                    {
                        rt_pwm_disable(pwm_dev, PWM_DEV_CHANNEL);
                        timeout_s.usec = 150;
                        rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
                        init = 0x00;
                        type6_status = END;
                    }
                  }
                  if(type6_status == LOW)
                  {
                    if(!init)
                    {
                        rt_pwm_disable(pwm_dev, PWM_DEV_CHANNEL);
                        timeout_s.usec = 150;
                        rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
                        init = 0x01;
                    }
                    else
                    {
                        rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);
                        timeout_s.usec = 150;
                        rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s));
                        init = 0x00;
                        type6_status = END;
                    }
                  }
                  if(type6_status == END)
                  {
                    type6_status = IDLE;
                    type6_bit ++;
                    if(type6_bit > 8-1)
                    {
                      i ++;
                      type6_bit = 0;
                      if(i>5)
                      {
                              i = 0;
                              status = 0x04;
                      }
                    }
                  }
                  break;
        case 0x04:
        {
            rt_pwm_disable(pwm_dev, PWM_DEV_CHANNEL);
            status = 0x00;
            init = 0x00;
            wake = 0x00;
            err = 0x01;
        }break;
        default:
        break;
    }
  return err;
}

/*
*功能名称: LocationSever 125k定位部分协议
*入口参数: 入口buf——本机ID  buflen buf长度 ;re_buf返回数据,rebuflen数据长度
*出口参数: FALSE代号 或 TRUE
*/
void Locationprotoc(const uint8_t* buf ,uint8_t buflen ,uint8_t* re_buf , uint8_t rebuflen)
{
  if((buflen != 2)&&(rebuflen != 4))
    return;
  *(re_buf+0)  = 0x69;//定位器类型头
  *(re_buf+1) = *(buf+1);
  *(re_buf+2) = *buf;
  *(re_buf+2) <<= 5;
  *(re_buf+2) = ((*(re_buf+2))|(((*(re_buf+1))^0xAA)&0x1F));
  *(re_buf+3) = CRC8(re_buf,sizeof(re_buf)-1);
}



/*
 * name : void Change_Resistor(u8 channel ,u8 Data, u16 CMD)
 *
 * funtion:   改变电位器电阻阻值
 */
void Change_Resistor(uint8_t Data)
{
//   PWR_L;
//    if(Data < 225)     //档位从0 ~ 224  一共 225个档位 ,每个75个单位 ，225最大 0 最小, 从12v ~ 5v
//    {
//      Data = GET_GEAR(Data); //转换档位
//       IWDG_ReloadCounter();  //喂狗
//       I2C_DP_CHA1_Write(2,CMD);
//      if(channel == CHANNEL1)
//      {
//             if(Data < 75)
//               {
//                       Relay_control(CHANNEL1,res_mode1);
//               }
//               else if (Data < 150)
//               {
//                       Data -= 75;
//                       Relay_control(CHANNEL1,res_mode2);
//               }
//               else if (Data < 225)
//               {
//                       Data -= 150;
//                       Relay_control(CHANNEL1,res_mode3);
//               }
//               Delay_1ms(100);
//               I2C_DP_CHA1_Write(Data,CMD);
//      }
//    }
//    PWR_H;

}

