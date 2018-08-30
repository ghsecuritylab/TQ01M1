#ifndef __SYS_CONF
#define	__SYS_CONF

#include "sys_def.h"
#include "alarms.h"
#include "string.h"
//#include "user_mb_app.h"

#define ENVIRONMENTAL_MONITOR  1//�������

//IPM����
#define IPM_DDS3366D   1
//#define IPM_YD2010     1

#define TRUE   1
#define FALSE  0

//�������ӳ��
enum
{
		DO_COMP1_BPOS=0,		//ѹ��1����
    DO_COMP2_BPOS,			//ѹ��2	
    DO_RH1_BPOS,				//�����1
		DO_FAN_BPOS,				//�ڷ��
		DO_UV1_BPOS,				//�����1,��ʱɱ��
		DO_WV_BPOS,					//��ú��ˮ��(ѹ������)
		DO_CV_BPOS,					//��ú�Ʊ�ˮ��
		DO_RSV1_BPOS,				//����
		DO_WP_BPOS,				  //��ˮ��
		DO_HWP_BPOS,				//��ˮ��ˮ��
		DO_PWP_BPOS,				//������
		DO_DWP_BPOS,				//ɱ����
		DO_DV_BPOS,					//��ˮ��
		DO_FV_BPOS,					//��ӽ�ˮ��
		DO_RSV2_BPOS,				//����
		DO_EL1_BPOS,			  //������1
	
		DO_EL2_BPOS,			  //������2
		DO_RSV3_BPOS,			  //Ԥ��
		DO_LED_LOCK_BPOS,		//ͯ��LED DC5V
		DO_PWR_CTRL_BPOS,		//12V��Դ����,�͵�ƽ��Ч
		DO_RSV_BPOS_0,			//Ԥ��	
		DO_RSV_BPOS_1,			//Ԥ��			
		DO_RSV_BPOS_2,			//Ԥ��	
		DO_RSV_BPOS_3,			//Ԥ��	
		
		DO_FILLTER_DUMMY_BPOS,//����
		DO_FILLTER_ELEMENT_DUMMY_BPOS_0,//��о  0
		DO_FILLTER_ELEMENT_DUMMY_BPOS_1,//��о  1
		DO_FILLTER_ELEMENT_DUMMY_BPOS_2,//��о  2
		DO_FILLTER_ELEMENT_DUMMY_BPOS_3,//��о  3
		DO_FILLTER_ELEMENT_DUMMY_BPOS_4,//��о  4
		DO_FILLTER_ELEMENT_DUMMY_BPOS_5,//��о  5
		DO_MAX_CNT,
};
#define   DO_FAN_LOW_BPOS 	DO_FAN_BPOS	//����͵�
#define   DO_UV2_BPOS   		DO_RSV1_BPOS		//�����2
#define   DO_ALARM_BPOS   	DO_RSV2_BPOS		//�����澯

//application delay
#define		MODBUS_MASTER_THREAD_DELAY  500
#define		MODBUS_MASTER_THREAD_DELAY_01  800
#define		MODBUS_SLAVE_THREAD_DELAY   2000
//#define		TCOM_THREAD_DELAY	          1150
//#define		TEAM_THREAD_DELAY           1200
#define		MBM_FSM_THREAD_DELAY				1150	
#define		MBM_FSM_THREAD_DELAY_01				1250	
#define		DI_THREAD_DELAY							1300
#define		DAQ_THREAD_DELAY						1350	
#define		CORE_THREAD_DELAY						3000
#define		SURV_THREAD_DELAY						1400
#define		CPAD_THREAD_DELAY					  1600
#define		BKG_THREAD_DELAY            2200
#define		TESTCASE_THREAD_DELAY		    1650


///////////////////////////////////////////////////////////////
//AI configuration 
///////////////////////////////////////////////////////////////
enum
{
		AI_SENSOR1 = 0,
		AI_NTC1,
		AI_NTC2,
		AI_NTC3,
		AI_NTC4,
		AI_MAX_CNT
};

#define AI_SENSOR_NUM   	1		//����������
#define AI_NTC_NUM   			4		//NTC����



