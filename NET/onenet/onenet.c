//单片机头文件
#include "stm32f10x.h"
#include "delay.h"
//网络设备
#include "esp8266.h"

//协议文件
#include "onenet.h"
#include "mqttkit.h"

//硬件驱动
#include "usart.h"
#include "oled.h"
#include "led.h"
//C库
#include <string.h>
#include <stdio.h>


#define PROID		"634423"   //产品ID

#define AUTH_INFO	"5126241"		//鉴权信息

#define DEVID		"1215192187"	//设备ID
//EgsUletm3cvkWr=UrC6sgzyG2oM=
//129


extern unsigned char esp8266_buf[128];
extern u8 temperature;
extern u8 humidity; 
extern u8 LED_value;
extern u8 Lighting,Voice,Gas; 
extern  u8 fan;
extern  u8 dis_hr1;   //心率数据
extern  u8 dis_spo21; //血氧数据
extern  u8 fscs; 
extern u8  miao,fen,shi;
extern u8  fen1,shi1;
extern u8  fen2,shi2;
extern u8  fen3,shi3;
extern u8  fen4,shi4;
extern u8 WDH;                    
extern u8 SD;     
extern u8 YL1; 
extern u8 YL2; 
extern u8 YL3; 
extern u8 YL4; 
// 假设添加了全局变量用于存储十进制经纬度
extern float longitude;
extern float latitude;
//==========================================================
//	函数名称：	OneNet_DevLink
//
//	函数功能：	与onenet创建连接
//
//	入口参数：	无
//
//	返回参数：	1-成功	0-失败
//
//	说明：		与onenet平台建立连接
//==========================================================
_Bool OneNet_DevLink(void)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};					//协议包

	unsigned char *dataPtr;
	
	_Bool status = 1;
	OLED_CLS();//OLED清屏
  OLED_ShowCN(15,2,15);OLED_ShowCN(31,2,16);OLED_ShowCN(47,2,17);OLED_ShowCN(63,2,13);OLED_ShowCN(79,2,14);OLED_ShowCN(95,2,10);//oled显示“服务器连接中”
	//printf("OneNet_DevLink\r\nPROID: %s,	AUIF: %s,	DEVID:%s\r\n", PROID, AUTH_INFO, DEVID);
	
	if(MQTT_PacketConnect(PROID, AUTH_INFO, DEVID, 256, 0, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);			//上传平台	
		dataPtr = ESP8266_GetIPD(250);									//等待平台响应
		if(dataPtr != NULL)
		{
			if(MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
			{
				switch(MQTT_UnPacketConnectAck(dataPtr))
				{
					case 0://printf("Tips:	连接成功\r\n");
status = 0;
					       OLED_CLS();//OLED清屏
					       OLED_ShowCN(31,4,13);OLED_ShowCN(47,4,14);OLED_ShowCN(63,4,12);OLED_ShowCN(79,4,18);//oled显示“连接成功”	
                 delay_ms(500);					
					       break;
					case 1://printf("WARN:	连接失败：协议错误\r\n");
break;
					case 2://printf("WARN:	连接失败：非法的clientid\r\n");
break;
					case 3://printf("WARN:	连接失败：服务器失败\r\n");
break;
					case 4://printf("WARN:	连接失败：用户名或密码错误\r\n");
break;
					case 5://printf("WARN:	连接失败：非法链接(比如token非法)\r\n");
break;
					
					default:
//printf("ERR:	连接失败：未知错误\r\n");
break;
				}
			}
		}
		
		MQTT_DeleteBuffer(&mqttPacket);								//删包
	}
	else
		//printf("WARN:	MQTT_PacketConnect Failed\r\n");
	  OLED_ShowCN(47,4,26);OLED_ShowCN(63,4,27);OLED_ShowCN(47,6,28);OLED_ShowCN(63,6,29);//链接失败，重连
	return status;
	
}


