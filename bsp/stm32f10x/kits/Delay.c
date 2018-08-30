//#include <stm32f10x.h>
#include "Delay.h"

/**************************
*�ڲ���ʱ
***************************/

/*---------------------------------
��ʱus
-----------------------------------*/
/*---------------------------------
��ʱus
-----------------------------------*/
void Delay_ms(unsigned long u32us)
{
		unsigned long i;
		
		while(u32us--)
		{
				for(i=0;i<5000;i++);
		}	
}

void Delay_us(unsigned long u32us)
{
		unsigned char i;
		
		while(u32us--)
		{
//				for(i=0;i<9;i++);
				for(i=0;i<7;i++);
		}	
}

void Delay05us(void)
{
		unsigned char i;
		for(i=0; i<1; i++);
}



static volatile uint32_t TimingDelay = 0;

void Delay_sys(volatile unsigned long nTime)
{ 
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}

static u8  fac_us=0;
static u16 fac_ms=0;

void delay_init(void)	 
{

//	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	//ϵͳʱ�ӳ�ʼ����ѡ���ⲿʱ��  HCLK/8
	fac_us=SystemCoreClock/8000000;	//Ϊϵͳʱ�ӵ�1/8  
	fac_ms=(u16)fac_us*1000;//����ÿ��ms��Ҫ��systickʱ����   
}								    
	    								   
void delay_1us(uint32_t nus)//��ʱus
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; 	 
	SysTick->VAL=0x00;       
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;       
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;      
	SysTick->VAL =0X00;        
}

void delay_1ms(uint16_t nms)//��ʱms
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;
	SysTick->VAL =0x00;           
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;        
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;      
	SysTick->VAL =0X00;     
}