enum
{
		AO_EC_FAN = 0,//EC���
		AO_PREV_1,
		AO_WATER_VALVE,
    AO_EC_COMPRESSOR,//��Ƶѹ��
    AO_PREV_2,
		AO_MAX_CNT,	
		AO_INV_FAN, //������� ��Ƶ
};

#define AO_REAL_CNT    1
#define ABNORMAL_VALUE    0x7FFF//�쳣ֵ
#define OVER_VALUE    1000//��ʪ�ȳ���ֵ

//����ģʽ
enum
{
		TEST_UNABLE = 0,//�˳�����ģʽ
		TEST_PRPDUCE_WATER=0x01,//��ˮ
		TEST_PURIFICATION=0x02, //����
    TEST_NORMAL_WATER=0x03,   //������ˮ
    TEST_HEAT_WATER=0x04,    //����ˮ
    TEST_PRPDUCE_COLDWATER=0x05,  //�Ʊ�ˮ
    TEST_ALL_OUT=0x5A,      //ȫ��
};

//�ֶ�����ģʽ
enum
{
		MANUAL_TEST_UNABLE = 0,//�˳�����ģʽ
		MANUAL_MODE_ENABLE=0x01,//�ֶ�ģʽ
		TEST_MODE_ENABLE=0x02, //����ģʽ
};

///////////////////////////////////////////////////////////////
//system configuration 
///////////////////////////////////////////////////////////////

typedef struct 
{
	  uint8_t      Err_Master0[10];//�쳣����
	  uint8_t      Err_Master1[2];//�쳣����	
	  uint8_t      Err_Master2[4];//�쳣����
	  uint16_t      Err_Master_Cnt[3];//ͨ���쳣
	  uint8_t      Err_Enable;//ͨ�Ÿ澯ʹ��
}mbm_Error_St;

typedef struct 
{
		uint16_t 	id;
		uint16_t*	reg_ptr;
		int16_t	min;
		uint16_t	max;
		uint16_t	dft;	
		uint8_t		permission;
		uint8_t		rw;
		uint8_t (*chk_ptr)(uint16_t pram);
}conf_reg_map_st;

typedef struct 
{
		uint16_t 	id;
		uint16_t*	reg_ptr;
		uint16_t	dft;
		//uint8_t		rw;
}sts_reg_map_st;

//system component mask, if set 1 measn exist, otherwise absent
typedef struct 
{
		uint16_t 	power_switch;
		uint16_t	set_voltage;
		uint16_t	set_current;
		//uint8_t		rw;
}pwr_dev_set_st;

typedef struct 
{
		uint16_t 	ain;
		uint16_t	din[2];
		uint16_t  din_pusl;
		uint16_t 	aout;
		uint16_t	dout[2];
		uint16_t	mb_comp;
		uint16_t  mb_discrete_mask;
		uint16_t  return_temp_mask;
		uint16_t  supply_temp_mask;
		uint16_t  cab_temp_mask;
		uint16_t  din_bitmap_polarity[2];
//    uint16_t  pwm_out;
//		pwr_dev_set_st mbm_pwr_cfg[2];
		
}dev_mask_st;


#define ALARM_TOTAL_WORD 2
typedef struct
{
		uint16_t temp;
		uint16_t hum;
}temp_sensor_cali_st;
typedef struct
{	
	uint16_t 		power_mode;					//power-off or power-on
	uint16_t 		standalone_timer;		//automatic, manual 	
	uint16_t 		cool_type;					//cooling type
	uint16_t 		cpad_baudrate;			//control pad communication baudrate
	uint16_t 		surv_baudrate;			//surveillance communication baudrate
	uint16_t 		surv_addr;					//surveillance communication address
	uint16_t 		diagnose_mode_en;		//diagnose mode enalbe 
	uint16_t 		alarm_bypass_en;		//diagnose mode enalbe 
	uint16_t 		testing_mode_en;		//test mode enalbe 
	uint16_t    power_mode_mb_en;   // modbuss power mode control enable
	uint16_t    cancel_alarm_mb_en; // cancel all alarm enable
	uint16_t    alarm_remove_bitmap[ALARM_TOTAL_WORD]; //reless alarm
	uint16_t    ntc_cali[AI_NTC_NUM];            // NTC cali
	uint16_t    ai_cali[AI_SENSOR_NUM];             //ai_ cali
	uint16_t    LED_Num;                   //LED����
	uint16_t    Alarm_Beep;                //���ظ澯��������
	temp_sensor_cali_st temp_sensor_cali[TEMP_HUM_SENSOR_NUM];     
}conf_general_st;


