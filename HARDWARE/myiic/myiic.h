#ifndef __MAX30102_H
#define __MAX30102_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//Mini STM32开发板
//iic 驱动函数	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2010/6/10 
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 正点原子 2009-2019
//All rights reserved
////////////////////////////////////////////////////////////////////////////////// 	  
//IO方向设置
//#define SDA_IN()  {GPIOB->CRH&=0XFFFFFFF0;GPIOB->CRH|=4;}	
//#define SDA_OUT() {GPIOB->CRH&=0XFFFFFFF0;GPIOB->CRH|=7;}
//IO方向设置
#define SDA_IN()    {GPIOB->CRH&=0XFFFFFF0F;GPIOB->CRH|=8<<4;}
#define SDA_OUT()  {GPIOB->CRH&=0XFFFFFF0F;GPIOB->CRH|=3<<4;};

//IO操作函数	 
#define iic_SCL    PBout(8) //SCL
#define iic_SDA    PBout(9) //SDA	 
#define READ_SDA   PBin(9)  //输入SDA 


//iic所有操作函数
void iic_Init(void);                //初始化iic的IO口				 
void iic_Start(void);				//发送iic开始信号
void iic_Stop(void);	  			//发送iic停止信号
void iic_Send_Byte(u8 txd);			//iic发送一个字节
u8 iic_Read_Byte(unsigned char ack);//iic读取一个字节
u8 iic_Wait_Ack(void); 				//iic等待ACK信号
void iic_Ack(void);					//iic发送ACK信号
void iic_NAck(void);				//iic不发送ACK信号

void iic_Write_One_Byte(u8 daddr,u8 addr,u8 data);
void iic_Read_One_Byte(u8 daddr,u8 addr,u8* data);

void iic_WriteBytes(u8 WriteAddr,u8* data,u8 dataLength);
void iic_ReadBytes(u8 deviceAddr, u8 writeAddr,u8* data,u8 dataLength);
#endif
















