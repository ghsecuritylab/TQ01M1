#include <rtthread.h>
#include "sys_conf.h"
#include "kits/fifo.h"
#include "alarms.h"
#include "local_status.h"
#include "global_var.h"
#include "req_execution.h"

#include "event_record.h"
#include "rtc_bsp.h"
#include "daq.h"
#include "dio_bsp.h"
#include "sys_status.h"
#include "team.h"
#include "sys_conf.h"
#include "led_bsp.h"
#include "password.h"
#include "calc.h"

//#define ACL_ENMODE_ENABLE	0x0000
//#define ACL_ENMODE_SUPPRESS	0x0001
//#define ACL_ENMODE_DISABLE	0x0002
//enable_mask
enum
{
	ACL_ENMODE_ENABLE=0,//ʹ��
	ACL_ENMODE_SUPPRESS,//����
	ACL_ENMODE_DISABLE,//��ֹ
	ACL_ENMODE_OTHER,//����
};

#define ACL_ENMODE_AUTO_RESET_ALARM     0X0004
#define ACL_ENMODE_HAND_RESET_ALARM     0X0000

#define MBM_DEV_STS_DEFAULT		0x8000

#define ACL_TEM_MAX 1400 //0.1��
#define ACL_TEM_MIN -280

#define ACL_HUM_MAX 1000 // 0.1%
#define ACL_HUM_MIN  0

#define IO_CLOSE 1
#define IO_OPEN 0

#define OVER_TIME_ACCURACY     	86400          //DATE



#define POWER_PARAM		0xFF80
#define POWER_DOT 0x7f
#define POWER_DOT_BIT 7


typedef struct alarm_acl_td
{
	uint16_t				  id;
	uint16_t 					state;
	uint16_t          alram_value;
	uint16_t					timeout;
	uint16_t 					enable_mask;
	uint16_t 					reset_mask;
	uint8_t           alarm_level;
	uint16_t          dev_type;
	uint16_t (*alarm_proc)(struct alarm_acl_td* str_ptr);	
	
}alarm_acl_status_st;


typedef enum
{
	SML_MIN_TYPE=0,
	THR_MAX_TYPE,
	OUT_MIN_MAX_TYPE,
	IN_MIN_MAX_TYPE,	
}Compare_type_st;


typedef struct
{
	uint32_t lock_time[3];//

	uint16_t last_state;
	char lock_flag;
	
}alarm_lock_st;

typedef struct
{
	char  cycle_flag;//�����ڴ�����־��
	uint16_t compress_state;//1\2ѹ����״̬��
	uint32_t start_time[10];//ѹ����1��2������ʱ��
	uint16 alarm_timer;
}compress_cycle_alarm_st;

#define  MAX_LOCK_CNT 6
typedef struct
{
	
 	uint8_t compress_high_flag[2]; //0��ʾѹ����1��ѹ���� 1��ʾѹ��������ѹ�澯
	uint16_t tem_sensor_fault;
	uint16_t hum_sensor_fault;
 	uint32_t fan_timer; // 1second++
	uint32_t compressor1_timer;//1second++
	uint32_t compressor2_timer;//1second++
	alarm_lock_st  alarm_lock[MAX_LOCK_CNT];
/*
	{0xffffffff,0xffffffff,0xffffffff},//ACL_HI_LOCK1                  0
	{0xffffffff,0xffffffff,0xffffffff},//ACL_HI_LOCK2                 1
	{0xffffffff,0xffffffff,0xffffffff},//ACL_LO_LOCK1                  2
	{0xffffffff,0xffffffff,0xffffffff},//ACL_LO_LOCK2                  3
	{0xffffffff,0xffffffff,0xffffffff},//ACL_EXTMP_LOCK1               4
	{0xffffffff,0xffffffff,0xffffffff},//ACL_EXTMP_LOCK2               5

*/
	compress_cycle_alarm_st cmpress_cycle_alarm[2];
	alarm_acl_status_st alarm_sts[ACL_TOTAL_NUM];
	
}alarm_st;


static alarm_st alarm_inst;
extern	sys_reg_st		g_sys; 

extern local_reg_st 				l_sys;	
static uint16_t io_calc(uint8_t data,uint8_t refer);
static uint16_t compare_calc(int16_t meter,int16_t min,int16_t max,Compare_type_st type);

//static uint16_t alarm_lock(uint16_t alarm_id);

static void   alarm_status_bitmap_op (uint8_t alarm_id,uint8_t option);


//��⺯��
static  uint16_t acl00(alarm_acl_status_st* acl_ptr);
static	uint16_t acl01(alarm_acl_status_st* acl_ptr);
static	uint16_t acl02(alarm_acl_status_st* acl_ptr);
static	uint16_t acl03(alarm_acl_status_st* acl_ptr);
static	uint16_t acl04(alarm_acl_status_st* acl_ptr);
static	uint16_t acl05(alarm_acl_status_st* acl_ptr);
static	uint16_t acl06(alarm_acl_status_st* acl_ptr);
static	uint16_t acl07(alarm_acl_status_st* acl_ptr);                 
static 	uint16_t acl08(alarm_acl_status_st* acl_ptr);                  
static 	uint16_t acl09(alarm_acl_status_st* acl_ptr);
static 	uint16_t acl10(alarm_acl_status_st* acl_ptr);                             
static	uint16_t acl11(alarm_acl_status_st* acl_ptr);
static	uint16_t acl12(alarm_acl_status_st* acl_ptr);
static	uint16_t acl13(alarm_acl_status_st* acl_ptr);
static	uint16_t acl14(alarm_acl_status_st* acl_ptr);                       
static	uint16_t acl15(alarm_acl_status_st* acl_ptr);
static	uint16_t acl16(alarm_acl_status_st* acl_ptr);
static	uint16_t acl17(alarm_acl_status_st* acl_ptr);
static	uint16_t acl18(alarm_acl_status_st* acl_ptr);
static	uint16_t acl19(alarm_acl_status_st* acl_ptr);                       
static	uint16_t acl20(alarm_acl_status_st* acl_ptr);


//�澯����ٲ�
static void alarm_arbiration(void);
static void alarm_bitmap_op(uint8_t component_bpos, uint8_t action);
static void alarm_bitmap_mask_op(uint8_t component_bpos, uint8_t action);

enum
{
		ALARM_ACL_ID_POS=			0,//�澯���
		ALARM_ACL_EN_MASK_POS		,	//�̵�������
		ALARM_ACL_RESET_POS     ,//�����ʽ
		AlARM_ACL_LEVEL_POS     ,//�澯�ȼ�
		ALARM_ACL_DEV_POS      ,//�澯����
		ALARM_ACL_MAX
};


#define ALARM_FSM_INACTIVE			0x0001	
#define ALARM_FSM_PREACTIVE			0x0002
#define ALARM_FSM_ACTIVE				0x0003	
#define ALARM_FSM_POSTACTIVE		0x0004	
#define ALARM_FSM_ERROR					0x0005	