unsigned char OneNet_FillB(char *buf)
{
	char text1[32];
	
	memset(text1, 0, sizeof(text1));
	
	strcpy(buf, ",;");
	//============================向onenet发送数据==============================================================			



	memset(text1, 0, sizeof(text1));
	sprintf(text1, "shi1,%d;", shi1);          
	strcat(buf, text1);

	memset(text1, 0, sizeof(text1));
	sprintf(text1, "fen1,%d;", fen1);         
	strcat(buf, text1);

	memset(text1, 0, sizeof(text1));
	sprintf(text1, "shi2,%d;", shi2);          
	strcat(buf, text1);

	memset(text1, 0, sizeof(text1));
	sprintf(text1, "fen2,%d;", fen2);         
	strcat(buf, text1);

	
	memset(text1, 0, sizeof(text1));
	sprintf(text1, "shi33,%d;", shi3);          
	strcat(buf, text1);

	memset(text1, 0, sizeof(text1));
	sprintf(text1, "fen3,%d;", fen3);         
	strcat(buf, text1);

		memset(text1, 0, sizeof(text1));
	sprintf(text1, "shi4,%d;", shi4);          
	strcat(buf, text1);

	memset(text1, 0, sizeof(text1));
	sprintf(text1, "fen4,%d;", fen4);         
	strcat(buf, text1);

	return strlen(buf);
}

/////////////////////////

//==========================================================
//	函数名称：	OneNet_SendData
//
//	函数功能：	上传数据到平台
//
//	入口参数：	type：发送数据的格式
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void OneNet_SendDataa(void)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};												//协议包
	
	char buf1[128];
	
	short body_len = 0, i = 0;
	
//	printf("Tips:	OneNet_SendData-MQTT\r\n");
	
	memset(buf1, 0, sizeof(buf1));
	
	body_len = OneNet_FillB(buf1);																	//获取当前需要发送的数据流的总长度
	
	if(body_len)
	{
		if(MQTT_PacketSaveData(DEVID, body_len, NULL, 5, &mqttPacket) == 0)							//封包
		{
			for(; i < body_len; i++)
				mqttPacket._data[mqttPacket._len++] = buf1[i];
			
			ESP8266_SendData(mqttPacket._data, mqttPacket._len);									//上传数据到平台
//			printf("Send %d Bytes\r\n", mqttPacket._len);
			
			MQTT_DeleteBuffer(&mqttPacket);															//删包
		}
//		else
//			printf("WARN:	EDP_NewBuffer Failed\r\n");
	}
	
}



unsigned char OneNet_FillBuf(char *buf)
{
	char text[32];
	
	memset(text, 0, sizeof(text));
	
	strcpy(buf, ",;");
	//============================向onenet发送数据==============================================================			

	memset(text, 0, sizeof(text));
	sprintf(text, "temperature,%02d:%02d:%02d:%02d;",  YL1, YL2, YL3, YL4);         
	strcat(buf, text);

	memset(text, 0, sizeof(text));
	sprintf(text, "humidity,H:%d D:%d R:%d;", humidity,temperature,Voice);         
	strcat(buf, text);

	memset(text, 0, sizeof(text));
	sprintf(text, "Gas,A%d B%d C%d;", Gas,dis_hr1,dis_spo21);  
	strcat(buf, text);
 

	
	
 
	
	
//	memset(text, 0, sizeof(text));
//	sprintf(text, "Voice,%d;", Lighting);  //人体       
//	strcat(buf, text);	
	
	
	
	
	memset(text, 0, sizeof(text));
	sprintf(text, "shi,%d;", shi);  
	strcat(buf, text);
		
	memset(text, 0, sizeof(text));
	sprintf(text, "fen,%d;", fen);         
	strcat(buf, text);	
//		memset(text, 0, sizeof(text));
//	sprintf(text, "fen5,%d;", fen5);         
//	strcat(buf, text);	
	return strlen(buf);
}


//==========================================================
//	函数名称：	OneNet_SendData
//
//	函数功能：	上传数据到平台
//
//	入口参数：	type：发送数据的格式
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void OneNet_SendData(void)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};												//协议包
	
	char buf[128];
	
	short body_len = 0, i = 0;
	