//status_set
/*
@permission_level: control pad accesssible user permission level
	0:	lowest						
	1:	above lowest
	2:	below highest
	3:	highest
@running_mode: control pad accesssible user permission level
	0:	standalone_power-off						
	1:	standalone_on
	2:	team_poweroff
	3:	team_power_on

@running_mode: control pad accesssible user permission level
	bit0:	fatal error						
	bit1:	internal modbus bus communication error
	bit2:	survallance modbus bus communication error
	bit3:	can bus communication error
*/
enum
{
		SYS_ERR_INIT=0,
		SYS_ERR_TEAM,
		SYS_ERR_MBM,
		SYS_ERR_MBS,
		SYS_ERR_CAN,
		SYS_ERR_CPAD,
};

enum
{
		WORK_MODE_STS_REG_NO= 0,//���鹤��״̬
		GEN_STS_REG_NO ,
		MBM_COM_STS_REG_NO,
		SENSOR_STS_REG_NO
};

enum
{
		ALARM_COM = 0,
		ALARM_NTC,
};

enum
{		
		PWR_STS_BPOS = 0,//����
		COOLING_STS_BPOS,//��ˮ
		OUTWATER_STS_BPOS,//��ˮ
		STERILIZE_STS_BPOS,//ɱ��
		DEFROST1_STS_BPOS,//��˪1
		DEFROST2_STS_BPOS,//��˪2
		FAN_STS_BPOS,
		HEATING_STS_BPOS,
		ALARM_STUSE_BPOS =14,
		ALARM_BEEP_BPOS =15,
};

typedef struct
{
	uint16_t 		permission_level;		//user authentication level
	uint16_t 		running_mode;				//automatic, manual or testing
	uint16_t 		sys_error_bitmap;		//system error status 
	uint16_t 		Alarm_AC_CNT[3];		//AC�澯����

}status_general_st;

enum
{
		P_ALOGORITHM=0,
		PID_ALOGORITHM,
		FUZZY_ALOGORITHM
};

enum
{
		HUM_RELATIVE=0,
		HUM_ABSOLUTE		
};
enum
{
		WATER_LEVEL_NO=0,//ˮ��
		WATER_LEVEL_LOW,//��ˮλ
		WATER_LEVEL_REPLENISHMENT,//��ˮˮλ	
		WATER_LEVEL_HIGH,	//��ˮλ	
		WATER_LEVEL_OVERFLOW,	//��ˮ
		WATER_LEVEL_OTHER,	//�쳣
};
// meter tem_hum
typedef struct
{
	uint16_t supply_air_temp;
	uint16_t return_air_temp;
	uint16_t remote_air_temp;
	uint16_t supply_air_max_temp;
	uint16_t return_air_max_temp;
	uint16_t remote_air_max_temp;
	uint16_t supply_air_hum;
	uint16_t return_air_hum;
	uint16_t remote_air_hum;
	uint16_t supply_air_min_temp;
}sys_tem_hum_st;
//algorithm
typedef struct
{
	uint16_t temp_calc_mode;
	uint16_t temp_ctrl_mode;
	uint16_t hum_ctrl_mode;
	uint16_t ctrl_target_mode;
	uint16_t supply_air_temp;
	uint16_t return_air_temp;
	uint16_t remote_air_temp;
	uint16_t supply_air_hum;
	uint16_t return_air_hum;
	uint16_t remote_air_hum;
	uint16_t temp_precision;
	uint16_t hum_precision;
	uint16_t temp_deadband;
	uint16_t hum_deadband;
	uint16_t sample_interval;
	uint16_t temp_integ;
	uint16_t temp_diff;
	uint16_t pid_action_max;
	uint16_t temp_req_out_max;
}algorithm_st;