#define ALARM_ACL_TRIGGERED			0x0001
#define ALARM_ACL_CLEARED			0x0000
#define ALARM_ACL_HOLD              0x0002





//uint16_t alarm_tem_erro,alarm_hum_erro;

static uint16_t (* acl[ACL_TOTAL_NUM])(alarm_acl_status_st*) = 
{
	
//�ط���ͷ籨��(�¶Ⱥ�ʪ��)
		acl00,//		ACL_E0	
		acl01,//		ACL_E1
		acl02,//		ACL_E2	
		acl03,//		ACL_E3		
		acl04,//	
		acl05,//		
		acl06,//	
		acl07,//		
		acl08,//	
		acl09,//		
		acl10,//	
		acl11,//	
		acl12,//	
		acl13,//
		acl14,//	
		acl15,//
		acl16,//	
		acl17,//	
		acl18,//
		acl19,//	
		acl20,//
	
};

#define DEV_TYPE_COMPRESSOR 0x0000
#define DEV_TYPE_FAN        0x0400
#define DEV_TYPE_OUT_FAN    0x0800
#define DEV_TYPE_HEATER     0x0c00
#define DEV_TYPE_HUM        0x1000
#define DEV_TYPE_POWER      0x1400
#define DEV_TYPE_TEM_SENSOR 0x1800
#define DEV_TYPE_WATER_PUMP 0x1c00
#define DEV_TYPE_OTHER      0x3c00


const uint16_t ACL_CONF[ACL_TOTAL_NUM][ALARM_ACL_MAX]=
//	id ,en_mask,reless_mask,DEV_type
{		
	  0,			ACL_ENMODE_OTHER,		ACL_ENMODE_AUTO_RESET_ALARM,  CRITICAL_ALARM_lEVEL,     DEV_TYPE_OTHER,//ACL_E0								
		1,			ACL_ENMODE_OTHER,		ACL_ENMODE_AUTO_RESET_ALARM,  CRITICAL_ALARM_lEVEL,     DEV_TYPE_OTHER,//ACL_E1						
		2,			ACL_ENMODE_OTHER,		ACL_ENMODE_AUTO_RESET_ALARM,  CRITICAL_ALARM_lEVEL,     DEV_TYPE_OTHER,//ACL_E2						
		3,			ACL_ENMODE_OTHER,		ACL_ENMODE_AUTO_RESET_ALARM,  CRITICAL_ALARM_lEVEL,     DEV_TYPE_OTHER,//ACL_E3		
		4,			ACL_ENMODE_OTHER,		ACL_ENMODE_AUTO_RESET_ALARM,  CRITICAL_ALARM_lEVEL,     DEV_TYPE_OTHER,//ACL_E4	
    5,			ACL_ENMODE_OTHER,		ACL_ENMODE_AUTO_RESET_ALARM,  CRITICAL_ALARM_lEVEL,     DEV_TYPE_OTHER,//ACL_E5
		6,			ACL_ENMODE_OTHER,		ACL_ENMODE_AUTO_RESET_ALARM,  CRITICAL_ALARM_lEVEL,     DEV_TYPE_OTHER,//ACL_E6		
		7,			ACL_ENMODE_OTHER,		ACL_ENMODE_AUTO_RESET_ALARM,  CRITICAL_ALARM_lEVEL,     DEV_TYPE_OTHER,//ACL_E7			
		8,			ACL_ENMODE_OTHER,		ACL_ENMODE_AUTO_RESET_ALARM,  CRITICAL_ALARM_lEVEL,     DEV_TYPE_OTHER,//ACL_E8						
		9,			ACL_ENMODE_OTHER,		ACL_ENMODE_AUTO_RESET_ALARM,  CRITICAL_ALARM_lEVEL,     DEV_TYPE_OTHER,//ACL_RESERVE_01
		10,			ACL_ENMODE_OTHER,		ACL_ENMODE_AUTO_RESET_ALARM,  CRITICAL_ALARM_lEVEL,     DEV_TYPE_OTHER,//ACL_FAN01_OD	
		11,			ACL_ENMODE_OTHER,		ACL_ENMODE_AUTO_RESET_ALARM,  CRITICAL_ALARM_lEVEL,     DEV_TYPE_OTHER,//ACL_HI_PRESS1			
		12,			ACL_ENMODE_OTHER,		ACL_ENMODE_AUTO_RESET_ALARM,  CRITICAL_ALARM_lEVEL,     DEV_TYPE_POWER,//ACL_HI_PRESS2		
		13,			ACL_ENMODE_OTHER,		ACL_ENMODE_AUTO_RESET_ALARM,  MIOOR_ALARM_LEVEL,        DEV_TYPE_OTHER,//ACL_FILTER_OT
		14,			ACL_ENMODE_OTHER,		ACL_ENMODE_AUTO_RESET_ALARM,  MIOOR_ALARM_LEVEL,        DEV_TYPE_OTHER,//ACL_FILTER_ELEMENT_0_OT		
		15,			ACL_ENMODE_OTHER,		ACL_ENMODE_AUTO_RESET_ALARM,  MIOOR_ALARM_LEVEL,        DEV_TYPE_OTHER,//ACL_FILTER_ELEMENT_1_OT		
		16,			ACL_ENMODE_OTHER,		ACL_ENMODE_AUTO_RESET_ALARM,  MIOOR_ALARM_LEVEL,        DEV_TYPE_OTHER,//ACL_FILTER_ELEMENT_2_OT		
		17,			ACL_ENMODE_OTHER,		ACL_ENMODE_AUTO_RESET_ALARM,  MIOOR_ALARM_LEVEL,        DEV_TYPE_OTHER,//ACL_FILTER_ELEMENT_3_OT		
		18,			ACL_ENMODE_OTHER,		ACL_ENMODE_AUTO_RESET_ALARM,  MIOOR_ALARM_LEVEL,        DEV_TYPE_OTHER,//ACL_FILTER_ELEMENT_4_OT		
		19,			ACL_ENMODE_OTHER,		ACL_ENMODE_AUTO_RESET_ALARM,  MIOOR_ALARM_LEVEL,        DEV_TYPE_OTHER,//ACL_UV1_OT		
		20,			ACL_ENMODE_OTHER,		ACL_ENMODE_AUTO_RESET_ALARM,  MIOOR_ALARM_LEVEL,        DEV_TYPE_OTHER,//ACL_UV2_OT	
};
/*
  * @brief  alarm data structure initialization
	* @param  none
  * @retval none
  */