//	printf("Tips:	OneNet_SendData-MQTT\r\n");
	
	memset(buf, 0, sizeof(buf));
	
	body_len = OneNet_FillBuf(buf);																	//获取当前需要发送的数据流的总长度
	
	if(body_len)
	{
		if(MQTT_PacketSaveData(DEVID, body_len, NULL, 5, &mqttPacket) == 0)							//封包
		{
			for(; i < body_len; i++)
				mqttPacket._data[mqttPacket._len++] = buf[i];
			
			ESP8266_SendData(mqttPacket._data, mqttPacket._len);									//上传数据到平台
//			printf("Send %d Bytes\r\n", mqttPacket._len);
			
			MQTT_DeleteBuffer(&mqttPacket);															//删包
		}
//		else
//			printf("WARN:	EDP_NewBuffer Failed\r\n");
	}
	
}

//==========================================================
//	函数名称：	OneNet_RevPro
//
//	函数功能：	平台返回数据检测
//
//	入口参数：	dataPtr：平台返回的数据
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void OneNet_RevPro(unsigned char *cmd)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};								//协议包
	
	char *req_payload = NULL;
	char *cmdid_topic = NULL;
	
	unsigned short req_len = 0;
	
	unsigned char type = 0;
	
	short result = 0;

	char *dataPtr = NULL;
	char numBuf[10];
	int num = 0;
	
	type = MQTT_UnPacketRecv(cmd);
	switch(type)
	{
		case MQTT_PKT_CMD:															//命令下发
			
			result = MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len);	//解出topic和消息体
			if(result == 0)
			{
			//	printf("cmdid: %s, req: %s, req_len: %d\r\n", cmdid_topic, req_payload, req_len);
				
				if(MQTT_PacketCmdResp(cmdid_topic, req_payload, &mqttPacket) == 0)	//命令回复组包
				{
		//		printf("Tips:	Send CmdResp\r\n");
					
					ESP8266_SendData(mqttPacket._data, mqttPacket._len);			//回复命令
					MQTT_DeleteBuffer(&mqttPacket);									//删包
				}
			}
		
		break;
			
		case MQTT_PKT_PUBACK:														//发送Publish消息，平台回复的Ack
		
			if(MQTT_UnPacketPublishAck(cmd) == 0)