//compressor
typedef struct
{
	uint16_t startup_delay;
	uint16_t stop_delay;
	uint16_t min_runtime;
	uint16_t min_stoptime;
	uint16_t startup_lowpress_shield;
	uint16_t alter_mode;
	uint16_t alter_time;
	uint16_t start_interval;
	uint16_t ev_ahead_start_time;
	uint16_t ev_ahead_shut_time;
	uint16_t speed_upper_lim;	
	uint16_t speed_lower_lim;		
	uint16_t ec_comp_start_req;	
  uint16_t startup_freq;	
  uint16_t high_press_threshold;
  uint16_t high_press_hyst;
  uint16_t ret_oil_period;
  uint16_t ret_oil_freq;
  uint16_t low_freq_switch_period;
  uint16_t low_freq_threshold;
  uint16_t step;
  uint16_t step_period;
}compressor_st;


//fan
typedef struct
{
	uint16_t type;
	uint16_t mode;
	uint16_t num;
  uint16_t adjust_step;
	uint16_t startup_delay;
	uint16_t cold_start_delay;
	uint16_t stop_delay;
	uint16_t set_speed;
	uint16_t min_speed;
	uint16_t max_speed;
	uint16_t dehum_ratio;
	uint16_t hum_min_speed;
	uint16_t set_flow_diff;
	uint16_t flow_diff_deadzone;
	uint16_t flow_diff_step;
	uint16_t flow_diff_delay;
	uint16_t target_suc_temp;
	uint16_t suc_temp_deadzone;
	uint16_t suc_temp_step;
	uint16_t suc_temp_delay;
	uint16_t noload_down;//�޸��ط������ʹ��
	uint16_t target_temp;
	uint16_t temp_dead_band;
	uint16_t temp_precision;
	uint16_t temp_add_fan_en;
	uint16_t tem_add_fan_delay;
	uint16_t fan_k;
	uint16_t CFM_Enable;//������ʾʹ��
	uint16_t CFM_Para_A;//��������A
	uint16_t CFM_Para_B;
	uint16_t CFM_Para_C;
}fan_st;

//team set
typedef struct
{
	uint16_t		team_en;					//team enable
	uint16_t		mode;							//team mode 0,1,2,3
	uint16_t		addr;							//team id
	uint16_t 		baudrate;					//team communication baudrate
	uint16_t		total_num;				//units number in the team
	uint16_t		backup_num;				//backup units
	uint16_t		rotate_period;		//upper byte:0:no rotate;1:daily;2:weekly;lower byte:week day(0:sunday,1:monday...)
	uint16_t		rotate_time;			//upper byte:hour;lower byte:minite;
	uint16_t		rotate_num;
	uint16_t		rotate_manual;	
	uint16_t		cascade_enable;
	uint16_t		team_fan_mode;
	uint16_t    fault_power_en;
}team_st;


//analog_in
typedef struct
{
	uint16_t	ai_data[AI_MAX_CNT];//
	uint64_t	ai_mask;
}ain_st;

#define DI_COMP_1_HI_TEMP_POS			((uint32_t)0x00000001<<0)
#define DI_COMP_2_HI_TEMP_POS			((uint32_t)0x00000001<<1)
#define DI_COMP_1_LOW_TEMP_POS		((uint32_t)0x00000001<<2)
#define DI_COMP_2_LOW_TEMP_POS		((uint32_t)0x00000001<<3)
#define DI_COMP_1_DISC_TEMP_POS		((uint32_t)0x00000001<<4)
#define DI_COMP_2_DISC_TEMP_POS		((uint32_t)0x00000001<<5)
#define DI_FAN_1_OVF_POS					((uint32_t)0x00000001<<6)
#define DI_FAN_2_OVF_POS					((uint32_t)0x00000001<<7)
#define DI_FAN_3_OVF_POS					((uint32_t)0x00000001<<8)
#define DI_AIR_LOSS_POS						((uint32_t)0x00000001<<9)
#define DI_FILTER_CLOG_POS				((uint32_t)0x00000001<<10)
#define DI_WATER_OVER_FLOW_POS		((uint32_t)0x00000001<<11)
#define DI_RMT_SHUT_POS						((uint32_t)0x00000001<<12)
#define DI_HUM_WATER_LV						((uint32_t)0x00000001<<13)
#define DI_RESERVE_2_POS					((uint32_t)0x00000001<<14)
#define DI_RESERVE_3_POS					((uint32_t)0x00000001<<15)