static void init_alarm(alarm_st* alarm_spr) 
{
		uint16 i;
		
		//��ʼACL 
		for(i=0;i<ACL_TOTAL_NUM;i++)
		{
			
			alarm_spr->alarm_sts[i].timeout =  0;
			alarm_spr->alarm_sts[i].state = ALARM_FSM_INACTIVE;
			alarm_spr->alarm_sts[i].id = ACL_CONF[i][ALARM_ACL_ID_POS];
			alarm_spr->alarm_sts[i].enable_mask = ACL_CONF[i][ALARM_ACL_EN_MASK_POS];
			alarm_spr->alarm_sts[i].reset_mask = ACL_CONF[i][ALARM_ACL_RESET_POS];
			alarm_spr->alarm_sts[i].alarm_level = ACL_CONF[i][AlARM_ACL_LEVEL_POS];
			alarm_spr->alarm_sts[i].dev_type = ACL_CONF[i][ALARM_ACL_DEV_POS];
			alarm_spr->alarm_sts[i].alarm_proc = acl[i];
			alarm_spr->alarm_sts[i].alram_value=0xffff;
		}
		
		//��ʼ��lock�����
		for(i=0;i<MAX_LOCK_CNT;i++)
		{
			alarm_spr->alarm_lock[i].last_state = ALARM_FSM_INACTIVE;
			alarm_spr->alarm_lock[i].lock_flag = 0;
			alarm_spr->alarm_lock[i].lock_time[0] = 0xffffffff;
			alarm_spr->alarm_lock[i].lock_time[1] = 0xffffffff;
			alarm_spr->alarm_lock[i].lock_time[2] = 0xffffffff;
		}
		//��ʼ��ѹ������ ������
		//alarm_spr->cmpress_cycle_alarm[0].alarm_timer
		alarm_spr->cmpress_cycle_alarm[0].cycle_flag = 0;
		alarm_spr->cmpress_cycle_alarm[1].cycle_flag = 0;

		alarm_spr->cmpress_cycle_alarm[0].compress_state = COMPRESSOR_FSM_STATE_IDLE;
		alarm_spr->cmpress_cycle_alarm[1].compress_state = COMPRESSOR_FSM_STATE_IDLE;
		
		for(i=0;i<10;i++)
		{
				alarm_spr->cmpress_cycle_alarm[0].start_time[i] = 0xffffffff; 
		    alarm_spr->cmpress_cycle_alarm[1].start_time[i] = 0xffffffff; 
		}
		//��ѹ����������ʼ��
		alarm_spr->compress_high_flag[0] = 0;
		alarm_spr->compress_high_flag[1] = 0;
		//��ʼ��������ʱ��
		alarm_spr->fan_timer = 0;
		alarm_spr->compressor1_timer = 0;
		alarm_spr->compressor2_timer = 0;
		
		
}




void alarm_acl_init(void)
{
	uint8_t i;
	// ��ʼ����̬�ڴ����ռ�
	chain_init();
	init_alarm(&alarm_inst);
	//��ʼ���ֶ��������
	for(i=0;i<ALARM_TOTAL_WORD;i++)
	{
			g_sys.config.general.alarm_remove_bitmap[i] = 0;
	}
	
} 

//uint8_t clear_alarm(uint8_t alarm_id)
//{
//		uint8_t byte_offset,bit_offset,i;
//	
//		if(alarm_id == 0xFF)
//		{
//					for(i=0;i<ALARM_TOTAL_WORD;i++)
//					{
//						g_sys.config.general.alarm_remove_bitmap[i] = 0xffff;
//					}
//				
//				return(1);
//		}
//		if(alarm_id < ACL_TOTAL_NUM)
//		{
//				byte_offset = alarm_id >> 4;
//				bit_offset = alarm_id & 0x0f;
//				g_sys.config.general.alarm_remove_bitmap[byte_offset] |= 0x0001<< bit_offset;
//			  return(1);
//		}
//		else
//		{
//				return(0);
//		}			
//}
 
uint8_t clear_alarm(void)
{
		uint8_t i;	

		for(i=0;i<ALARM_TOTAL_WORD;i++)
		{
			g_sys.config.general.alarm_remove_bitmap[i] = 0xffff;
		}

		return 1;		
}

static uint8_t get_alarm_remove_bitmap(uint8_t alarm_id)
{
	uint8_t byte_offset,bit_offset;
	if(alarm_id < ACL_TOTAL_NUM)
	{
		
			byte_offset = alarm_id >> 4;
			bit_offset = alarm_id & 0x0f;
			if((g_sys.config.general.alarm_remove_bitmap[byte_offset] >> bit_offset) & 0x0001 )
			{
					return(1);
			}
			else
			{
					return(0);
			}
	}
	else
	{
			return(0);
	}
	
}

static void clear_alarm_remove_bitmap(uint8_t alarm_id)
{
	uint8_t byte_offset,bit_offset;
	
		if(alarm_id < ACL_TOTAL_NUM)
		{
				byte_offset = alarm_id >> 4;
				bit_offset = alarm_id & 0x0f;
				g_sys.config.general.alarm_remove_bitmap[byte_offset] = g_sys.config.general.alarm_remove_bitmap[byte_offset] & (~(0x0001 << bit_offset));
		}
	
}



uint16_t Alarm_acl_delay(uint8_t ACL_Num)
{
	uint16_t ACL_Delay;
	
	switch (ACL_Num)
	{
			case(ACL_E0):
			case(ACL_E1):
			case(ACL_E2):
			case(ACL_E3):
			case(ACL_E4):
			case(ACL_E5):
			case(ACL_E6):
			case(ACL_E7):
			case(ACL_E8):
			{
					ACL_Delay = 5;
					break;
			}
			case(ACL_FAN01_OD):
			case(ACL_HI_PRESS1):
			case(ACL_HI_PRESS2):
			{
					ACL_Delay = 3;
					break;
			}
			case(ACL_FILTER_OT):
			case(ACL_FILTER_ELEMENT_0_OT):
			case(ACL_FILTER_ELEMENT_1_OT):
			case(ACL_FILTER_ELEMENT_2_OT):
			case(ACL_FILTER_ELEMENT_3_OT):
			case(ACL_FILTER_ELEMENT_4_OT):
			case(ACL_UV1_OT):
			case(ACL_UV2_OT):
			{
					ACL_Delay = 30;
					break;
			}
			default:
			{
					ACL_Delay = 5;
					break;	
			}				
	}
	return ACL_Delay;
}

