#include <string.h>
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "oled.h"
#include "usart.h"
#include "dht11.h"
#include "adc.h"
#include "onenet.h"//网络协议层
#include "esp8266.h"//网络设备
#include "max30102.h"
#include "myiic.h"
#include "algorithm.h"
#include "key.h"  

// 定义Save_Data结构体，用于存储GPS相关数据
typedef struct {
    char GPS_Buffer[256];  // 存储GPS数据缓冲区
    char UTCTime[11];      // UTC时间
    char latitude[11];     // 纬度信息
    char N_S[2];           // 南北标志
    char longitude[12];    // 经度信息
    char E_W[2];           // 东西标志
    uint8_t isGetData;     // 是否获取到数据标志
    uint8_t isParseData;   // 是否解析完数据标志
    uint8_t isUsefull;     // 数据是否有效标志
} Save_Data_TypeDef;

//Save_Data_TypeDef Save_Data;
float longitude ;
float latitude;

// 全局变量（仅保留时间+定时器3核心，无无关变量）
u8 miao, fen, shi;              // 系统时间：秒/分/时
u16 g_tim3_1ms_cnt = 0;         // 定时器3 1ms计数变量
u8 flag_100ms = 0;              // 100ms标志位（可选，用于低频任务）
 u8 miao1=0, fen1=1, shi1;       // 1号预约时间：秒/分/时
u8 miao2=0, fen2=3, shi2;       // 2号预约时间：秒/分/时
u8 miao3=0, fen3=5, shi3;       // 3号预约时间：秒/分/时
u8 miao4=0, fen4=7, shi4;       // 4号预约时间：秒/分/时
u8 YL1 = 2, YL2 = 2, YL3 = 3, YL4 = 4; // 1-4号预约标识
// 定义一些全局变量
u8 WDH = 10;                    
u8 SD = 30;                       // 湿度阈值变量
u8 temperature;                  // 温度变量
u8 temph = 20;                 
u8 Lightingh = 40;                // 空气高值
u8 fan;                          
u8 humidity;                     // 湿度变量
u16 ADC_Value[30];               // adc数据缓冲区
u16 value1, value2, value3;      // DMA数据处理的三个中间变量 
float Distance;                   // 距离变量
u8 Lighting, Voice, Gas;         // 空气相关变量
u8 LED_value;                    // LED灯的值，0或1
u8 fscs = 0;                      
u8 dis_hr1; 
u8 dis_spo21;                    // 血氧数据
u8 yz = 0;                       // 按键设置模式
u8 die = 0;                   
u8 buf1[10] = {0};
u8 buf2[10] = {0};
u8 buf3[10] = {0};
u8 buf4[10] = {0};
u8 buf5[10] = {0};
u8 buf6[10] = {0};
u8 buf7[10] = {0};
u8 buf8[10] = {0};
u8 buf9[10] = {0};
u8 buf10[10] = {0};
u16 i;                           // for循环用到的变量

#define MAX_BRIGHTNESS 255

u32 aun_ir_buffer[500];// IR LED sensor data，红外LED传感器数据缓冲区
int32_t n_ir_buffer_length; // 数据长度
u32 aun_red_buffer[500]; // Red LED sensor data，红色LED传感器数据缓冲区

int32_t n_sp02; // SPO2 value，血氧值
int8_t ch_spo2_valid;   // indicator to show if the SP02 calculation is valid，血氧计算是否有效的标志
int32_t n_heart_rate;   // heart rate value，心率值
int8_t  ch_hr_valid;    // indicator to show if the heart rate calculation is valid，心率计算是否有效的标志

//  Distance;

float dis_hr = 0, dis_spo2 = 0;

float jd;            
float weid;            
u16 gps1, gps2, gps3, gps4, gps5, gps6, gps7, gps8, gps9, gps10;
u16 gps11, gps22, gps33, gps44, gps55, gps66, gps77, gps88, gps99, gps1010;

// 错误日志函数，当发生错误时循环打印错误编号
void errorLog(int num) {
    while (1) {
        printf("ERROR%d\r\n",num);
    }
}

// 将度分格式转换为十进制格式的函数
float convertDMStoDD(const char *dms) {
    float degrees = 0;
    float minutes = 0;
    sscanf(dms, "%f", &degrees);
    int intDegrees = (int)(degrees / 100);
    minutes = degrees - intDegrees * 100;
    return intDegrees + minutes / 60;
}