#define ST_PWR_PA_AB_POS					((uint32_t)0x00000001<<16)
#define ST_PWR_PB_AB_POS					((uint32_t)0x00000001<<17)
#define ST_PWR_PC_AB_POS					((uint32_t)0x00000001<<18)
#define ST_HUM_WL_H_POS						((uint32_t)0x00000001<<19)
#define ST_HUM_HC_H_POS						((uint32_t)0x00000001<<20)
#define ST_HUM_WQ_L_POS						((uint32_t)0x00000001<<21)

//Digtal input status
/*
bit map:
bit0: 	compressor 1 hi temp valve
bit1: 	compressor 2 hi temp valve
bit2: 	compressor 1 low temp valve
bit3: 	compressor 2 low temp valve
bit4: 	compressor 1 discharge temp valve
bit5:		compressor 2 discharge temp valve
bit6: 	fan 1 overload valve
bit7: 	fan 2 overload valve
bit8:		fan 3 overload valve
bit9:		air lost valve
bit10:	filter clog valve
bit11:	water overflow valve
bit12:	remote shut valve
bit13:	reserve1
bit14:	reserve2
bit15:	reserve3

bit16:	power phase A error 
bit17:	power phase B error 
bit18:	power phase C error 
bit19:	humidifier water level high
bit20:	humidifier heating current high
bit21:	humidifier conductivity low
*/

typedef struct
{
	uint32_t	din_data;
	uint32_t	din_mask;
}din_st;


///////////////////////////////////////////////////////////////
//system output status 
///////////////////////////////////////////////////////////////

//analog_out
//this feature is not yet determined, reserve interface for future application
typedef struct
{
	int16_t	ec_fan[3];
	int16_t	vf_compressor[2];
	int16_t	reserve_aout[2];
}aout_st;

//Digital output definition
typedef struct
{
	int16_t	fan_out[MAX_FAN_NUM];
	int16_t	compressor_out[MAX_COMPRESSOR_NUM];
	int16_t	heater_out[MAX_HEATER_NUM];
	int16_t	liq_val_bypass_out[MAX_COMPRESSOR_NUM];
	int16_t	hot_gas_bypass_out[MAX_COMPRESSOR_NUM];
	int16_t	humidifier_out;
	int16_t	dehumidification_out;
	int16_t	water_injection_out;
	int16_t	common_alarm_out;
	int16_t	scr_out;
	int16_t	usr_out[DO_MAX_CNT];
}dout_st;

///////////////////////////////////////////////////////////////
//system log
///////////////////////////////////////////////////////////////
//alarm status
typedef struct
{
	int16_t 	alarm_id;
	time_t		trigger_time;
}alarm_status_st;

//alarm history
typedef struct
{
	int16_t alarm_id;
	time_t		trigger_time;
	time_t		clear_time;	
}alarm_history_st;

//alarm system runtime log, record components accumulative running time
/*
@comp_id:
	0:	compressor 1
	1:	compressor 2
	2:	fan 1
	3:	fan 2
	4:	fan 3
	5:	heater 1
	6:	heater 2
	7:	humidifier
@action:
	0:	deactivated
	1:	activated
@trigger_time:
	sys_time
*/

typedef struct
{
	int16_t comp1_runtime_day;
	int16_t comp1_runtime_min;
	int16_t comp2_runtime_day;
	int16_t comp2_runtime_min;
	int16_t fan1_runtime_day;
	int16_t fan1_runtime_min;
	int16_t fan2_runtime_day;
	int16_t fan2_runtime_min;
	int16_t fan3_runtime_day;
	int16_t fan3_runtime_min;
	int16_t heater1_runtime_day;
	int16_t heater1_runtime_min;
	int16_t heater2_runtime_day;
	int16_t heater2_runtime_min;
	int16_t humidifier_runtime_day;	
	int16_t humidifier_runtime_min;
}sys_runtime_log_st;