void alarm_acl_exe(void)
{
		extern	sys_reg_st		g_sys;
		uint16_t acl_trigger_state;	
		uint16_t i;
		uint16_t c_state;
		uint16_t log_id;
		static	uint8_t	u8CNT=0;
		
		u8CNT++;
		if(u8CNT>=0xFF)
		{
			u8CNT=0x00;	
		}
		i=u8CNT%2;
//		//���μ������1S
		if(i!=0)
		{
			return ;
		}
		
//		acl_power_on_timer();
		
    for(i=0;i<ACL_TOTAL_NUM;i++)	
		{
				//if acl disabled, continue loop
				
				g_sys.config.alarm[i].enable_mode=ACL_ENMODE_AUTO_RESET_ALARM;
				if(((g_sys.config.alarm[i].enable_mode & alarm_inst.alarm_sts[i].enable_mask) == ACL_ENMODE_DISABLE)&&(alarm_inst.alarm_sts[i].state == ALARM_FSM_INACTIVE))
				{
						continue;				
				}
			
				acl_trigger_state = acl[i](&alarm_inst.alarm_sts[i]);
				c_state = alarm_inst.alarm_sts[i].state;	
				log_id = alarm_inst.alarm_sts[i].id|(alarm_inst.alarm_sts[i].alarm_level<<8)|alarm_inst.alarm_sts[i].dev_type;
				switch (c_state)
				{
						case(ALARM_FSM_INACTIVE):
						{
							  if(acl_trigger_state == ALARM_ACL_TRIGGERED)
								{
//										alarm_inst.alarm_sts[i].timeout = g_sys.config.alarm[i].delay;
										alarm_inst.alarm_sts[i].timeout = Alarm_acl_delay(i);
										alarm_inst.alarm_sts[i].state = ALARM_FSM_PREACTIVE;
								}
								else if(acl_trigger_state == ALARM_ACL_HOLD)
								{
								 		alarm_inst.alarm_sts[i].state = ALARM_FSM_INACTIVE;
								}
								else
								{
										;
								}
								
								break;
						}
						case(ALARM_FSM_PREACTIVE):
						{
									 //״̬���ص� ALARM_FSM_INACTIVE ״̬
								if((g_sys.config.alarm[i].enable_mode & alarm_inst.alarm_sts[i].enable_mask) == ACL_ENMODE_DISABLE)
								{
										alarm_inst.alarm_sts[i].state = ALARM_FSM_INACTIVE;		
								}
								else if(acl_trigger_state == ALARM_ACL_TRIGGERED)
								{
										
									
									 if(alarm_inst.alarm_sts[i].timeout > 0)
										{
												alarm_inst.alarm_sts[i].timeout --;
												alarm_inst.alarm_sts[i].state = ALARM_FSM_PREACTIVE;
										}
										else
										{
												alarm_inst.alarm_sts[i].state = ALARM_FSM_ACTIVE;
//												rt_kprintf("i=%d,alarm_inst.alarm_sts[i].id=%X,alarm_inst.alarm_sts[i].alram_value=%d\n",i,alarm_inst.alarm_sts[i].id,alarm_inst.alarm_sts[i].alram_value);		
												//yxq
												add_alarmlog_fifo(log_id,ALARM_TRIGER,alarm_inst.alarm_sts[i].alram_value);
												alarm_status_bitmap_op(alarm_inst.alarm_sts[i].id&0x00ff,1);
												node_append(log_id,alarm_inst.alarm_sts[i].alram_value);
											
										}
								}
								else if(acl_trigger_state == ALARM_ACL_HOLD)
								{
								 		;
								}
								else
								{
										alarm_inst.alarm_sts[i].timeout = 0;
										alarm_inst.alarm_sts[i].state = ALARM_FSM_INACTIVE;		
								}							
								break;
						}
						case(ALARM_FSM_ACTIVE):
						{
							 //״̬���ص� ALARM_FSM_INACTIVE ״̬
								if((g_sys.config.alarm[i].enable_mode & alarm_inst.alarm_sts[i].enable_mask) == ACL_ENMODE_DISABLE)
								{
										alarm_inst.alarm_sts[i].timeout = 0;
									  alarm_inst.alarm_sts[i].state = ALARM_FSM_POSTACTIVE;		
								}
								else if(acl_trigger_state == ALARM_ACL_TRIGGERED)
								{					
										alarm_inst.alarm_sts[i].timeout = 0;
										alarm_inst.alarm_sts[i].state = ALARM_FSM_ACTIVE;
								}
								else if(acl_trigger_state == ALARM_ACL_CLEARED)
								{	
									//�Զ��������
									if((g_sys.config.alarm[i].enable_mode & alarm_inst.alarm_sts[i].reset_mask) == ACL_ENMODE_AUTO_RESET_ALARM)
									{
//										alarm_inst.alarm_sts[i].timeout = g_sys.config.alarm[i].delay;
										alarm_inst.alarm_sts[i].timeout = Alarm_acl_delay(i);
										
										alarm_inst.alarm_sts[i].state = ALARM_FSM_POSTACTIVE;	
									}
									else
									{
										alarm_inst.alarm_sts[i].state = ALARM_FSM_ACTIVE;
									}
								}
								else if(acl_trigger_state == ALARM_ACL_HOLD)
								{
								 		;
								}
								else
								{
									;
								}
								break;
						}
						case(ALARM_FSM_POSTACTIVE):
						{
								 //״̬���ص� ALARM_FSM_INACTIVE ״̬
								if((g_sys.config.alarm[i].enable_mode & alarm_inst.alarm_sts[i].enable_mask) == ACL_ENMODE_DISABLE)
								{
											add_alarmlog_fifo(log_id,ALARM_END,alarm_inst.alarm_sts[i].alram_value);												
									     //ɾ��״̬�ڵ�
											node_delete(log_id);
											alarm_status_bitmap_op(alarm_inst.alarm_sts[i].id&0x00ff,0);
												
											alarm_inst.alarm_sts[i].state = ALARM_FSM_INACTIVE;
											alarm_inst.alarm_sts[i].timeout = 0;
								}
								else if(acl_trigger_state == ALARM_ACL_CLEARED)//yxq
								{
										
										if(alarm_inst.alarm_sts[i].timeout > 0)
										{
												alarm_inst.alarm_sts[i].timeout --;
												alarm_inst.alarm_sts[i].state = ALARM_FSM_POSTACTIVE;
										}
										else
										{
												//yxq
												add_alarmlog_fifo(log_id,ALARM_END,alarm_inst.alarm_sts[i].alram_value);
//												
//												//ɾ��״̬�ڵ�
												alarm_status_bitmap_op(i,0);
												node_delete(log_id);
											
												
												alarm_inst.alarm_sts[i].state = ALARM_FSM_INACTIVE;
										}
												
								}
								else if(acl_trigger_state == ALARM_ACL_TRIGGERED)
								{
										alarm_inst.alarm_sts[i].timeout = 0;
										alarm_inst.alarm_sts[i].state = ALARM_FSM_ACTIVE;
								}
								else if(acl_trigger_state == ALARM_ACL_HOLD)
								{
								 		;
								}
								else
								{
									;
								}
								break;
						}
						default://yxq
							{
								alarm_inst.alarm_sts[i].state=ALARM_FSM_INACTIVE;
								
								break;
							}
				}
		}
		g_sys.status.ComSta.u16Alarm_bitmap[0]=g_sys.status.alarm_bitmap[0];
//		rt_kprintf("u16Alarm_bitmap[0] = %x\n",g_sys.status.ComSta.u16Alarm_bitmap[0]);									
		alarm_arbiration();

//		rt_kprintf("alarm_bitmap[0] = %x,[1] = %x,[2] = %x,[3] = %x,[4] = %x,[5] = %x\n",g_sys.status.alarm_bitmap[0],g_sys.status.alarm_bitmap[1],g_sys.status.alarm_bitmap[2],
//		g_sys.status.alarm_bitmap[3],g_sys.status.alarm_bitmap[4],g_sys.status.alarm_bitmap[5]);	

}
//��ȡ����λ
uint8_t get_alarm_bitmap(uint8_t alarm_id)
{
	uint8_t byte_offset,bit_offset;

	{
			byte_offset = alarm_id>>4;
			bit_offset = alarm_id&0x0f;
			
			if((g_sys.status.alarm_bitmap[byte_offset] >>bit_offset) & (0x0001))
			{
				return(1);
			}
			else
			{
				return(0);
			}		
	}	
}


