#ifndef __LED_H
#define __LED_H	 
#define __LED1_H

#include "sys.h"

#define LED PCout(13)	// LED接口	
#define LED2 PCout(14)	// LED接口	
#define LED3 PCout(15)	// LED接口	

#define LED4 PBout(13)	// LED接口	
#define LED5 PBout(14)	// LED接口	
#define LED6 PBout(11)	// LED接口	

#define LED_OFF		1
#define LED_ON		0

void LED_Init(void);//初始化
			    
#endif