//alarm system runtime log, record components change of output states 
/*
@comp_id:
	0:	compressor 1
	1:	compressor 2
	2:	fan 1
	3:	fan 2
	4:	fan 3
	5:	heater 1
	6:	heater 2
	7:	humidifier
@action:
	0:	deactivated
	1:	activated
@trigger_time:
	sys_time
*/
typedef struct
{
	uint16_t comp_id;
	uint16_t action;
	time_t		trigger_time;
	time_t		clear_time;
}sys_status_log_st;


///////////////////////////////////////////////////////////////
//alarms definition
///////////////////////////////////////////////////////////////

//alarms: acl definition
/*
@id:			alarm id
@delay:		trigger&clear delay 
@timeout:	delay timeout count down
@trigger_time:	alarm trigger time
@enable mode:	alarm enable mode
`0x00:		enable
`0x01:		suspend
`0x02:		forbid
@enable mask:	alarm enable mask
'0x03:	all mode enable
'0x02:	enable or forbid 
'0x01:	enable or suspend
'0x00:	only enable
@alarm_param:	related paramter(eg. threshold)
@void (*alarm_proc): designated alarm routine check function
*/

typedef struct
{
	uint16_t 					id;
	uint16_t					delay;
	uint16_t					enable_mode;
	uint16_t 					alarm_param;
}alarm_acl_conf_st;


//system memory configuration map
typedef struct sys_conf_map
{
	int16_t 					id;
	void*						str_ptr;
	int16_t						length;
}sys_conf_map_st;


typedef struct sys_status_map
{
	int16_t 					id;
	int16_t	*					addr;
	int16_t						length;
}sys_status_map_st;


//������ˮ������
typedef struct
{
		uint16_t      u16SN_Code[4];//SN��
		uint16_t      u16M_Type;//�豸����
		uint16_t      u16Power_Mode;//���ػ�
		uint16_t      u16Start_Temp;//��ˮ�����¶�
		uint16_t      u16Start_Humidity;//��ˮ����ʪ��
		uint16_t      u16Stop_Temp;//��ˮֹͣ�¶�
		uint16_t      u16Stop_Humidity;//��ˮֹͣʪ��
		uint16_t      u16Start_Defrost_Temp;//��˪�����¶�
		uint16_t      u16Stop_Defrost_Temp;//��˪ֹͣ�¶�
		uint16_t      u16Sterilize_Mode;//ɱ��ģʽ:BIT0-220V,BIT1-24V
		uint16_t      u16Sterilize_Time[2];//ɱ��ʱ��
		uint16_t      u16Sterilize_Interval[2];//ɱ�����
		uint16_t      u16Water_Ctrl;//ˮ·���Ʒ���
		uint16_t      u16Water_Mode;//��ˮģʽ
		uint16_t      u16Water_Flow;//��ˮ����
		uint16_t      u16NormalWater_Temp;//����ˮ�¶�
		uint16_t      u16HotWater_Temp;//��ˮ�¶�
		uint16_t      u16WaterSource_Mode;//���ˮԴģʽ
		uint16_t      u16Change_WaterTank;//����Դˮ��
		uint16_t      u16Rsv0[4];//
		uint16_t      u16Reset;//�ָ�����
		uint16_t      u16Test_Mode_Type;//����ģʽѡ��
		uint16_t      u16Manual_Test_En;//�ֶ�����ģʽʹ��
		uint16_t      u16BITMAP_MANUAL;//�ֶ����
		uint16_t      u16TPower_En;//��ʱ���ػ�
		uint16_t      u16TPower_On;//��ʱ����ʱ��
		uint16_t      u16TPower_Off;//��ʱ�ػ�ʱ��
		uint16_t      u16Rsv1[3];//
		uint16_t      u16FILTER_ELEMENT_Type;//�����澯����
		uint16_t      u16Clear_RT;//�������ʱ��
		uint16_t      u16Clear_ALARM;//����澯
		uint16_t      u16Set_Time[2];//����ϵͳʱ��
		uint16_t      u16Fan_Start_Delay;//���������ʱ
		uint16_t      u16Fan_Stop_Delay;//����ر���ʱ
		uint16_t      u16Comp_Interval;//ѹ�����
		uint16_t      u16ColdWater_Mode;//��ˮģʽ
		uint16_t      u16ColdWater_StartTemp;//�Ʊ�ˮ�¶�
		uint16_t      u16ColdWater_StopTemp;//�Ʊ�ˮֹͣ�¶�		
}ComPara_Conf_st;