//�澯����
static void alarm_arbiration(void)
{
		uint8_t compress1_alarm=0, compress2_alarm=0,close_dev=0;
		uint8_t index;
	
		if(get_alarm_bitmap(ACL_HI_PRESS1))//��ѹ
		{
			compress1_alarm=1;
		}
		else
		{
			compress1_alarm=0;
		}

		if(get_alarm_bitmap(ACL_HI_PRESS2))//��ѹ
		{
			compress2_alarm=1;
		}
		else
		{
			compress1_alarm=0;
		}
		
		if(get_alarm_bitmap(ACL_FAN01_OD))//���
		{
			close_dev=1;
		}
		else
		{
			close_dev=0;
		}


		//ѹ����1�Ŀ���
		if((close_dev)||(compress1_alarm))
		{
			alarm_bitmap_op(DO_COMP1_BPOS,0);
			alarm_bitmap_mask_op(DO_COMP1_BPOS,1);
		}
		else
		{
			alarm_bitmap_op(DO_COMP1_BPOS,1);
			alarm_bitmap_mask_op(DO_COMP1_BPOS,0);
		}

		//ѹ����2�Ŀ���
		if((close_dev)||(compress2_alarm))
		{
			alarm_bitmap_op(DO_COMP2_BPOS,0);
			alarm_bitmap_mask_op(DO_COMP2_BPOS,1);
		}
		else
		{
			alarm_bitmap_op(DO_COMP2_BPOS,1);
			alarm_bitmap_mask_op(DO_COMP2_BPOS,0);
		}
		//�������
		if((close_dev))
		{
			alarm_bitmap_op(DO_FAN_BPOS,0);
			alarm_bitmap_mask_op(DO_FAN_BPOS,1);
		}
		else
		{
			alarm_bitmap_op(DO_FAN_BPOS,1);
			alarm_bitmap_mask_op(DO_FAN_BPOS,0);
		}	
		
//		if((g_sys.config.ComPara.u16Manual_Mode_En)||(g_sys.config.ComPara.u16Test_Mode_En))//����ģʽ,���Ƹ澯���
//		{
//				return;
//		}
		 //�رչ����澯
		alarm_bitmap_op(DO_ALARM_BPOS,0);	
		alarm_bitmap_mask_op(DO_ALARM_BPOS,1);
		
		if(g_sys.config.general.Alarm_Beep)
		{
				if((get_alarm_bitmap(ACL_HI_PRESS1))||(get_alarm_bitmap(ACL_HI_PRESS1)))
				{
						//������������
						alarm_bitmap_op(DO_ALARM_BPOS,1);
						alarm_bitmap_mask_op(DO_ALARM_BPOS,1);
									
				}
		}
		else
		{
					//���� ������������
				for(index=0;index<ACL_TOTAL_NUM;index++)
				{
					
						if((g_sys.config.alarm[index].enable_mode & alarm_inst.alarm_sts[index].enable_mask) == ACL_ENMODE_ENABLE)
						{
							if(get_alarm_bitmap(index))//��������	
							{
								//������������
								
								alarm_bitmap_op(DO_ALARM_BPOS,1);
								alarm_bitmap_mask_op(DO_ALARM_BPOS,1);
														
								break;
							}
						}
				}
					
		}		
}

////����ʱ����� 
////����ֵʱ�䵥λ����
//static uint16_t dev_runingtime(uint16_t low,uint16_t high)
//{
//		uint16_t runing_day;
//		uint32_t run_time;
//		
//		run_time = high;
//		run_time = (run_time<<16) + low;
//		run_time = run_time >>12;
//		runing_day = run_time/24;
//	
//		return(runing_day);
//}



//
uint8_t get_alarm_bitmap_op(uint8_t component_bpos)
{
		uint8_t byte_offset,bit_offset;
	
		byte_offset =  component_bpos >> 4;
		bit_offset = component_bpos & 0x0f;
		if((l_sys.bitmap[byte_offset][BITMAP_ALARM]>>bit_offset)&0x01)
		{
				return(1);
		}
		else
		{
				return(0);
		}
//		if((l_sys.bitmap[BITMAP_ALARM]>>component_bpos)&0x01)
//		{
//				return(1);
//		}
//		else
//		{
//				return(0);
//		}			
}

uint8_t get_alarm_bitmap_mask(uint8_t component_bpos)
{
		uint8_t byte_offset,bit_offset;
	
		byte_offset =  component_bpos >> 4;
		bit_offset = component_bpos & 0x0f;	
		if((l_sys.bitmap[byte_offset][BITMAP_MASK] >>bit_offset)&0x01)
		{
				return(1);
		}
		else
		{
				return(0);
		}
		
//		if((l_sys.bitmap[BITMAP_MASK] >>component_bpos)&0x01)
//		{
//				return(1);
//		}
//		else
//		{
//				return(0);
//		}
			
}



//DO_FAN_BPOS//offset
//����λ����

static void alarm_bitmap_op(uint8_t component_bpos, uint8_t action)
{
		extern local_reg_st l_sys;
		uint8_t byte_offset,bit_offset;
	
		byte_offset =  component_bpos >> 4;
		bit_offset = component_bpos & 0x0f;	
		if(action == 0)
		{
				l_sys.bitmap[byte_offset][BITMAP_ALARM] &= ~(0x0001<<bit_offset);
		}
		else
		{
				l_sys.bitmap[byte_offset][BITMAP_ALARM] |= (0x0001<<bit_offset);
		}
	
//		if(action == 0)
//		{
//				l_sys.bitmap[BITMAP_ALARM] &= ~(0x0001<<component_bpos);
//		}
//		else
//		{
//				l_sys.bitmap[BITMAP_ALARM] |= (0x0001<<component_bpos);
//		}
}