// 解析GPS缓冲区数据的函数
void parseGpsBuffer() {
    char *subString;
    char *subStringNext;
    char i = 0;
    // 如果已经获取到GPS数据
    if (Save_Data.isGetData) {
        Save_Data.isGetData = false;
        printf("**************\r\n");
        printf(Save_Data.GPS_Buffer);

        for (i = 0 ; i <= 6 ; i++) {
            if (i == 0) {
                // 查找第一个逗号，若未找到则表示解析错误
                if ((subString = strstr(Save_Data.GPS_Buffer, ",")) == NULL)
                    errorLog(1);  
            } else {
                subString++;
                // 查找下一个逗号
                if ((subStringNext = strstr(subString, ",")) != NULL) {
                    char usefullBuffer[2]; 
                    // 根据不同的索引处理不同的数据
                    switch(i) {
                        case 1:memcpy(Save_Data.UTCTime, subString, subStringNext - subString);break; // 获取UTC时间
                        case 2:memcpy(usefullBuffer, subString, subStringNext - subString);break; // 获取是否有效标志
                        case 3:memcpy(Save_Data.latitude, subString, subStringNext - subString);break; // 获取纬度信息
                        case 4:memcpy(Save_Data.N_S, subString, subStringNext - subString);break; // 获取N/S
                        case 5:memcpy(Save_Data.longitude, subString, subStringNext - subString);break; // 获取经度信息
                        case 6:memcpy(Save_Data.E_W, subString, subStringNext - subString);break; // 获取E/W
                        default:break;
                    }

                    subString = subStringNext;
                    Save_Data.isParseData = true;
                    // 根据有效标志设置数据是否有效
                    if(usefullBuffer[0] == 'A')
                        Save_Data.isUsefull = true;
                    else if(usefullBuffer[0] == 'V')
                        Save_Data.isUsefull = false;

                } else {
                    errorLog(2); // 解析错误
                }
            }
        }
    }
}

// 打印GPS缓冲区数据的函数
void printGpsBuffer() {
    // 如果已经解析完GPS数据
    if (Save_Data.isParseData) {
        Save_Data.isParseData = false;

        printf("Save_Data.UTCTime = "); // 当前时间
        printf(Save_Data.UTCTime);
        printf("\r\n");

        // 如果数据有效
        if (Save_Data.isUsefull) {
            Save_Data.isUsefull = false;

            // 纬度处理
            latitude = convertDMStoDD(Save_Data.latitude);
            if (Save_Data.N_S[0] == 'S') {
                latitude = -latitude;
            }
            sprintf((char *)buf6, "%.6f", latitude);
            printf("Save_Data.latitude = %s\r\n", buf6);
            OLED_ShowStr(0, 6, buf6, 2); // 显示纬度

            // 经度处理
            longitude = convertDMStoDD(Save_Data.longitude);
            if (Save_Data.E_W[0] == 'W') {
                longitude = -longitude;
            }
            sprintf((char *)buf6, "%.6f", longitude);
            printf("Save_Data.longitude = %s\r\n", buf6);
            OLED_ShowStr(0, 4, buf6, 2); // 显示经度
        } else {
            // 无有效数据时显示默认值
            sprintf(buf6, "0.000000");
            OLED_ShowStr(0, 4, buf6, 2); // 显示经度
            OLED_ShowStr(0, 6, buf6, 2); // 显示纬度
        }
    }
}

// 初始化USART3的函数
void USART3_Init(u32 bound) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 使能GPIOA和USART3时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    // 配置USART3_TX PB10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 配置USART3_RX PB11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // USART3 初始化设置
    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART3, &USART_InitStructure);

    // 使能USART3
    USART_Cmd(USART3, ENABLE);
}

// 通过USART3发送字符串的函数
void USART3_SendString(char *str) {
    while (*str) {
        // 等待发送缓冲区为空
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
        // 发送一个字符
        USART_SendData(USART3, (uint8_t)*str++);
    }
    // 等待发送完成
    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
}