typedef struct
{
	uint16_t    eevproType;				//�������ͷ�����
	uint16_t    excSpeed;			    //�����ٶ� pps
	
	uint16_t    excAllOpenSteps;		//ȫ������
	uint16_t    excOpenValveMinSteps;   //��������--��С����      //@2017-08-21
	uint16_t    excOpenValveMinDegree;  //�ط�ʱ�����ȵ���С����ֵ
	uint16_t    eevHoldTime;				//unit 100ms      
	uint16_t    restore_factory_setting;
}Moter_st;

//ƽ̨ͨ�Ų���
typedef struct
{
	uint16_t    Fixed_Report;		  //��ʱ�ϱ����
	uint16_t    Real_Report;			//ʵʱ�ϱ����
}Platform_st;

typedef struct
{
	conf_general_st 			general;
	dev_mask_st						dev_mask;
//	power_supply_st       ac_power_supply;
	algorithm_st					algorithm;
	alarm_acl_conf_st			alarm[ACL_TOTAL_NUM];
//	mbm_Conf_st           mbm_Conf;
	ComPara_Conf_st				ComPara;
	fan_st								fan;
	compressor_st					compressor;	
	Moter_st 							Moter;
	Platform_st           Platform;
}config_st;

typedef struct
{
		uint16_t 			dev_sts;
		uint16_t 			conductivity;
		uint16_t 			hum_current;
		uint16_t 			water_level;
}mbm_hum_st;

//pwr
typedef struct{
	 uint16_t  status;           
	 uint16_t  output_voltage;
	 uint16_t  output_current;
	 uint16_t  pwr_on;
	 uint16_t  set_voltage;
	 uint16_t  set_current;
}pwr_imf_st;

typedef struct
{
		uint16_t 			dev_sts;
		uint16_t 			temp;
		uint16_t 			hum;
}mbm_tnh_st;

typedef struct
{
		uint16_t 			Temp;
		uint16_t 			Hum;
}Com_tnh_st;

//system information
typedef struct
{
	uint16_t	 			status_reg_num;													
	uint16_t	 			config_reg_num;
	uint16_t	 			software_ver;
	uint16_t	 			hardware_ver;
	uint16_t	 			serial_no[4];
	uint16_t	 			man_date[2];
}sys_info_st;

//modbus master data structure
typedef struct
{
	mbm_tnh_st 			tnh[TEMP_HUM_SENSOR_NUM];			//��ʪ��
//	mbm_IPM_St      IPM;//IPM���
//	mbm_PDU_St      PDU[4];//PDU
//	mbm_AC_St       AC;//�յ�
////	mbm_UPS_APC_St			UPS;
////#if (UPS_TYPE==UPS_EMERSON_ITA2)
////	mbm_UPS_ITA2_St 		UPS;
////#elif (UPS_TYPE==UPS_JYD_SIN)
////	mbm_UPS_JYD_SIN_St	UPS;
////#endif
//	mbm_UPS_HW_St		UPS;
	mbm_Error_St    Err_M;
}mbm_sts_st;

typedef struct
{
		uint16_t low;
		uint16_t high;
}run_time_st;

typedef struct
{
		uint16_t pwr_off_alarm;
		uint16_t critical_cnt;
		uint16_t major_cnt;
		uint16_t mioor_cnt;
		uint16_t total_cnt;
}alarm_state_cnt_st;

typedef struct{
	uint16_t work_mode;
	uint16_t limit_day;
	uint16_t runing_day;
	uint16_t runing_hour;
	uint16_t runing_sec;
	uint16_t runing_State;
	uint8_t pass_word[4];	
}work_mode_st;

typedef struct
{
		uint16_t Grade_Manage;//�ܿؽ׶�
		uint16_t Password_Poweron;//�����ܿ�����
		uint16_t Password_Grade[4][2];//1-4���ܿ�����������
		uint16_t Remain_day;//��ǰ�׶�ʣ������
		uint16_t Run_day;//��ǰ�׶���������
		uint16_t Run_hour;//��ǰ�׶�����Сʱ
		uint16_t Run_second;//��ǰ�׶�������
	  uint16_t Run_State;//����״̬
}ControlPassword_st;