static void alarm_bitmap_mask_op(uint8_t component_bpos, uint8_t action)
{
		extern local_reg_st l_sys;
		uint8_t byte_offset,bit_offset;
	
		byte_offset =  component_bpos >> 4;
		bit_offset = component_bpos & 0x0f;		
		if(action == 0)
		{
				l_sys.bitmap[byte_offset][BITMAP_MASK] &= ~(0x0001<<bit_offset);
		}
		else
		{
				l_sys.bitmap[byte_offset][BITMAP_MASK] |= (0x0001<<bit_offset);
		}
//		if(action == 0)
//		{
//				l_sys.bitmap[BITMAP_MASK] &= ~(0x0001<<component_bpos);
//		}
//		else
//		{
//				l_sys.bitmap[BITMAP_MASK] |= (0x0001<<component_bpos);
//		}
}

//�澯״̬λ
static void   alarm_status_bitmap_op (uint8_t alarm_id,uint8_t option)
{
		uint8_t byte_offset,bit_offset;
		
		byte_offset = alarm_id >> 4;
		bit_offset = alarm_id &0x0f;
		if(option == 1)
		{
				g_sys.status.alarm_bitmap[byte_offset] |= (0x0001<<bit_offset);
		}
		else
		{
				g_sys.status.alarm_bitmap[byte_offset] &= ~(0x0001<<bit_offset);
		}
		
}

/**
  * @brief  alarm 0 rule check function
	* @param  str_ptr: alarm_acl_st* structure pointer 
  * @retval none
  */



static uint16_t io_calc(uint8_t data,uint8_t refer)
{
	
	if(data==refer)
	{
			
		return 1;
	}
	else
	{
		
		return 0;
	}
}

/**
  * @brief  alarm 1 rule check function
	* @param  str_ptr: alarm_acl_st* structure pointer 
  * @retval none
  */


// ģ����������⺯��
static uint16_t compare_calc(int16_t meter,int16_t min,int16_t max,Compare_type_st type)
{
	
		if(type==THR_MAX_TYPE)//����������޴���
		{
			if(meter > max)
			{
				return(1);
			}
			else
			{
				return(0);
			}
		}
		else if(type==SML_MIN_TYPE)//С����С���޴���
		{
			if(meter<min)
			{
				return(1);
			}
			else
			{
				return(0);
			}
		}
		else if(type==IN_MIN_MAX_TYPE)//�������ڱ���
		{
			if((meter > min)&&(meter < max))
			{
				return(1);
			}
			else
			{
				return(0);
			}
			
		}
		else//���������ⱨ��
		{
			if((meter<min)||(meter>max))
			{
				return(1);
			}
			else
			{
				
				return(0);
			}
			
		}
	
}

static  uint8_t acl_clear(alarm_acl_status_st* acl_ptr)
{
		if((get_alarm_remove_bitmap(acl_ptr->id) == 1)&&(acl_ptr->state > ALARM_FSM_PREACTIVE))
		{
				acl_ptr->state = ALARM_FSM_POSTACTIVE;
				acl_ptr->timeout = 0;
				clear_alarm_remove_bitmap(acl_ptr->id);
				return(1);
		}
		clear_alarm_remove_bitmap(acl_ptr->id);
		return(0);
}


//ACL_E0 �޳�ˮ�澯
static  uint16_t acl00(alarm_acl_status_st* acl_ptr)
{
		uint8_t data;	
		static uint8_t u8Delay;	

		// ��� ����
		if(acl_clear(acl_ptr))
		{
				return(ALARM_ACL_CLEARED);
		}
		
		if((g_sys.config.ComPara.u16Water_Mode)&&(g_sys.config.ComPara.u16Water_Flow))
		{
				if(g_sys.status.ComSta.u16Cur_Water<1)//��ˮ����
				{
						u8Delay++;
				}
				else
				{
						u8Delay=0;					
				}
		}
		else
		{
				data=0;		
				u8Delay=0;				
		}
		if(u8Delay>30)
		{
				data=1;
		}
		else
		{
				data=0;		
				u8Delay=0;					
		}
		return data;	
}


//ACL_E1 ��ˮ��ˮλ>�ϸ���
static  uint16_t acl01(alarm_acl_status_st* acl_ptr)
{
		uint8_t data;	
		uint16_t u16WL;

			// ��� ����
		if(acl_clear(acl_ptr))
		{
				return(ALARM_ACL_CLEARED);
		}
		//ˮλ	
		u16WL=Get_Water_level();
		if((u16WL&D_U))
		{
			data=1;
		}
		else
		{
				data=0;			
		}
		return data;			
	
}

//ACL_E2 Դˮ��ˮλ>�ϸ���
static  uint16_t acl02(alarm_acl_status_st*acl_ptr)
{
		uint8_t data;	
		uint16_t u16WL;

			// ��� ����
		if(acl_clear(acl_ptr))
		{
				return(ALARM_ACL_CLEARED);
		}
		//ˮλ	
		u16WL=Get_Water_level();
		if((u16WL&S_U))
		{
			data=1;
		}
		else
		{
				data=0;			
		}
		return data;		
	
}

//ACL_E3
static  uint16_t acl03(alarm_acl_status_st*acl_ptr)
{
		int16_t min;
		int16_t max;
		uint8_t req;
		int16_t meter;
		
		// ��� ����
		if(acl_clear(acl_ptr))
		{
				return(ALARM_ACL_CLEARED);
		}
		
		max = ACL_TEM_MAX;
		min = ACL_TEM_MIN;
		
		req =0;
		
		 //NTC
		 if((g_sys.config.dev_mask.ain) & (0x01<<AI_NTC1))
		 {
		 	  
				meter =   g_sys.status.ain[AI_NTC1];
				if(compare_calc( meter,min,max,OUT_MIN_MAX_TYPE))
				{
						sys_set_remap_status(SENSOR_STS_REG_NO,AI_NTC1,1);
				    req =1;
				}
				else
				{
				    sys_set_remap_status(SENSOR_STS_REG_NO,AI_NTC1,0);
				}   		 	 
		 }
		 if((g_sys.config.dev_mask.ain) & (0x01<<AI_NTC2))
		 {
				meter =   g_sys.status.ain[AI_NTC2];
				if(compare_calc( meter,min,max,OUT_MIN_MAX_TYPE))
				{
						sys_set_remap_status(SENSOR_STS_REG_NO,AI_NTC1,2);	
						req =1;
				}
				else
				{
						sys_set_remap_status(SENSOR_STS_REG_NO,AI_NTC1,0);
				}   					
		 }
		 if((g_sys.config.dev_mask.ain) & (0x01<<AI_NTC3))
		 {
				meter =   g_sys.status.ain[AI_NTC3];
				if(compare_calc( meter,min,max,OUT_MIN_MAX_TYPE))
				{
						sys_set_remap_status(SENSOR_STS_REG_NO,AI_NTC3,1);	
						req =1;
				}
				else
				{
						sys_set_remap_status(SENSOR_STS_REG_NO,AI_NTC3,0);
				}   					
		 }
		 
		 acl_ptr->alram_value = g_sys.status.status_remap[SENSOR_STS_REG_NO]; 
			return(req);		 
}