// 定时器3中断服务函数（仅做：100ms标志+系统时间更新）
void TIM3_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        // 1. 生成100ms标志位（可选，主循环可用于低频刷新）
        g_tim3_1ms_cnt++;
        if(g_tim3_1ms_cnt >= 100)
        {
            g_tim3_1ms_cnt = 0;
            flag_100ms = 1;
        }

        // 2. 系统时间更新（1ms*1000=1秒，核心功能）
        static u16 sys_1s_cnt = 0;
        sys_1s_cnt++;
        if(sys_1s_cnt >= 1000)
        {
            sys_1s_cnt = 0;
            miao++;          // 秒加1
            if(miao >= 60)   // 秒满60进分
            {
                miao = 0;
                fen++;
                if(fen >= 60) // 分满60进时
                {
                    fen = 0;
                    shi++;
                    if(shi >= 24) // 时满24归0
                        shi = 0;
                }
            }
        }

        TIM_ClearITPendingBit(TIM3, TIM_IT_Update); // 清除中断标志
    }
}

// 定时器3初始化函数（1ms中断，参数固定999,71，不可改）
void TIM3_Int_Init(u16 arr, u16 psc)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); // 使能TIM3时钟

    // 定时器时基配置（72MHz→1ms中断）
    TIM_TimeBaseStructure.TIM_Period = arr;        // 自动重装值：999
    TIM_TimeBaseStructure.TIM_Prescaler = psc;     // 预分频器：71
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;   // 时钟分割：无
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);     // 使能更新中断

    // 中断优先级配置
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM3, ENABLE); // 使能TIM3
}
int main(void)
{ static uint8_t key_off_count = 0;
	unsigned char *dataPtr = NULL;
	unsigned short timeCount = 300;	//发送间隔变量
	u8 t=0;
	u32	un_min, un_max, un_prev_data; 
	int32_t n_brightness;
	u16 i;
	u8 temp[6];
  u8 sz=0;
	float f_temp;
	
//==================================片内外设初始化==============================================
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组
	delay_init();			          //延时函数初始化
		uart_init(9600);	 //串口初始化为9600
	Usart2_Init(115200); 	      //串口2，与ESP8266通信
  USART3_Init(9600); // 初始化串口3，波特率设为9600，根据需要调整
	LED_Init();				          //LED初始化
	OLED_Init(); 	              //OLED初始化  
	TIM3_Int_Init(999, 71); // 定时器3初始化：1ms中断（核心）
//	DHT11_Init();               //初始化温湿度模块
	ADCx_Init();                //初始化ADC 
 
////===================================网络协议初始化===============================================	
ESP8266_Init();					    //初始化ESP8266
delay_ms(500);
while(OneNet_DevLink())			//接入OneNET
delay_ms(500);
	LED = LED_OFF;						    //入网成功，点亮板子LED
	  show();                     //OLED显示主界面	
	MX_GPIO_Init();
	max30102_init();
  Key_GPIO_Config();//按键端口初始化
	un_min=0x3FFFF;
	un_max=0;
	
	n_ir_buffer_length=500; //buffer length of 100 stores 5 seconds of samples running at 100sps
	//read the first 500 samples, and determine the signal range
    for(i=0;i<n_ir_buffer_length;i++)
    {
        while(MAX30102_INT==1);   //wait until the interrupt pin asserts
        
		max30102_FIFO_ReadBytes(REG_FIFO_DATA,temp);
		aun_red_buffer[i] =  (long)((long)((long)temp[0]&0x03)<<16) | (long)temp[1]<<8 | (long)temp[2];    // Combine values to get the actual number
		aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03)<<16) |(long)temp[4]<<8 | (long)temp[5];   // Combine values to get the actual number
            
        if(un_min>aun_red_buffer[i])
            un_min=aun_red_buffer[i];    //update signal min
        if(un_max<aun_red_buffer[i])
            un_max=aun_red_buffer[i];    //update signal max
    }
	un_prev_data=aun_red_buffer[i];
	//calculate heart rate and SpO2 after first 500 samples (first 5 seconds of samples)
    maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid); 