typedef enum{
	RETURN_AIR_PLUSS_MODE = 0,
	SET_FAN_SPEED_MODE,
}return_air_mode_st;
typedef struct
{
	 uint16_t           timer;
	 return_air_mode_st  return_air_work_mode;
}return_air_sta_st;

typedef struct
{
		uint16_t 			Sec;
		uint16_t 			Min;
		uint16_t 			Hour;
		uint16_t 			Day;
		uint16_t 			Mon;
		uint16_t 			Year;
		uint16_t 			Weekday;
	  time_t 		    u32Systime;
}System_Time_st;

typedef struct
{
	 uint16_t       u16Hardware_ver;
	 uint16_t       u16Software_ver;
	 uint16_t       u16Status_remap[4];
	 uint16_t       u16Din_bitmap[2];
	 uint16_t       u16Dout_bitmap[2];
	 uint16_t       u16Alarm_bitmap[ALARM_TOTAL_WORD];
	 uint16_t       u16Ain[AI_MAX_CNT];//
	 uint16_t       u16AO[AO_MAX_CNT];
	 Com_tnh_st     u16TH[2];//��ʪ��
	 uint16_t       u16Pluse_CNT;
	 uint16_t       u16Cur_Water;
	 uint16_t       u16Last_Water;
	 uint16_t       u16Cumulative_Water[2];//�ۼ�ȡˮ
	 uint16_t       u16PM25;
	 uint16_t       u16Rev1[3];
	 uint16_t       u16Runtime[2][DO_MAX_CNT];//ʹ��ʱ��
	 uint16_t       u16WL;//ˮλ
	System_Time_st  Sys_Time;//ϵͳʱ��
		uint16_t    TEST; 
		uint16_t    TEST2; 
		uint16_t    TEST3; 
		uint16_t    TEST4; 
			uint16_t    TEST5; 
			uint16_t    TEST6; 
			uint16_t    REQ_TEST[3]; 
}ComSta_st;


typedef struct
{
		sys_info_st						sys_info;
		status_general_st			general;												//3
		mbm_sts_st						mbm;														//25
		uint16_t 							ain[AI_MAX_CNT];//
		uint16_t							aout[AO_MAX_CNT];								//6
//		uint16_t							CFM;														//�ܷ���
//    uint16_t							pwmout[PWM_MAX_CNT];						//2
		uint16_t							din_bitmap[2];									//2
		uint16_t							dout_bitmap[2];										//2
		uint16_t							status_remap[4];								//4
		uint16_t							alarm_bitmap[7];								//6	
		uint16_t							Alarm_COM_NTC_BIT[2];						//2	
//		uint16_t							flash_program_flag;							//1
//		run_time_st						run_time[DO_MAX_CNT];	
		alarm_state_cnt_st    alarm_status_cnt;
		sys_tem_hum_st        sys_tem_hum;
//		work_mode_st          sys_work_mode;
//		uint16_t              flow_diff_timer;
//		return_air_sta_st     return_air_status;
//		uint16_t							Hum_Water_Level;								//1
//		ControlPassword_st			ControlPassword;//5������ܿ�
		ComSta_st             ComSta;
}status_st;


typedef struct
{
	config_st config;
	status_st status;	
}sys_reg_st;


typedef enum
{
	TEM_HUM_SENSOR1_BPOS=0,
	TEM_HUM_SENSOR2_BPOS,
	TEM_HUM_SENSOR3_BPOS,
	TEM_HUM_SENSOR4_BPOS,
	TEM_HUM_SENSOR5_BPOS,
	TEM_HUM_SENSOR6_BPOS,
	TEM_HUM_SENSOR7_BPOS,
	TEM_HUM_SENSOR8_BPOS,
	//
	POWER_MODULE_BPOS,
	AC_MODULE_BPOS,
	UPS_MODULE_BPOS,
	RESERVE_GSM_BPOS,
	PDU_MODULE_BPOS,
	PDU_01_MODULE_BPOS,
	PDU_02_MODULE_BPOS,
	PDU_03_MODULE_BPOS,
}DEV_MASK_MB_BPOS;


#endif //	__SYS_CONF