// ACL_E4
static  uint16_t acl04(alarm_acl_status_st*acl_ptr)
{
//		uint8_t data;	
//		uint16_t u16WL;

//			// ��� ����
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(sys_get_pwr_sts() == 0)
//		{
//				return(ALARM_ACL_CLEARED);
//		}		
//		//ˮλ	
//		u16WL=Get_Water_level();
//		if(!(u16WL&D_M))
//		{
//			if((sys_get_do_sts(DO_FAN_BPOS)==0)&&((sys_get_do_sts(DO_COMP1_BPOS)==0)||(sys_get_do_sts(DO_COMP1_BPOS)==0)))
//			{
//					data=1;
//			}
//			else
//			{
//					data=0;				
//			}
//		}
//		return data;	
				return(ALARM_ACL_CLEARED);
}
//ACL_E5 ��ʪ�ȴ���������
static	uint16_t acl05(alarm_acl_status_st* acl_ptr)
{
		int16_t min;
		int16_t max;
		uint8_t req;
		int16_t meter;
		
		// ��� ����
		if(acl_clear(acl_ptr))
		{
				return(ALARM_ACL_CLEARED);
		}
		
		max = ACL_TEM_MAX;
		min = ACL_TEM_MIN;
		
		req =0;
		
		 //TH
		 if((g_sys.config.dev_mask.mb_comp) & (0x01<<0x00))
		 {
				meter =   g_sys.status.ComSta.u16TH[0].Temp;
				if(compare_calc( meter,min,max,OUT_MIN_MAX_TYPE))
				{
						sys_set_remap_status(SENSOR_STS_REG_NO,AI_NTC3+1,1);
				    req =1;
				}
				else
				{
						sys_set_remap_status(SENSOR_STS_REG_NO,AI_NTC3+1,0);
				} 							
		 }
		 acl_ptr->alram_value = g_sys.status.status_remap[SENSOR_STS_REG_NO]; 
		 return(req);			 
}

//ACL_E6 
static  uint16_t acl06(alarm_acl_status_st*acl_ptr)
{
		int16_t min;
		int16_t max;
		uint8_t req;
		int16_t meter;
		
		// ��� ����
		if(acl_clear(acl_ptr))
		{
				return(ALARM_ACL_CLEARED);
		}
		
		max = ACL_HUM_MAX;
		min = ACL_HUM_MIN;		
		req =0;
		
		 //TH
		 if((g_sys.config.dev_mask.mb_comp) & (0x01<<0x00))
		 {
				meter =   g_sys.status.ComSta.u16TH[0].Hum;
				if(compare_calc( meter,min,max,OUT_MIN_MAX_TYPE))
				{
						sys_set_remap_status(SENSOR_STS_REG_NO,AI_NTC3+1,1);
				    req =1;
				}
				else
				{

						sys_set_remap_status(SENSOR_STS_REG_NO,AI_NTC3+1,0);
				} 							
		 }
		 acl_ptr->alram_value = g_sys.status.status_remap[SENSOR_STS_REG_NO]; 
		 return(req);			 
}
//ACL_E7 ��˪ģʽ�����δ��
static  uint16_t acl07(alarm_acl_status_st*acl_ptr)
{
		uint8_t data;	

			// ��� ����
		if(acl_clear(acl_ptr))
		{
				return(ALARM_ACL_CLEARED);
		}
		if((sys_get_remap_status(WORK_MODE_STS_REG_NO, DEFROST1_STS_BPOS) != 0)||(sys_get_remap_status(WORK_MODE_STS_REG_NO, DEFROST2_STS_BPOS) != 0))				//alarm alternation
		{
			if(sys_get_do_sts(DO_FAN_BPOS)==0)
			{
					data=1;
			}
			else
			{
					data=0;				
			}
		}
		return data;	
}
//ACL_E8 ����ɱ����δ��
static  uint16_t acl08(alarm_acl_status_st*acl_ptr)
{
//		uint8_t data=0;	

//			// ��� ����
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if((sys_get_remap_status(WORK_MODE_STS_REG_NO, OUTWATER_STS_BPOS) != 0))				//alarm alternation
//		{
//				//����ʽ��ˮ��
//				if((g_sys.config.ComPara.u16Water_Mode==WATER_HEAT)||(l_sys.OutWater_Key&WATER_HEAT))//��ˮ
//				{
//						if(!(g_sys.config.ComPara.u16Water_Ctrl&HEART_POT))//����ʽ��ˮ
//						{
//								return(ALARM_ACL_CLEARED);
//						}
//				}
//				//220V�����
//				if((g_sys.config.ComPara.u16Sterilize_Mode&STERILIZE_BIT0)&&(sys_get_do_sts(DO_UV1_BPOS)==0))
//				{				
//					data=1;				
//				}
//				//24V�����
//				if((g_sys.config.ComPara.u16Sterilize_Mode&STERILIZE_BIT1)&&(sys_get_do_sts(DO_UV2_BPOS)==0))
//				{				
//					data=1;				
//				}
//		}
//		return data;	
			return(ALARM_ACL_CLEARED);
}

//ACL_RESERVE_01
static  uint16_t acl09(alarm_acl_status_st* acl_ptr)
{
			return 0;
}

// ACL_FAN01_OD
static  uint16_t acl10(alarm_acl_status_st* acl_ptr)
{
		uint8_t data;

		// ��� ����
		if(acl_clear(acl_ptr))
		{
				return(ALARM_ACL_CLEARED);
		}

		if(sys_get_do_sts(DO_FAN_BPOS)==0)
		{
				return(ALARM_ACL_HOLD);
		}			

		data = sys_get_di_sts(DI_FAN01_OD_BPOS);	
		data = io_calc( data,IO_CLOSE);			

		return(data);
}
//ACL_HI_PRESS1
static	uint16_t acl11(alarm_acl_status_st* acl_ptr)
{
		uint8_t data;

		// ��� ����
		if(acl_clear(acl_ptr))
		{
				return(ALARM_ACL_CLEARED);
		}

		if(devinfo_get_compressor_cnt() == 0)
		{
				return(ALARM_ACL_CLEARED);
		}	

		data = sys_get_di_sts(DI_HI_PRESS1_BPOS);	
		data = io_calc( data,IO_CLOSE);			

		return(data);
}
//ACL_HI_PRESS2
static	uint16_t acl12(alarm_acl_status_st* acl_ptr)
{
		uint8_t data;

		// ��� ����
		if(acl_clear(acl_ptr))
		{
				return(ALARM_ACL_CLEARED);
		}

		if(devinfo_get_compressor_cnt() == 0)
		{
				return(ALARM_ACL_CLEARED);
		}	

		data = sys_get_di_sts(DI_HI_PRESS2_BPOS);	
		data = io_calc( data,IO_CLOSE);			

		return(data);
}
//ACL_FILTER_OT
static	uint16_t acl13(alarm_acl_status_st* acl_ptr)
{
		uint32_t run_time=0;
		int16_t max;
		//����ȷ��
		if(acl_clear(acl_ptr))
		{
				return(ALARM_ACL_CLEARED);
		}
		if(sys_get_pwr_sts() == 0)
		{
				return(ALARM_ACL_CLEARED);
		}
//		run_time = dev_runingtime(g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].low,g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].high);
		run_time = g_sys.status.ComSta.u16Runtime[1][DO_FILLTER_DUMMY_BPOS];
		max = g_sys.config.alarm[ACL_FILTER_OT].alarm_param;
		
		alarm_inst.alarm_sts[acl_ptr->id].alram_value = run_time;
		return(compare_calc( run_time,0,max,THR_MAX_TYPE));	
}
//ACL_FILTER_ELEMENT_0_OT
static	uint16_t acl14(alarm_acl_status_st* acl_ptr)
{
		uint32_t run_time=0;
		int16_t max;
		//����ȷ��
		if(acl_clear(acl_ptr))
		{
				return(ALARM_ACL_CLEARED);
		}
		if(sys_get_pwr_sts() == 0)
		{
				return(ALARM_ACL_CLEARED);
		}
//		run_time = dev_runingtime(g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].low,g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].high);
		run_time = g_sys.status.ComSta.u16Runtime[1][DO_FILLTER_ELEMENT_DUMMY_BPOS_0];
		max = g_sys.config.alarm[ACL_FILTER_ELEMENT_0_OT].alarm_param;
		
		alarm_inst.alarm_sts[acl_ptr->id].alram_value = run_time;
		return(compare_calc( run_time,0,max,THR_MAX_TYPE));	
}