USART3_SendString("AF:30\r\n"); // 通过串口3发送数据	

	while(1) 
	{


        sprintf((char *)buf4, "Y:%02d:%02d:%02d:%02d", YL1, YL2, YL3, YL4);
        OLED_ShowStr(0, 0, buf4, 2); // OLED第2行显示时间（位置可自定义）




 


 
        sprintf((char *)buf7, "t:%02d:%02d:%02d", shi, fen, miao);
        OLED_ShowStr(0, 7, buf7, 1); // OLED第2行显示时间（位置可自定义）
        // ==================== OLED实时显示更新 ====================
// 
//        sprintf((char *)buf9, "S:%d ", yz);
//        OLED_ShowStr(64, 2, buf9, 2);
//        // 系统时间显示（时:分:秒，补0优化）
  
        // 1号预约时间显示
        sprintf((char *)buf8, "t1:%02d:%02d", shi1, fen1);
        OLED_ShowStr(0, 5, buf8, 1);
        // 2号预约时间显示
        sprintf((char *)buf8, "t2:%02d:%02d", shi2, fen2);
        OLED_ShowStr(70, 5, buf8, 1);
        // 3号预约时间显示
        sprintf((char *)buf8, "t3:%02d:%02d", shi3, fen3);
        OLED_ShowStr(0, 6, buf8, 1);
        // 4号预约时间显示
        sprintf((char *)buf8, "t4:%02d:%02d", shi4, fen4);
        OLED_ShowStr(70, 6, buf8, 1);

        // ==================== 预约时间匹配：触发PC13 LED点亮 ====================
        if((fen==fen1 && shi==shi1 && miao==miao1 && YL1>0) ||
           (fen==fen2 && shi==shi2 && miao==miao2 && YL2>0) ||
           (fen==fen3 && shi==shi3 && miao==miao3 && YL3>0) ||
           (fen==fen4 && shi==shi4 && miao==miao4 && YL4>0))
        {
 USART3_SendString("A7:00006\r\n"); // 通过串口3发送数据
    // 强节奏模式：3声快连响 + 中等间隔 + 2声快连响 + 长停顿
    // 电平定义：0=响/亮  1=停/灭  单次周期1200ms，节奏鲜明+时长适中
    PCout(13) = 0; delay_ms(70);  // 响1：70ms（短而促）
    PCout(13) = 1; delay_ms(30);  // 隔1：30ms（极短间隔，连响感）
    PCout(13) = 0; delay_ms(70);  // 响2：70ms
    PCout(13) = 1; delay_ms(30);  // 隔2：30ms
    PCout(13) = 0; delay_ms(70);  // 响3：70ms
    PCout(13) = 1; delay_ms(150); // 隔3：150ms（中等间隔，区分两段）
    PCout(13) = 0; delay_ms(70);  // 响4：70ms
    PCout(13) = 1; delay_ms(30);  // 隔4：30ms
    PCout(13) = 0; delay_ms(70);  // 响5：70ms
    PCout(13) = 1; delay_ms(550); // 长停顿：550ms（循环间隔，不刺耳）
        }
//=====================================温湿度模块===================================================	
//	  if(DHT11_Read_Data(&temperature,&humidity) == 0)
//		{
//			sprintf((char *)buf1,"T:%dC",temperature);
//			sprintf((char *)buf2,"H:%d%%",humidity);
//			OLED_ShowStr(0,0,buf1,1);
//			OLED_ShowStr(50,0,buf2,1);
//		}
		

	 
//	sprintf((char *)buf10,"WDH:%d",WDH);
//	OLED_ShowStr(0,1,buf10,1);
//	sprintf((char *)buf10,"SDH:%d",SD);
//	OLED_ShowStr(0,2,buf10,1);

//if(temperature<=WDH)//启动加湿器
//{
//  
//PCout(15)=0;
// 
//}
//else

//{
//PCout(15)=1;

//}
//if(temperature>=38)//温度异常
//{
// USART3_SendString("A7:00002\r\n"); // 通过串口3发送数据

//delay_ms(1000);//延时1秒		

// 	 PCout(14)=0;	
//}
//else
//{

// PCout(14)=1;	
//}		
 
////======================= 传感器数据的获取，通过ADC规则通道查询获取========		
//      for(i=0,value1=0,value2=0,value3=0;i<30;)
//		{
//			value1+=ADC_Value[i++];	
//			value2+=ADC_Value[i++];
//			value3+=ADC_Value[i++];
//		}			
//		Lighting=100-(4096-value1/10)*100/4096;  
//		Voice=100-(4096-value2/10)*100/4096;     
//		Gas=(value3/10)*100/4096;       
//		value1=0;
//		value2=0;
//		value3=0;
////		if(Lighting>50)

////{
////Lighting=0;
//}
//else
//{
//Lighting=1;
//}

if( Key_Scan1(GPIOB,GPIO_Pin_5) == KEY_ON  )	 //手动呼叫
{	
Voice=Voice+1;
}
 if(Voice>=2)
{
Voice=0;
}

		sprintf((char *)buf4,"H:%d ",	Voice);
	  OLED_ShowStr(0,2,buf4,1);//OLED显示呼叫

// 保留你原有的计数和按键判断逻辑，仅修改YLx的赋值部分
if( Key_Scan1(GPIOB,GPIO_Pin_4) == KEY_ON  )	 //判断KEY1是否按下
{			
    sz=sz+1;
// 按键1次：YL1=0；2次：YL2=0；3次：YL3=0；4次：YL4=0；0次（重置）：全部1
if(sz == 1)
{
    YL1=0;
 
}
else if(sz == 2)
{
    YL2=0;
 
}
else if(sz == 3)
{
    YL3=0;
 
}
else if(sz == 4)
{
    YL4=0;
 
}
}

if(sz>=5)  // 计数到5重置为0
{
    sz=0;
}

 






// 按键检测核心逻辑
if( Key_Scan1(GPIOB,GPIO_Pin_0) == KEY_ON )	 // 判断KEY1是否按下
{			
    temperature = 1;
    key_off_count = 0;  // 按下时重置计数（关键：消除之前的OFF计数）
}
else  // 按键未按下（KEY_OFF）
{
    // 每次检测到OFF，计数+1（先判断是否未到5次，避免溢出）
    if(key_off_count < 2)
    {
        key_off_count++;
    }
    
    // 延时消抖（500ms，保持原有延时逻辑）
    delay_ms(100);
    
    // 再次确认当前还是OFF，且计数达到5次
    if( Key_Scan1(GPIOB,GPIO_Pin_0) == KEY_OFF && key_off_count >= 2 )	 		
    {
        temperature = 0;
        key_off_count = 0;  // 重置计数，避免重复触发
    }
}
 

		sprintf((char *)buf3,"D:%d ",temperature);

		OLED_ShowStr(85,2,buf3,1);//OLED显示跌倒





if(temperature==0)//倾斜
{
PCout(13)=1;	
 PCout(14)=0;	

}
else 
{
 PCout(13)=0;	
PCout(14)=1;	

}	








//if(	Gas>=80)//烟雾报警,启动风扇
//{
//// USART3_SendString("A7:00006\r\n"); // 通过串口3发送数据
//PCout(13)=0;
//delay_ms(100);//延时1秒		
//PCout(13)=1;
// 
//}
//else

//{
// 
//}
//if(	humidity<=SD)// 湿度
//{

// PBout(13)=1;	
//PCout(14)=1;	
//}
//else

//{
// PBout(13)=0;	
//PCout(14)=0;	

//}







//if(Lighting==0)//跌倒报警
//{
//USART3_SendString("A7:00004\r\n"); // 通过串口3发送数据
//PCout(13)=0;
//delay_ms(100); 	
//PCout(13)=1;
//}

		 
//if( Key_Scan1(GPIOB,GPIO_Pin_12) == KEY_ON  )	 //判断KEY1是否按下
//{			
//Voice=1;//人异常
//// USART3_SendString("A7:00001\r\n"); // 通过串口3发送数据
////PCout(13)=0;
////delay_ms(1000);//延时1秒		
////PCout(13)=1;

//}
//else

//{
//Voice=0;//人正常

//}
//	sprintf((char *)buf3,"R:%d ",Voice);
//	OLED_ShowStr(0,4,buf3,1);//OLED显示跌倒		






//			if(Key_Scan(GPIOB, GPIO_Pin_4) == KEY_ON)
//				{
//						yz++;
//						if(yz >=2) yz = 0;  
//				}









///////////////////////////
//if( Key_Scan1(GPIOB,GPIO_Pin_5) == KEY_ON &&yz == 0)	 //判断KEY1是否按下
//{			
//SD=SD+1;                 //温度高值+
//if(SD>=90)
//{
////  	delay_ms(10);
//SD=10;
//}
//}

// if( Key_Scan1(GPIOB,GPIO_Pin_15) == KEY_ON &&yz == 0)	 //判断KEY1是否按下
//{	
//SD=SD-1; 

//if(SD<10)
//{
////  	delay_ms(10);
//SD=90;
//}

//}



//if( Key_Scan1(GPIOB,GPIO_Pin_5) == KEY_ON &&yz == 1)	 //判断KEY1是否按下
//{			
//WDH=WDH+1;                 //温度高值+
//if(WDH>=90)
//{
////  	delay_ms(10);
//WDH=10;
//}
//}

// if( Key_Scan1(GPIOB,GPIO_Pin_15) == KEY_ON &&yz == 1)	 //判断KEY1是否按下
//{	
//WDH=WDH-1; 

//if(WDH<10)
//{
////  	delay_ms(10);
//WDH=90;
//}

//}



////////////////////////////////////////////////心率/////////////////
	    i=0;
      un_min=0x3FFFF;
      un_max=0;
		//dumping the first 100 sets of samples in the memory and shift the last 400 sets of samples to the top
			for(i=100;i<500;i++)
			{
					aun_red_buffer[i-100]=aun_red_buffer[i];
					aun_ir_buffer[i-100]=aun_ir_buffer[i];
					
					//update the signal min and max
					if(un_min>aun_red_buffer[i])
					un_min=aun_red_buffer[i];
					if(un_max<aun_red_buffer[i])
					un_max=aun_red_buffer[i];
			}
		//take 100 sets of samples before calculating the heart rate.
        for(i=400;i<500;i++)
        {
            un_prev_data=aun_red_buffer[i-1];
            while(MAX30102_INT==1);
            max30102_FIFO_ReadBytes(REG_FIFO_DATA,temp);
							aun_red_buffer[i] =  (long)((long)((long)temp[0]&0x03)<<16) | (long)temp[1]<<8 | (long)temp[2];    // Combine values to get the actual number
						aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03)<<16) |(long)temp[4]<<8 | (long)temp[5];   // Combine values to get the actual number
        
            if(aun_red_buffer[i]>un_prev_data)
            {
                f_temp=aun_red_buffer[i]-un_prev_data;
                f_temp/=(un_max-un_min);
                f_temp*=MAX_BRIGHTNESS;
                n_brightness-=(int)f_temp;
                if(n_brightness<0)
                    n_brightness=0;
            }
            else
            {
                f_temp=un_prev_data-aun_red_buffer[i];
                f_temp/=(un_max-un_min);
                f_temp*=MAX_BRIGHTNESS;
                n_brightness+=(int)f_temp;
                if(n_brightness>MAX_BRIGHTNESS)
                    n_brightness=MAX_BRIGHTNESS;
            }
			if((ch_hr_valid == 1) )//**/ ch_hr_valid == 1 && ch_spo2_valid ==1 && n_heart_rate<120 && n_sp02<101
			{			
				if(n_heart_rate	<120)		
					dis_hr = n_heart_rate;
				else
					dis_hr=0;
				dis_spo2 = n_sp02;				
			}
			else
			{
				
				dis_hr = 0;
				dis_spo2 = 0;
			}										
			t++;
			if(t>10)
			{
				t=0;
			}

		}
maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);

if(dis_hr==0)

{
dis_spo2=0;
}

if(dis_spo2<=0) 

{dis_spo2=0;

}
if( dis_hr>0 )	 //判断KEY1是否按下
{			
sprintf((char *)buf7,"xin:%.0f;%.0f    ",dis_hr,dis_spo2);
OLED_ShowStr(0,3,buf7,1);
dis_hr1=dis_hr; 
dis_spo21=dis_spo2; //血氧数据



}

//if( Key_Scan1(GPIOB,GPIO_Pin_4) == KEY_ON )	 //判断KEY1是否按下
//{	



//if(dis_hr1>100||dis_hr1<60)
//{
// USART3_SendString("A7:00003\r\n"); // 通过串口3发送数据
//	delay_ms(1000);//延时1秒		
//}
//if(dis_spo2<60)
//{
// USART3_SendString("A7:00005\r\n"); // 通过串口3发送数据
//	delay_ms(1000);//延时1秒		
//}

//	}



 // ==================== OneNET云平台数据上传（间隔约3S）====================
        timeCount ++;
        if(timeCount >= 10) // 主循环约10ms/次，300次≈3S
        {
            timeCount = 0;
             OneNet_SendData();    // 发送第一组数据（开启网络时取消注释）
             ESP8266_Clear();      // 清空ESP8266接收缓存
             delay_ms(200);
             OneNet_SendDataa();   // 发送第二组数据
            ESP8266_Clear();      // 清空ESP8266接收缓存
        }

        // ==================== 接收OneNET服务器下发数据并解析 ====================
        dataPtr = ESP8266_GetIPD(0);
        if(dataPtr != NULL)
        {
            OneNet_RevPro(dataPtr); // 解析服务器下发指令
        }

	}	
}



















