#include "myiic.h"
#include "delay.h"

//初始化iic
void iic_Init(void)
{					     

 
	iic_SCL=1;
	iic_SDA=1;

}
//产生iic起始信号
void iic_Start(void)
{
	SDA_OUT();     //sda线输出
	iic_SDA=1;	  	  
	iic_SCL=1;
	delay_us(4);
 	iic_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	iic_SCL=0;//钳住I2C总线，准备发送或接收数据 
}	  
//产生iic停止信号
void iic_Stop(void)
{
	SDA_OUT();//sda线输出
	iic_SCL=0;
	iic_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	iic_SCL=1; 
	iic_SDA=1;//发送I2C总线结束信号
	delay_us(4);							   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 iic_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA设置为输入  
	iic_SDA=1;delay_us(1);	   
	iic_SCL=1;delay_us(1);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			iic_Stop();
			return 1;
		}
	}
	iic_SCL=0;//时钟输出0 	   
	return 0;  
} 
//产生ACK应答
void iic_Ack(void)
{
	iic_SCL=0;
	SDA_OUT();
	iic_SDA=0;
	delay_us(2);
	iic_SCL=1;
	delay_us(2);
	iic_SCL=0;
}
//不产生ACK应答		    
void iic_NAck(void)
{
	iic_SCL=0;
	SDA_OUT();
	iic_SDA=1;
	delay_us(2);
	iic_SCL=1;
	delay_us(2);
	iic_SCL=0;
}					 				     
//iic发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void iic_Send_Byte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    iic_SCL=0;//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
        iic_SDA=(txd&0x80)>>7;
        txd<<=1; 	  
		delay_us(2);   //对TEA5767这三个延时都是必须的
		iic_SCL=1;
		delay_us(2); 
		iic_SCL=0;	
		delay_us(2);
    }	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 iic_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
	{
        iic_SCL=0; 
        delay_us(2);
		iic_SCL=1;
        receive<<=1;
        if(READ_SDA)receive++;   
		delay_us(1); 
    }					 
    if (!ack)
        iic_NAck();//发送nACK
    else
        iic_Ack(); //发送ACK   
    return receive;
}


void iic_WriteBytes(u8 WriteAddr,u8* data,u8 dataLength)
{		
	u8 i;	
    iic_Start();  

	iic_Send_Byte(WriteAddr);	    //发送写命令
	iic_Wait_Ack();
	
	for(i=0;i<dataLength;i++)
	{
		iic_Send_Byte(data[i]);
		iic_Wait_Ack();
	}				    	   
    iic_Stop();//产生一个停止条件 
	delay_ms(10);	 
}

void iic_ReadBytes(u8 deviceAddr, u8 writeAddr,u8* data,u8 dataLength)
{		
	u8 i;	
    iic_Start();  

	iic_Send_Byte(deviceAddr);	    //发送写命令
	iic_Wait_Ack();
	iic_Send_Byte(writeAddr);
	iic_Wait_Ack();
	iic_Send_Byte(deviceAddr|0X01);//进入接收模式			   
	iic_Wait_Ack();
	
	for(i=0;i<dataLength-1;i++)
	{
		data[i] = iic_Read_Byte(1);
	}		
	data[dataLength-1] = iic_Read_Byte(0);	
    iic_Stop();//产生一个停止条件 
	delay_ms(10);	 
}

void iic_Read_One_Byte(u8 daddr,u8 addr,u8* data)
{				  	  	    																 
    iic_Start();  
	
	iic_Send_Byte(daddr);	   //发送写命令
	iic_Wait_Ack();
	iic_Send_Byte(addr);//发送地址
	iic_Wait_Ack();		 
	iic_Start();  	 	   
	iic_Send_Byte(daddr|0X01);//进入接收模式			   
	iic_Wait_Ack();	 
    *data = iic_Read_Byte(0);		   
    iic_Stop();//产生一个停止条件	    
}

void iic_Write_One_Byte(u8 daddr,u8 addr,u8 data)
{				   	  	    																 
    iic_Start();  
	
	iic_Send_Byte(daddr);	    //发送写命令
	iic_Wait_Ack();
	iic_Send_Byte(addr);//发送地址
	iic_Wait_Ack();	   	 										  		   
	iic_Send_Byte(data);     //发送字节							   
	iic_Wait_Ack();  		    	   
    iic_Stop();//产生一个停止条件 
	delay_ms(10);	 
}



