//ACL_FILTER_ELEMENT_1_OT
static	uint16_t acl15(alarm_acl_status_st* acl_ptr)
{
		uint32_t run_time=0;
		int16_t max;
		//����ȷ��
		if(acl_clear(acl_ptr))
		{
				return(ALARM_ACL_CLEARED);
		}
		if(sys_get_pwr_sts() == 0)
		{
				return(ALARM_ACL_CLEARED);
		}
//		run_time = dev_runingtime(g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].low,g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].high);
		run_time = g_sys.status.ComSta.u16Runtime[1][DO_FILLTER_ELEMENT_DUMMY_BPOS_1];
		max = g_sys.config.alarm[ACL_FILTER_ELEMENT_1_OT].alarm_param;
		
		alarm_inst.alarm_sts[acl_ptr->id].alram_value = run_time;
		return(compare_calc( run_time,0,max,THR_MAX_TYPE));	
}

//ACL_FILTER_ELEMENT_2_OT
static	uint16_t acl16(alarm_acl_status_st* acl_ptr)
{
		uint32_t run_time=0;
		int16_t max;
		//����ȷ��
		if(acl_clear(acl_ptr))
		{
				return(ALARM_ACL_CLEARED);
		}
		if(sys_get_pwr_sts() == 0)
		{
				return(ALARM_ACL_CLEARED);
		}
//		run_time = dev_runingtime(g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].low,g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].high);
		run_time = g_sys.status.ComSta.u16Runtime[1][DO_FILLTER_ELEMENT_DUMMY_BPOS_2];
		max = g_sys.config.alarm[ACL_FILTER_ELEMENT_2_OT].alarm_param;
		
		alarm_inst.alarm_sts[acl_ptr->id].alram_value = run_time;
		return(compare_calc( run_time,0,max,THR_MAX_TYPE));	
}

//ACL_FILTER_ELEMENT_3_OT
static	uint16_t acl17(alarm_acl_status_st* acl_ptr)
{
		uint32_t run_time=0;
		int16_t max;
		//����ȷ��
		if(acl_clear(acl_ptr))
		{
				return(ALARM_ACL_CLEARED);
		}
		if(sys_get_pwr_sts() == 0)
		{
				return(ALARM_ACL_CLEARED);
		}
//		run_time = dev_runingtime(g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].low,g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].high);
		run_time = g_sys.status.ComSta.u16Runtime[1][DO_FILLTER_ELEMENT_DUMMY_BPOS_3];
		max = g_sys.config.alarm[ACL_FILTER_ELEMENT_3_OT].alarm_param;
		
		alarm_inst.alarm_sts[acl_ptr->id].alram_value = run_time;
		return(compare_calc( run_time,0,max,THR_MAX_TYPE));	
}

//ACL_FILTER_ELEMENT_4_OT
static	uint16_t acl18(alarm_acl_status_st* acl_ptr)
{
		uint32_t run_time=0;
		int16_t max;
		//����ȷ��
		if(acl_clear(acl_ptr))
		{
				return(ALARM_ACL_CLEARED);
		}
		if(sys_get_pwr_sts() == 0)
		{
				return(ALARM_ACL_CLEARED);
		}
//		run_time = dev_runingtime(g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].low,g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].high);
		run_time = g_sys.status.ComSta.u16Runtime[1][DO_FILLTER_ELEMENT_DUMMY_BPOS_4];
		max = g_sys.config.alarm[ACL_FILTER_ELEMENT_4_OT].alarm_param;
		
		alarm_inst.alarm_sts[acl_ptr->id].alram_value = run_time;
		return(compare_calc( run_time,0,max,THR_MAX_TYPE));	
}

//ACL_UV1_OT
static	uint16_t acl19(alarm_acl_status_st* acl_ptr)
{
//					return(ALARM_ACL_CLEARED);
		uint32_t run_time=0;
		int16_t max;
		//����ȷ��
		if(acl_clear(acl_ptr))
		{
				return(ALARM_ACL_CLEARED);
		}
		if(sys_get_pwr_sts() == 0)
		{
				return(ALARM_ACL_CLEARED);
		}
//		run_time = dev_runingtime(g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].low,g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].high);
		run_time = g_sys.status.ComSta.u16Runtime[1][DO_UV1_BPOS];
		max = g_sys.config.alarm[ACL_UV1_OT].alarm_param;
		
		alarm_inst.alarm_sts[acl_ptr->id].alram_value = run_time;
		return(compare_calc( run_time,0,max,THR_MAX_TYPE));	
}
//ACL_UV2_OT
static	uint16_t acl20(alarm_acl_status_st* acl_ptr)
{
//				return(ALARM_ACL_CLEARED);
		uint32_t run_time=0;
		int16_t max;
		//����ȷ��
		if(acl_clear(acl_ptr))
		{
				return(ALARM_ACL_CLEARED);
		}
		if(sys_get_pwr_sts() == 0)
		{
				return(ALARM_ACL_CLEARED);
		}
		run_time = g_sys.status.ComSta.u16Runtime[1][DO_UV2_BPOS];
		max = g_sys.config.alarm[ACL_UV2_OT].alarm_param;
		
		alarm_inst.alarm_sts[acl_ptr->id].alram_value = run_time;
		return(compare_calc( run_time,0,max,THR_MAX_TYPE));	
}






