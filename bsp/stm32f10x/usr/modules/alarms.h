#ifndef __ALAMRS_H__
#define __ALAMRS_H__

#include "sys_conf.h"


#define ACL_INACTIVE 		0
#define ACL_PREACTIVE 	1
#define ACL_ACTIVE 			2
#define ACL_POSTACTIVE	3

#define ACL_ENABLE			0
#define ACL_SUPPRESS		1
#define ACL_DISABLE			2

//alarm acl def
enum
{
//����	
			ACL_E0 = 0		,//�޳�ˮ�澯
			ACL_E1			  ,
			ACL_E2		    ,
			ACL_E3			  ,
			ACL_E4		 		,//
      ACL_E5        ,//
			ACL_E6				,
			ACL_E7				,
			ACL_E8			  ,//�����ɱ��δ��
			ACL_RESERVE_01	,
	
			ACL_FAN01_OD		,
			ACL_HI_PRESS1		,//��ѹ1
			ACL_HI_PRESS2		,//��ѹ2
			ACL_FILTER_OT		,//����
			ACL_FILTER_ELEMENT_0_OT	,//��о1
			ACL_FILTER_ELEMENT_1_OT	,//��о2
			ACL_FILTER_ELEMENT_2_OT	,//��о3
			ACL_FILTER_ELEMENT_3_OT	,//��о4
			ACL_FILTER_ELEMENT_4_OT	,//��о5
			ACL_UV1_OT	,//�����1
			
			ACL_UV2_OT	,//�����2��ʱ
//�쳣
			ACL_TOTAL_NUM				,

};

//Alair,20161227
enum
{
	DEV_RETURN_SENSOR1_FAULT_BPOS=0,
	DEV_RETURN_SENSOR2_FAULT_BPOS,
	DEV_RETURN_SENSOR3_FAULT_BPOS,
	DEV_RETURN_SENSOR4_FAULT_BPOS,
	DEV_SUPPLY_SENSOR1_FAULT_BPOS,
	DEV_SUPPLY_SENSOR2_FAULT_BPOS,
	//
	DEV_TEM_HUM_RESERVE1_FAULT_BPOS,
	DEV_TEM_HUM_RESERVE2_FAULT_BPOS,
	//
	DEV_NTC7_BPOS,
	DEV_NTC8_BPOS,
	DEV_CIOL_NTC1_FAULT_BPOS,
	DEV_CIOL_NTC2_FAULT_BPOS,
	DEV_RETUREN_NTC1_FAULT_BPOS,
	DEV_RETUREN_NTC2_FAULT_BPOS,
	DEV_SUPPLY_NTC1_FAULT_BPOS,
	DEV_SUPPLY_NTC2_FAULT_BPOS,
};


void alarm_acl_init(void);
void alarm_acl_exe(void);

void In_alarm_stats(void);

uint8_t get_alarm_bitmap(uint8_t alarm_id);

uint8_t clear_alarm(void);
uint8_t get_alarm_bitmap_mask(uint8_t component_bpos);
uint8_t get_alarm_bitmap_op(uint8_t component_bpos);

#endif //__ALAMRS_H__