//				printf("Tips:	MQTT Publish Send OK\r\n");
			
		break;
		
		default:
			result = -1;
		break;
	}
	
	ESP8266_Clear();									//清空缓存
	
	if(result == -1)
		return;
	
	dataPtr = strchr(req_payload, ':');					//搜索':'

	if(dataPtr != NULL && result != -1)					//如果找到了
	{
		dataPtr++;
		
		while(*dataPtr >= '0' && *dataPtr <= '9')		//判断是否是下发的命令控制数据
		{
			numBuf[num++] = *dataPtr++;
		}
		numBuf[num] = 0;
		
		num = atoi((const char *)numBuf);			  //转为数值形式
		
//============================接受onenet发来的数据=================================================================	
	if(strstr((char *)req_payload, "K1OFF"))//搜索"LED_value"  
	{
 
	}
	else if(strstr((char *)req_payload, "K1ON"))		//搜索"fan"
	{
 
	}
	else if(strstr((char *)req_payload, "K2ON"))		//搜索其他 你设置的变量 以此类推  继续添加你想添加的功能
	{

 
	}
	
	
	
	
	else if(strstr((char *)req_payload, "K2OFF"))		//搜索其他 你设置的变量 以此类推  继续添加你想添加的功能
	{


	}
	else if(strstr((char *)req_payload, "K3ON"))		//搜索其他 你设置的变量 以此类推  继续添加你想添加的功能
	{
	printf("A7:00005\r\n");//打印吃药语言播报的串口
	}
	else if(strstr((char *)req_payload, "MSkk"))		//搜索其他 你设置的变量 以此类推  继续添加你想添加的功能
	{
	
	}

	else if(strstr((char *)req_payload, "K4ON"))		//搜索其他 你设置的变量 以此类推  继续添加你想添加的功能
	{

	printf("A7:00003\r\n");//打印吃药语言播报的串口
	}
	else if(strstr((char *)req_payload, "K4OFF"))		//搜索其他 你设置的变量 以此类推  继续添加你想添加的功能
	{

	}

	else if(strstr((char *)req_payload, "K5OFF"))		//搜索其他 你设置的变量 以此类推  继续添加你想添加的功能
	{

	}
	else if(strstr((char *)req_payload, "K5ON"))		//搜索其他 你设置的变量 以此类推  继续添加你想添加的功能
	{
	printf("A7:00004\r\n");//打印吃药语言播报的串口
	}


/////////////////////上位机设置时间1

// ==================== OneNET远程绑定式时间设置（时分一体，不可分开设置）====================
// 系统时间绑定设置：关键字sysTime，格式 sysTime=HHMM （HH=时 0-23，MM=分 0-59，例：18时30分 → sysTime=1830）
else if (strstr((char *)req_payload, "sysTime") && strchr((char *)req_payload, '=')) 
{
    char *eqPos = strchr(req_payload, '=');
    char *valueStart = eqPos + 1;
    if (*valueStart == '}') valueStart++;
    u16 timeVal = (u16)atoi(valueStart); // 读取组合值（如1830）
    // 拆分时和分：高两位=时，低两位=分
    shi = timeVal / 100;  // 1830/100 = 18（时）
    fen = timeVal % 100;  // 1830%100 = 30（分）
    // 越界保护：时0-23，分0-59，越界则不更新
    if(shi >=24) shi = 0;
    if(fen >=60) fen = 0;
    miao = 0; // 可选：设置时间时秒归0，提升时间准确性
    PCout(13) = 0;
delay_ms(100);
    PCout(13) = 1; // 配置成功，LED短亮提示
}
// 1号预约时间绑定设置：关键字yt1Time，格式 yt1Time=HHMM （例：20时15分 → yt1Time=2015）
else if (strstr((char *)req_payload, "yt1Time") && strchr((char *)req_payload, '=')) 
{
    char *eqPos = strchr(req_payload, '=');
    char *valueStart = eqPos + 1;
    if (*valueStart == '}') valueStart++;
    u16 timeVal = (u16)atoi(valueStart);
    shi1 = timeVal / 100;
    fen1 = timeVal % 100;
    if(shi1 >=24) shi1 = 0;
    if(fen1 >=60) fen1 = 0;
    PCout(13) = 0;
	delay_ms(100);
    PCout(13) = 1;
}
// 2号预约时间绑定设置：关键字yt2Time，格式 yt2Time=HHMM
else if (strstr((char *)req_payload, "yt2Time") && strchr((char *)req_payload, '=')) 
{
    char *eqPos = strchr(req_payload, '=');
    char *valueStart = eqPos + 1;
    if (*valueStart == '}') valueStart++;
    u16 timeVal = (u16)atoi(valueStart);
    shi2 = timeVal / 100;
    fen2 = timeVal % 100;
    if(shi2 >=24) shi2 = 0;
    if(fen2 >=60) fen2 = 0;
    PCout(13) = 0;
	delay_ms(100);
    PCout(13) = 1;
}
// 3号预约时间绑定设置：关键字yt3Time，格式 yt3Time=HHMM
else if (strstr((char *)req_payload, "yt3Time") && strchr((char *)req_payload, '=')) 
{
    char *eqPos = strchr(req_payload, '=');
    char *valueStart = eqPos + 1;
    if (*valueStart == '}') valueStart++;
    u16 timeVal = (u16)atoi(valueStart);
    shi3 = timeVal / 100;
    fen3 = timeVal % 100;
    if(shi3 >=24) shi3 = 0;
    if(fen3 >=60) fen3 = 0;
    PCout(13) = 0;
	delay_ms(100);
    PCout(13) = 1;
}
// 4号预约时间绑定设置：关键字yt4Time，格式 yt4Time=HHMM
else if (strstr((char *)req_payload, "yt4Time") && strchr((char *)req_payload, '=')) 
{
    char *eqPos = strchr(req_payload, '=');
    char *valueStart = eqPos + 1;
    if (*valueStart == '}') valueStart++;
    u16 timeVal = (u16)atoi(valueStart);
    shi4 = timeVal / 100;
    fen4 = timeVal % 100;
    if(shi4 >=24) shi4 = 0;
    if(fen4 >=60) fen4 = 0;
    PCout(13) = 0;
	delay_ms(100);
    PCout(13) = 1;
}
		
		
			else if(strstr((char *)req_payload, "YL1JIA"))		//搜索其他 你设置的变量 以此类推  继续添加你想添加的功能
		{
	 YL1=YL1+1;
   if( YL1>=9)
		{
		 YL1=9;
		}
		}
		else if(strstr((char *)req_payload, "YL1L"))		//搜索其他 你设置的变量 以此类推  继续添加你想添加的功能
		{
		YL1=YL1-1;
		if(YL1<=1)
		{
		YL1=1;
		}
		}
		
		///
					else if(strstr((char *)req_payload, "YL2JIA"))		//搜索其他 你设置的变量 以此类推  继续添加你想添加的功能
		{
	 YL2=YL2+1;
   if( YL2>=9)
		{
		 YL2=9;
		}
		}
		else if(strstr((char *)req_payload, "YL2L"))		//搜索其他 你设置的变量 以此类推  继续添加你想添加的功能
		{
		YL2=YL2-1;
		if(YL2<=1)
		{
		YL2=1;
		}
		}
		
		
				///3
					else if(strstr((char *)req_payload, "YL3JIA"))		//搜索其他 你设置的变量 以此类推  继续添加你想添加的功能
		{
	 YL3=YL3+1;
   if( YL3>=9)
		{
		 YL3=9;
		}
		}
		else if(strstr((char *)req_payload, "YL3L"))		//搜索其他 你设置的变量 以此类推  继续添加你想添加的功能
		{
		YL3=YL3-1;
		if(YL3<=1)
		{
		YL3=1;
		}
		}
		
			
				///4
					else if(strstr((char *)req_payload, "YL4JIA"))		//搜索其他 你设置的变量 以此类推  继续添加你想添加的功能
		{
	 YL4=YL4+1;
   if( YL4>=9)
		{
		 YL4=9;
		}
		}
		else if(strstr((char *)req_payload, "YL4L"))		//搜索其他 你设置的变量 以此类推  继续添加你想添加的功能
		{
		YL4=YL4-1;
		if(YL4<=1)
		{
		YL4=1;
		}
		}
		
		
			else if (strstr((char *)req_payload, "YL1") && strchr((char *)req_payload, '=')) //药量1设置
		{
			char *eqPos = strchr(req_payload, '=');
			char *valueStart = eqPos + 1;
			if (*valueStart == '}') valueStart++;
			YL1 = (u8)atoi(valueStart);
			PCout(13) = 0; delay_ms(200); PCout(13) = 1;
			 
		}
		else if (strstr((char *)req_payload, "YL2") && strchr((char *)req_payload, '=')) //药量2设置
		{
			char *eqPos = strchr(req_payload, '=');
			char *valueStart = eqPos + 1;
			if (*valueStart == '}') valueStart++;
			YL2 = (u8)atoi(valueStart);
			PCout(13) = 0; delay_ms(200); PCout(13) = 1;
 
		}
		
		else if (strstr((char *)req_payload, "YL3") && strchr((char *)req_payload, '=')) //药量3设置
		{
			char *eqPos = strchr(req_payload, '=');
			char *valueStart = eqPos + 1;
			if (*valueStart == '}') valueStart++;
			YL3 = (u8)atoi(valueStart);
			PCout(13) = 0; delay_ms(200); PCout(13) = 1;
 
		}		
		else if (strstr((char *)req_payload, "YL4") && strchr((char *)req_payload, '=')) //药量4设置
		{
			char *eqPos = strchr(req_payload, '=');
			char *valueStart = eqPos + 1;
			if (*valueStart == '}') valueStart++;
			YL4 = (u8)atoi(valueStart);
			PCout(13) = 0; delay_ms(200); PCout(13) = 1;
 
		}		
				
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
	}
//====================================================================================================================
	if(type == MQTT_PKT_CMD || type == MQTT_PKT_PUBLISH)
	{
		MQTT_FreeBuffer(cmdid_topic);
		MQTT_FreeBuffer(req_payload);
	}

}

