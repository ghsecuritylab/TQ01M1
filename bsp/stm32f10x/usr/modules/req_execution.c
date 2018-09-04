#include <rtthread.h>
#include "team.h"
#include "calc.h"
#include "sys_conf.h"
#include "local_status.h"
#include "req_execution.h"
#include "alarm_acl_funcs.h"
#include "sys_status.h"
#include "dio_bsp.h"
#include "rtc_bsp.h"
#include <stdlib.h>
#include <math.h>
#include "global_var.h"
#include "pwm_bsp.h"
#include "usart_bsp.h"

//�����л�״̬
enum
{
		PPE_FSM_POWER_ON,
		PPE_FSM_PN_SWITCH,
		PPE_FSM_NO_CONF,
		PPE_FSM_NORMAL,
		PPE_FSM_REVERSE
};


//˫��Դ�л�״̬
enum
{
		DP_FSM_IDLE=0,
		DP_FSM_NORMAL_A,
		DP_FSM_SWITCH_A,
		DP_FSM_NORMAL_B,
		DP_FSM_SWITCH_B,
		DP_FSM_APART,
};
//˫��Դ�쳣״̬
enum
{
		IO_ABNORMAL=0x01,
		A_ABNORMAL=0x02,
		B_ABNORMAL=0x04,
};
enum
{
		SWITCH_NO=0,//��˫��Դ�л�
		SWITCH_A,//ѡ��A·
		SWITCH_B,//ѡ��B·
};

#define INIT_DELAY 	10

#define COMPRESSOR_BITMAP_MASK_POS				0x0006
#define HEATER_BITMAP_MASK_POS						0x0418
#define HUMIDIFIER_BITMAP_MASK_POS				0x0020
#define DEHUMER_BITMAP_MASK_POS						0x0300

#define HUM_CHECKE_INTERVAL     12*3600


enum
{
		RUNING_STATUS_COOLING_BPOS=0,
		RUNING_STATUS_HEATING_BPOS,
		RUNING_STATUS_HUMIDIFYING_BPOS,
		RUNING_STATUS_DEHUMING_BPOS,
};

enum
{
		COMPRESSOR_SIG_HOLD=0,
		COMPRESSOR_SIG_ON,
		COMPRESSOR_SIG_OFF,
		COMPRESSOR_SIG_ERR,
};

enum
{
    WATER_COOLED_SIG_HOLD = 0,
    WATER_COOLED_SIG_ON,
    WATER_COOLED_SIG_OFF,
    WATER_COOLED_SIG_ERR,
};

enum
{
		HEATER_SIG_IDLE=0,
		HEATER_SIG_L1,
		HEATER_SIG_L2,
		HEATER_SIG_L3,
		HEATER_SIG_ERR,
};

enum
{
		HEATER_FSM_IDLE=0,
		HEATER_FSM_L1,
		HEATER_FSM_L2,
		HEATER_FSM_L3,
		HEATER_FSM_ERR,
};




enum
{
		FAN_SIG_IDLE=0,
		FAN_SIG_START,
		FAN_SIG_STOP
};


enum
{
		FAN_TPYE_AC=0,
		FAN_TPYE_EC
};

typedef struct{
		uint16_t time_out;
		uint16_t flush_delay_timer;
		uint16_t hum_fill_cnt;
		uint32_t hum_timer;
		uint32_t check_timer;
		uint8_t  check_fill_flag;
		uint8_t  check_drain_flag;
		uint8_t  check_flag;
		uint16_t warm_time;
}hum_timer;

//static hum_timer hum_delay_timer;


//�������λ��������
void req_bitmap_op(uint8_t component_bpos, uint8_t action)
{
		extern local_reg_st l_sys;
		uint8_t byte_offset,bit_offset;
	
		byte_offset =  component_bpos >> 4;
		bit_offset = component_bpos & 0x0f;
	
		if(action == 0)
		{
				l_sys.bitmap[byte_offset][BITMAP_REQ] &= ~(0x0001<<bit_offset);
		}
		else
		{			
				l_sys.bitmap[byte_offset][BITMAP_REQ] |= (0x0001<<bit_offset);
		}
		
//			if(action == 0)
//			{
//					l_sys.bitmap[BITMAP_REQ] &= ~(0x0001<<component_bpos);
//			}
//			else
//			{			
//					l_sys.bitmap[BITMAP_REQ] |= (0x0001<<component_bpos);
//			}
}

//����ģ�������������
static void req_ao_op(uint8_t component_bpos, int16_t value)
{
		extern local_reg_st l_sys;
    extern sys_reg_st   g_sys;
  
    if((g_sys.config.ComPara.u16Manual_Test_En == 0)&&(g_sys.config.ComPara.u16Test_Mode_Type == 0))
    {   //normal mode
        switch(component_bpos)
        {
            case (AO_EC_FAN):
            {
                if(1)
                {
                    l_sys.ao_list[component_bpos][BITMAP_REQ]     = value;
                }
                break;
            }
            case (AO_EC_COMPRESSOR):
            {
                if(1)
                {
                    l_sys.ao_list[component_bpos][BITMAP_REQ]     = value;
                }
                break;
            }
            default:
            {
                l_sys.ao_list[component_bpos][BITMAP_REQ]         = value;
                break;
            }				
        }
    }
    else
    {   //dianose or test mode, output directly
        l_sys.ao_list[component_bpos][BITMAP_REQ]     = value;
    }    
}

//static void req_pwm_op(uint8_t component_bpos, int16_t value)
//{
//		extern local_reg_st l_sys;

//		l_sys.pwm_list[component_bpos][BITMAP_REQ] = value;
//}
//ģ��������ٺ�����������Ŀ�꣬���ղ����������б仯��
static int16_t analog_step_follower(int16_t target, uint16_t dev_type)
{
    extern sys_reg_st		g_sys;
    uint16_t current_output;
    int16_t ret_val;
    int16_t delta;

    switch (dev_type)
    {
        case (AO_EC_FAN):
        {
            current_output = g_sys.status.aout[AO_EC_FAN];
            delta = target - current_output;
            if(abs(delta) > g_sys.config.fan.adjust_step)
            {
                if(delta > 0)
                    ret_val = current_output + g_sys.config.fan.adjust_step;
                else
                    ret_val = current_output - g_sys.config.fan.adjust_step;
            }
            else if(delta == 0)
            {
                ret_val = current_output;
            }
            else
            {
                ret_val = current_output + delta;
            }       
            break;
        }
        default:
        {
            ret_val = target;
            break;
        }
    }
    return ret_val;
}


//�����λ���
static void Fan_Fsm_Out(uint8_t Fan_Gear)
{
	
			switch(Fan_Gear)
			{
				case FAN_GEAR_START:
								req_bitmap_op(DO_FAN_LOW_BPOS,1);
								break;
				case FAN_GEAR_NO:
								req_bitmap_op(DO_FAN_LOW_BPOS,0);
								break;
				default:
								req_bitmap_op(DO_FAN_LOW_BPOS,0);
								break;			
			}					
}
	
/**
  * @brief 	fan output control state FSM execution
	* @param  none
	* @retval none
  */
//���״̬��ִ�к���
static void fan_fsm_exe(uint8_t fan_signal)
{
		extern sys_reg_st		g_sys;
		extern local_reg_st l_sys;

//		rt_kprintf("FAN_FSM_STATE = %d,fan_signal = %d\n",l_sys.l_fsm_state[FAN_FSM_STATE],fan_signal);			
		switch(l_sys.l_fsm_state[FAN_FSM_STATE])
		{
				case(FSM_FAN_IDLE):
				{
						if(fan_signal == FAN_SIG_START)
						{
								l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_INIT;
								l_sys.comp_timeout[DO_FAN_BPOS] = g_sys.config.ComPara.u16Fan_Start_Delay;		//assign startup delay to timeout counter
						}
						else
						{
								l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_IDLE;	
								l_sys.comp_timeout[DO_FAN_BPOS] = l_sys.comp_timeout[DO_FAN_BPOS];	//remains timeout counter
						}
						l_sys.Fan.Fan_Gear=FAN_GEAR_NO;//�����						//disable fan output						
						break;
				}		
				case(FSM_FAN_INIT):
				{
						if(fan_signal != FAN_SIG_START)
						{	
								l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_IDLE;	
								l_sys.Fan.Fan_Gear=FAN_GEAR_NO;//�����
								l_sys.comp_timeout[DO_FAN_BPOS] = l_sys.comp_timeout[DO_FAN_BPOS];	//reset timeout counter		
						}
						else
						{
								if(l_sys.comp_timeout[DO_FAN_BPOS] == 0)																
								{
										l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_START_UP;
									  l_sys.comp_timeout[DO_FAN_BPOS] = g_sys.config.fan.cold_start_delay;
										l_sys.Fan.Fan_Gear=FAN_GEAR_START;//
								}
								else	//wait until startup delay elapses
								{
										l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_INIT;	
										l_sys.Fan.Fan_Gear=FAN_GEAR_NO;//�����
								}
						}					
						break;
				}	
				case(FSM_FAN_START_UP):
				{
						if(l_sys.comp_timeout[DO_FAN_BPOS] == 0)
						{
								l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_NORM;
						}
						else
						{
								l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_START_UP;	
						}
							l_sys.Fan.Fan_Gear=FAN_GEAR_START;//
						l_sys.comp_timeout[DO_FAN_BPOS] = l_sys.comp_timeout[DO_FAN_BPOS];	//remain timeout counter
						break;
				}
				case(FSM_FAN_NORM):
				{							
						if((fan_signal == FAN_SIG_STOP)&&(l_sys.comp_timeout[DO_FAN_BPOS] == 0))
						{
								l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_SHUT;
								l_sys.comp_timeout[DO_FAN_BPOS] = g_sys.config.ComPara.u16Fan_Stop_Delay;			//assign startup delay to timeout counter
						}
						else
						{
								l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_NORM;	
								l_sys.comp_timeout[DO_FAN_BPOS] = l_sys.comp_timeout[DO_FAN_BPOS];	//reset timeout counter
						}
						l_sys.Fan.Fan_Gear=FAN_GEAR_START;//
						
						break;					
				}								
				case(FSM_FAN_SHUT):							
				{							
						if(fan_signal == FAN_SIG_START)
						{	
								l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_NORM;	
								l_sys.Fan.Fan_Gear=FAN_GEAR_START;//
								l_sys.comp_timeout[DO_FAN_BPOS] = l_sys.comp_timeout[DO_FAN_BPOS];					//reset timeout counter				
						}
						else
						{
								if(l_sys.comp_timeout[DO_FAN_BPOS] == 0)																
								{
										l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_IDLE;
										l_sys.Fan.Fan_Gear=FAN_GEAR_NO;//																			//enable fan output
										l_sys.comp_timeout[DO_FAN_BPOS] = l_sys.comp_timeout[DO_FAN_BPOS];																//reset timeout counter					
								}
								else																																		//wait until startup delay elapses
								{
										l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_SHUT;	
										l_sys.comp_timeout[DO_FAN_BPOS] = l_sys.comp_timeout[DO_FAN_BPOS];	//remain timeout counter											
								}
						}	
						break;
				}					
				default:
				{ 
						l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_IDLE;	
						l_sys.Fan.Fan_Gear=FAN_GEAR_NO;//	
						l_sys.comp_timeout[DO_FAN_BPOS] = 0;																			//reset timeout counter		
						break;
				}						
		}

//		if(g_sys.config.fan.type == FAN_TPYE_EC)
		{
			if((sys_get_pwr_sts() != 0)&&(l_sys.Fan.Fan_Gear!=FAN_GEAR_NO))
			{
				l_sys.Fan.Fan_Gear=FAN_GEAR_LOW;//����͵�
			}
		}
//		rt_kprintf("Fan_Gear = %d\n",l_sys.Fan.Fan_Gear);		
		Fan_Fsm_Out(l_sys.Fan.Fan_Gear);//���DO���					
}

#define EC_FAN_NOLOAD_DELAY   20
static void ec_fan_output(int16_t req_temp, int16_t req_hum,uint8_t fan_sig)
{
		extern sys_reg_st		g_sys;
		extern local_reg_st 	l_sys;

		uint16_t status_sense;
		uint16_t require;
	  uint16_t fan_mode;
//		static uint16_t ec_fan_shut_delay = 0; 	
	
	  fan_mode = g_sys.config.fan.mode;

		require = 0;

		{
				if(l_sys.Fan.Fan_Gear==FAN_GEAR_NO)//δ�����
				{
						require = 0;
				}
				else if(g_sys.config.fan.type == FAN_TPYE_EC)
				{
						if(l_sys.l_fsm_state[FAN_FSM_STATE] == FSM_FAN_START_UP)
						{
								require = g_sys.config.fan.set_speed;
						}
						else
						{
								//fan mode in invertor compressor mode
								if(fan_mode == FAM_MODE_INV_COMP)//����
								{   								
												require = analog_step_follower(req_temp, AO_EC_FAN);
//												ec_fan_shut_delay = EC_FAN_NOLOAD_DELAY;                          
								}
								else
								{     										
										if((g_sys.config.fan.noload_down != 0)&&(status_sense == 0))
										{
												require = g_sys.config.fan.min_speed;
										}
										else
										{
												require = g_sys.config.fan.set_speed;
										}                  
								}   										
						}
						require = lim_min_max(g_sys.config.fan.min_speed,g_sys.config.fan.max_speed,require);	
				}
//				else if(g_sys.config.fan.type == FAN_TPYE_AC)
//				{
//						require = 100;
//				}
//				else
//				{
//						require = 0;
//				}	
		}
		req_ao_op(AO_EC_FAN,require);
}


/**************************************/
//���״̬���źŲ�����·
static uint8_t fan_signal_gen(void)
{
		extern sys_reg_st		g_sys;
		extern local_reg_st l_sys;
		uint8_t fan_signal;

		
		fan_signal=0;
		if((sys_get_pwr_sts() == 1)&&(l_sys.Fan_Close==0x00))
		{			
				fan_signal = FAN_SIG_START;
		}
		else if((sys_get_pwr_sts() == 0)||(l_sys.Fan_Close!=0x00))
		{		
//			if((g_sys.status.ComSta.u16Dout_bitmap&(~((0x0001<<DO_FAN_BPOS)|(0x0001<<DO_ALARM_BPOS)|(0x0001<<DO_RH1_BPOS)
//				|(0x0001<<DO_LAMP_BPOS)|(0x0001<<DO_HWP_BPOS)|(0x0001<<DO_PWP_BPOS)))) == 0)
			if((g_sys.status.ComSta.u16Dout_bitmap[0]&((0x0001<<DO_COMP1_BPOS)|(0x0001<<DO_COMP2_BPOS))) == 0)
			{
					fan_signal = FAN_SIG_STOP;				
			}
			else
			{
					fan_signal = FAN_SIG_IDLE;
			}			
		}  
		else
		{
				fan_signal = FAN_SIG_IDLE;
		}
//		rt_kprintf("fan_signal=%x,Fan_Close=%d\n",fan_signal,l_sys.Fan_Close);			
		return fan_signal;
}

//��ˮ
void fan_req_exe(int16_t target_req_temp,int16_t target_req_hum)
{
		uint8_t fan_signal;	
		fan_signal = fan_signal_gen();  //���״̬���źŲ���
		fan_fsm_exe(fan_signal);        //���״̬��ִ��
		ec_fan_output(target_req_temp,target_req_hum,fan_signal);		//���ģ���������		
}

static uint16_t compressor_signal_gen(int16_t req_temp, int16_t req_hum, uint8_t* comp1_sig, uint8_t* comp2_sig)
{
		extern sys_reg_st		g_sys;
		extern local_reg_st l_sys;		
		uint8_t comp1_alarm_flag,comp2_alarm_flag;
		uint16_t compressor_count;		
		
		if((g_sys.config.general.alarm_bypass_en == 0)&&((l_sys.bitmap[0][BITMAP_REQ]&(0x0001<<DO_COMP1_BPOS)) != 0)&&((l_sys.bitmap[0][BITMAP_ALARM]&(0x0001<<DO_COMP1_BPOS)) == 0)&&((l_sys.bitmap[0][BITMAP_MASK]&(0x0001<<DO_COMP1_BPOS)) != 0))
				comp1_alarm_flag = 1;
		else
				comp1_alarm_flag = 0;

		if((g_sys.config.general.alarm_bypass_en == 0)&&((l_sys.bitmap[0][BITMAP_REQ]&(0x0001<<DO_COMP2_BPOS)) != 0)&&((l_sys.bitmap[0][BITMAP_ALARM]&(0x0001<<DO_COMP2_BPOS)) == 0)&&((l_sys.bitmap[0][BITMAP_MASK]&(0x0001<<DO_COMP2_BPOS)) != 0))
				comp2_alarm_flag = 1;
		else
				comp2_alarm_flag = 0;		

		compressor_count = devinfo_get_compressor_cnt();
		
		if(sys_get_do_sts(DO_FAN_BPOS) == 0)		//fan disabled, emergency shutdown
		{
				*comp1_sig = COMPRESSOR_SIG_ERR;
				*comp2_sig = COMPRESSOR_SIG_ERR;
				l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 0;
				return 0;
		}			
		
		if((sys_get_pwr_sts() == 0)||(l_sys.Comp_Close!=0x00))
		{
				*comp1_sig = COMPRESSOR_SIG_OFF;
				*comp2_sig = COMPRESSOR_SIG_OFF;
				l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 0;
				return 0;		
		}			
		if((sys_get_remap_status(WORK_MODE_STS_REG_NO,FAN_STS_BPOS) != 0))
		{		
				if(compressor_count == 1)	//one compressor configured
				{							
						if(get_alarm_bitmap(ACL_E1)||get_alarm_bitmap(ACL_E2))//�澯
						{
								*comp1_sig = COMPRESSOR_SIG_OFF;
								*comp2_sig = COMPRESSOR_SIG_OFF;
						}
						else
						{
								*comp1_sig = COMPRESSOR_SIG_ON;
								*comp2_sig = COMPRESSOR_SIG_OFF;							
						}
						l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 0;
				}
				else if(compressor_count == 2)	//two compressors configured
				{
								switch(l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE])
								{
								case(0):
										{
												*comp1_sig = COMPRESSOR_SIG_OFF;
												*comp2_sig = COMPRESSOR_SIG_OFF;
												if((get_alarm_bitmap(ACL_E1)||get_alarm_bitmap(ACL_E2))||//ˮ���澯
														((comp1_alarm_flag&comp2_alarm_flag) != 0)||				//�澯
													((sys_get_remap_status(WORK_MODE_STS_REG_NO, DEFROST1_STS_BPOS) != 0)&&(sys_get_remap_status(WORK_MODE_STS_REG_NO, DEFROST2_STS_BPOS) != 0)))//��˪
												{
														l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 0;
												}
												else
												{
														l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 1;
												}
												break;
										}
								case(1):
										{
												*comp1_sig = COMPRESSOR_SIG_ON;
												*comp2_sig = COMPRESSOR_SIG_ON;
												if((get_alarm_bitmap(ACL_E1)||get_alarm_bitmap(ACL_E2))||//ˮ���澯
														((comp1_alarm_flag&comp2_alarm_flag) != 0)||				//�澯
													((sys_get_remap_status(WORK_MODE_STS_REG_NO, DEFROST1_STS_BPOS) != 0)&&(sys_get_remap_status(WORK_MODE_STS_REG_NO, DEFROST2_STS_BPOS) != 0)))//��˪
												{
														l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 0;
												}
												else if((comp1_alarm_flag == 1)||(sys_get_remap_status(WORK_MODE_STS_REG_NO, DEFROST1_STS_BPOS) != 0))				//alarm alternation
												{
														l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 1;
														*comp1_sig = COMPRESSOR_SIG_OFF;											
												}
												else if((comp2_alarm_flag == 1)||(sys_get_remap_status(WORK_MODE_STS_REG_NO, DEFROST2_STS_BPOS) != 0))			//alarm alternation
												{
														l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 1;
														*comp2_sig = COMPRESSOR_SIG_OFF;		
												}
												else
												{
														l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 1;
												}
												break;
										}			
								default:
										{
												*comp1_sig = COMPRESSOR_SIG_OFF;
												*comp2_sig = COMPRESSOR_SIG_OFF;
												l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 0;
												break;
										}
								}				
				}
				else
				{
						*comp1_sig = COMPRESSOR_SIG_OFF;
						*comp2_sig = COMPRESSOR_SIG_OFF;	
						l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 0;						
				}
		}
		else
		{
				*comp1_sig = COMPRESSOR_SIG_OFF;
				*comp2_sig = COMPRESSOR_SIG_OFF;		
				l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 0;
		}
		return 1;
}

 void compressor_alarm_signal_gen(uint8_t* comp1_sig,uint8_t* comp2_sig)
 {
		extern local_reg_st l_sys;
		
			
		if((get_alarm_bitmap_mask(DO_COMP1_BPOS)==1)&&(get_alarm_bitmap_op(DO_COMP1_BPOS)==0))
		{
				*comp1_sig = COMPRESSOR_SIG_OFF;
				if((l_sys.l_fsm_state[COMPRESS1_FSM_STATE] != COMPRESSOR_FSM_STATE_STOP)&&(l_sys.l_fsm_state[COMPRESS1_FSM_STATE] != COMPRESSOR_FSM_STATE_IDLE))
				{
						l_sys.l_fsm_state[COMPRESS1_FSM_STATE] = COMPRESSOR_FSM_STATE_STOP;
						
				}
		}
		
		if((get_alarm_bitmap_mask(DO_COMP2_BPOS)==1)&&(get_alarm_bitmap_op(DO_COMP2_BPOS)==0))
		{
				*comp2_sig = COMPRESSOR_SIG_OFF;
				if((l_sys.l_fsm_state[COMPRESS2_FSM_STATE] != COMPRESSOR_FSM_STATE_STOP)&&(l_sys.l_fsm_state[COMPRESS2_FSM_STATE] != COMPRESSOR_FSM_STATE_IDLE))
				{
						l_sys.l_fsm_state[COMPRESS2_FSM_STATE] = COMPRESSOR_FSM_STATE_STOP;
				}
		}

 }

//ѹ����״̬������ 
static void compressor_fsm(uint8_t compressor_id, uint8_t signal)
{
		extern sys_reg_st		g_sys;
		extern local_reg_st l_sys;	
		extern team_local_st team_local_inst;
		
		uint16_t compress_fsm_state;
		
		uint8_t l_fsm_state_id;
		uint8_t do_bpos;
		
		if(compressor_id == 0)
		{
				l_fsm_state_id = COMPRESS1_FSM_STATE;
				do_bpos = DO_COMP1_BPOS;
		}
		else
		{
				l_fsm_state_id = COMPRESS2_FSM_STATE;
				do_bpos = DO_COMP2_BPOS;				
		}
		
		compress_fsm_state = l_sys.l_fsm_state[l_fsm_state_id];

//		rt_kprintf("compressor_id=%d,signal=%d,,compress_fsm_state=%d,l_sys.Comp_Close=%d,l_sys.comp_startup_interval=%d\n",compressor_id,signal,compress_fsm_state,l_sys.Comp_Close,l_sys.comp_startup_interval);			
		
		switch(compress_fsm_state)
		{
				case(COMPRESSOR_FSM_STATE_IDLE):
				{
						if((signal == COMPRESSOR_SIG_ON)&&(l_sys.comp_timeout[do_bpos] == 0))
						{
								l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_INIT;
								l_sys.comp_timeout[do_bpos] = g_sys.config.compressor.startup_delay;
							
								req_bitmap_op(do_bpos,0);
						}
						else
						{
								l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_IDLE;
								l_sys.comp_timeout[do_bpos] = l_sys.comp_timeout[do_bpos];		
								req_bitmap_op(do_bpos,0);
						}
						break;
				}
				case(COMPRESSOR_FSM_STATE_INIT):
				{
						if(signal != COMPRESSOR_SIG_ON)
						{
								l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_IDLE;
								l_sys.comp_timeout[do_bpos] = 0;
								req_bitmap_op(do_bpos,0);						
						}
						else if((signal == COMPRESSOR_SIG_ON)&&(l_sys.comp_timeout[do_bpos] == 0))
						{
								if(l_sys.comp_startup_interval == 0)
								{
										l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_STARTUP;	
										l_sys.comp_timeout[do_bpos] = g_sys.config.compressor.startup_lowpress_shield;
										req_bitmap_op(do_bpos,1);
										l_sys.comp_startup_interval = g_sys.config.ComPara.u16Comp_Interval;
								}
								else
								{
										l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_INIT;
										l_sys.comp_timeout[do_bpos] = l_sys.comp_timeout[do_bpos];
										req_bitmap_op(do_bpos,0);
								}
						}
						else
						{
								l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_INIT;
								l_sys.comp_timeout[do_bpos] = l_sys.comp_timeout[do_bpos];		
								req_bitmap_op(do_bpos,0);
						}
						break;
				}				
				case(COMPRESSOR_FSM_STATE_STARTUP):
				{
						if(signal == COMPRESSOR_SIG_ERR)
						{
									l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_STOP;				
						}
						else if(l_sys.comp_timeout[do_bpos] == 0)
						{
								l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_NORMAL;								
								l_sys.comp_timeout[do_bpos] = (g_sys.config.compressor.min_runtime>g_sys.config.compressor.startup_lowpress_shield)? (g_sys.config.compressor.min_runtime - g_sys.config.compressor.startup_lowpress_shield):0;
								req_bitmap_op(do_bpos,1);
						}
						else
						{
								l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_STARTUP;
								l_sys.comp_timeout[do_bpos] = l_sys.comp_timeout[do_bpos];		
								req_bitmap_op(do_bpos,1);
						}
						break;
				}						
				case(COMPRESSOR_FSM_STATE_NORMAL):
				{
						if(signal == COMPRESSOR_SIG_ERR)
						{
									l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_STOP;					
						}
						else if((signal == COMPRESSOR_SIG_OFF)&&(l_sys.comp_timeout[do_bpos] == 0))
						{
								l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_SHUTING;
								l_sys.comp_timeout[do_bpos] = g_sys.config.compressor.stop_delay;
								req_bitmap_op(do_bpos,1);
						}
						else
						{
								l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_NORMAL;
								l_sys.comp_timeout[do_bpos] = l_sys.comp_timeout[do_bpos];		
								req_bitmap_op(do_bpos,1);
						}
						break;
				}					
				case(COMPRESSOR_FSM_STATE_SHUTING):
				{
						if(signal == COMPRESSOR_SIG_ERR)
						{
									l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_STOP;
						}
						else if((signal == COMPRESSOR_SIG_OFF)&&(l_sys.comp_timeout[do_bpos] == 0))
						{
									l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_STOP;													
						}
						else if(signal == COMPRESSOR_SIG_ON)
						{
								l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_NORMAL;
								l_sys.comp_timeout[do_bpos] = 0;
								req_bitmap_op(do_bpos,1);
						}
						else
						{
								l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_SHUTING;
								l_sys.comp_timeout[do_bpos] = l_sys.comp_timeout[do_bpos];		
								req_bitmap_op(do_bpos,1);
						}
						break;
				}	
				case(COMPRESSOR_FSM_STATE_STOP):
				{						
//						l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_IDLE;
//						l_sys.comp_timeout[do_bpos] = g_sys.config.compressor.min_stoptime;
//						req_bitmap_op(do_bpos,0);	
							if(l_sys.comp_stop_interval == 0)
							{
									l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_IDLE;
									l_sys.comp_timeout[do_bpos] = g_sys.config.compressor.min_stoptime;
									req_bitmap_op(do_bpos,0);	
									l_sys.comp_stop_interval = g_sys.config.ComPara.u16Comp_Interval;
							}
							else
							{
									l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_STOP;
							}					
						break;
				}									
		}
}

//compressor requirement execution 
static void compressor_req_exe(int16_t req_temp,int16_t req_hum)
{
		uint8_t comp1_sig, comp2_sig;		
		compressor_signal_gen(req_temp,req_hum,&comp1_sig,&comp2_sig);    //FSM signal generation
		compressor_alarm_signal_gen(&comp1_sig,&comp2_sig);                // compressors alarm

		compressor_fsm(0,comp1_sig);                                      //compressor 1 FSM execution
		compressor_fsm(1,comp2_sig);                                      //compressor 2 FSM execution
}

#define POWERTIME   5400
#define THINTERVAL  180

enum
{
	 FC_WL=0x01,
	 FC_PT=0x02,	
	 FC_TH=0x04,	
	 FC_WS=0x08,	
	 FC_CW=0x10,	
};
//���,ѹ����������
void Sys_Fan_CP_WL(void)
{
//		extern sys_reg_st		g_sys;	
//		extern local_reg_st l_sys;
//		uint16_t u16WL;
//		uint16_t Test=0;

//		if(sys_get_pwr_sts() == 1)
//		{
//				Test=1;
//				l_sys.PowerOn_Time++;			
//		}
//		else
//		{
//				l_sys.PowerOn_Time=0;	
//		}
//		
//		u16WL=Get_Water_level();
////		rt_kprintf("u16WL=%x,Test=%x,,TH_Check_Interval=%d,din_bitmap[0]=%x\n",u16WL,Test,l_sys.TH_Check_Interval,g_sys.status.ComSta.u16Din_bitmap[0]);			
////		//TEST
////		u16WL=0x13;
//		//���
//		if((u16WL==0x07)||(u16WL==0x0F)||(u16WL==0x18)||(u16WL==0x19)||(u16WL==0x1B)||(u16WL==0x1F)||(u16WL==0x38)||(u16WL==0x39)||(u16WL==0x3A)||(u16WL==0x3B)||(u16WL==0x3F))
//		{
//				Test|=0x02;
//				l_sys.Fan_Close=TRUE;
//				l_sys.Comp_Close=TRUE;
//		}
//		else if((u16WL<0x03)&&(l_sys.PowerOn_Time>=POWERTIME))//90����С����ˮλ
//		{
//				Test|=0x04;
//				l_sys.Fan_Close=TRUE;
//				l_sys.Comp_Close=TRUE;
//		}
//		else if((l_sys.TH_Check_Interval==0)&&((g_sys.status.ComSta.u16TH[0].Temp<g_sys.config.ComPara.u16Stop_Temp)||(g_sys.status.ComSta.u16TH[0].Hum<g_sys.config.ComPara.u16Stop_Humidity)))//��ʪ�Ȳ���������
//		{
//				Test|=0x08;
//				l_sys.TH_Check_Interval=THINTERVAL;//��ʪ�ȼ����
//				l_sys.Fan_Close=TRUE;
//				l_sys.Comp_Close=TRUE;
//		}
//		else if(g_sys.config.ComPara.u16WaterSource_Mode)//���ˮԴ
//		{
//				Test|=0x10;
//				l_sys.Fan_Close=TRUE;
//				l_sys.Comp_Close=TRUE;
//		}
//		else
//		{
//							Test|=0x20;
//				if((l_sys.TH_Check_Interval==0)&&((g_sys.status.ComSta.u16TH[0].Temp>=g_sys.config.ComPara.u16Start_Temp)&&(g_sys.status.ComSta.u16TH[0].Hum>=g_sys.config.ComPara.u16Start_Humidity)))//��ʪ����������
//				{
//												Test|=0x40;
//						l_sys.TH_Check_Interval=THINTERVAL;//��ʪ�ȼ����
//						l_sys.Fan_Close=FALSE;		
//						l_sys.Comp_Close=FALSE;	
//				}								
//		}	
//		rt_kprintf("u16WL=%x,Test=%x,,TH_Check_Interval=%d,din_bitmap[0]=%x\n",u16WL,Test,l_sys.TH_Check_Interval,g_sys.status.ComSta.u16Din_bitmap[0]);			

		extern sys_reg_st		g_sys;	
		extern local_reg_st l_sys;
		uint16_t u16WL;
		uint16_t Test=0;

		if(sys_get_pwr_sts() == 1)
		{
				Test=1;
				l_sys.PowerOn_Time++;			
		}
		else
		{
				l_sys.PowerOn_Time=0;	
		}
		
		u16WL=Get_Water_level();
//		rt_kprintf("u16WL=%x,Test=%x,,TH_Check_Interval=%d,din_bitmap[0]=%x\n",u16WL,Test,l_sys.TH_Check_Interval,g_sys.status.ComSta.u16Din_bitmap[0]);			
//		//TEST
//		u16WL=0x13;
		//ˮλ�쳣
//		if((u16WL==0x07)||(u16WL==0x0F)||(u16WL==0x18)||(u16WL==0x19)||(u16WL==0x1B)||(u16WL==0x1F)||(u16WL==0x38)||(u16WL==0x39)||(u16WL==0x3A)||(u16WL==0x3B)||(u16WL==0x3F))
		if((u16WL&S_U)||(u16WL&D_M)||(u16WL&D_U))
		{
				Test|=0x02;
				l_sys.Fan_Close |= FC_WL;
				l_sys.Comp_Close |= FC_WL;
		}
		else
		{
				l_sys.Fan_Close &= ~FC_WL;
				l_sys.Comp_Close &= ~FC_WL;
		}
//		//��ʱ��δ����ˮλ
//		if((u16WL<0x03)&&(l_sys.PowerOn_Time>=POWERTIME))//90����С����ˮλ
//		{
//				Test|=0x02;
//				l_sys.Fan_Close |= FC_PT;
//				l_sys.Comp_Close |= FC_PT;
//		}
//		else
//		{
//				l_sys.Fan_Close &= ~FC_PT;
//				l_sys.Comp_Close &= ~FC_PT;	
//		}
		
		if((l_sys.TH_Check_Interval==0)&&((g_sys.status.ComSta.u16TH[0].Temp<g_sys.config.ComPara.u16Stop_Temp)||(g_sys.status.ComSta.u16TH[0].Hum<g_sys.config.ComPara.u16Stop_Humidity)))//��ʪ�Ȳ���������
		{
				Test|=0x08;
				l_sys.TH_Check_Interval=THINTERVAL;//��ʪ�ȼ����
				l_sys.Fan_Close |= FC_TH;
				l_sys.Comp_Close |= FC_TH;
		}
		else 
		if((l_sys.TH_Check_Interval==0)&&((g_sys.status.ComSta.u16TH[0].Temp>=g_sys.config.ComPara.u16Start_Temp)&&(g_sys.status.ComSta.u16TH[0].Hum>=g_sys.config.ComPara.u16Start_Humidity)))//��ʪ����������
		{
										Test|=0x40;
				l_sys.TH_Check_Interval=THINTERVAL;//��ʪ�ȼ����
				l_sys.Fan_Close &= ~FC_TH;
				l_sys.Comp_Close &= ~FC_TH;
		}	
//		else
//		{
//				l_sys.Fan_Close &= ~FC_TH;
//				l_sys.Comp_Close &= ~FC_TH;
//		}		
		
		if(g_sys.config.ComPara.u16WaterSource_Mode)//���ˮԴ
		{
				Test|=0x10;
				l_sys.Fan_Close |= FC_WS;
				l_sys.Comp_Close |= FC_WS;
		}
		else
		{
				l_sys.Fan_Close &= ~FC_WS;
				l_sys.Comp_Close &= ~FC_WS;	
		}	
		
		if(l_sys.Cold_Water==TRUE)//�Ʊ�ˮ
		{
				Test|=0x20;
				l_sys.Fan_Close |= FC_CW;
				l_sys.Comp_Close |= FC_CW;
		}
		else
		{
				l_sys.Fan_Close &= ~FC_CW;
				l_sys.Comp_Close &= ~FC_CW;	
		}	
		g_sys.status.ComSta.REQ_TEST[0]=Test;
		g_sys.status.ComSta.REQ_TEST[1]=g_sys.status.ComSta.u16Din_bitmap[0];
		rt_kprintf("u16WL=%x,Test=%x,TH_Check_Interval=%d,din_bitmap[0]=%x,Fan_Close=%x,Comp_Close=%x\n",u16WL,Test,l_sys.TH_Check_Interval,g_sys.status.ComSta.u16Din_bitmap[0],l_sys.Fan_Close,l_sys.Comp_Close);			
			
		return;
}

//������
void Pwp_req_exe(void)
{
		extern sys_reg_st		g_sys;
		extern local_reg_st l_sys;	
		uint16_t u16WL;
		static uint8_t u8WL_Pwp=FALSE;
		
		u16WL=Get_Water_level();
		//������
//		if((u16WL==0x03)||(u16WL==0x07)||(u16WL==0x0B)||(u16WL==0x0F)||(u16WL==0x1B)||(u16WL==0x1F))
		//Դˮ����ˮλ
		if((u16WL&S_M)||(u16WL&S_U))
		{
				u8WL_Pwp=TRUE;
		}		
		
		if(u8WL_Pwp==TRUE)
		{
				l_sys.Pwp_Open=TRUE;	
				if(((u16WL&S_U)==0)&&((u16WL&S_M)==0)&&((u16WL&S_L)==0))//����ˮλ�˳�
				{
						u8WL_Pwp=FALSE;
				}					
		}		
		else if(g_sys.config.ComPara.u16Change_WaterTank)//����Դˮ��
		{
				l_sys.Pwp_Open=TRUE;		
		}
		else
		{
				l_sys.Pwp_Open=FALSE;			
		}
		//��������
		if(l_sys.Pwp_Open==TRUE)
		{
				req_bitmap_op(DO_PWP_BPOS,1);	
		}	
		else
		{
				req_bitmap_op(DO_PWP_BPOS,0);			
		}
		return;
}
//��������
uint16_t PluseCalc_Water(uint16_t PluseCnt)
{
		extern sys_reg_st		g_sys;
		extern local_reg_st l_sys;		
		uint16_t u16Water_Flow;	
		float fWflow;	
		if(PluseCnt<L300)//С��0.3L
		{
			fWflow=	(float)PluseCnt*L300_FACTOR;
		}
		else if(PluseCnt<L500)
		{
			fWflow=	(float)PluseCnt*L500_FACTOR;
		}
		else if(PluseCnt<L1000)
		{
			fWflow=	(float)PluseCnt*L1000_FACTOR;
		}
		else if(PluseCnt<L1500)
		{
			fWflow=	(float)PluseCnt*L1500_FACTOR;
		}
		else if(PluseCnt<L2000)
		{
			fWflow=	(float)PluseCnt*L2000_FACTOR;
		}
		else
		{
			fWflow=	(float)PluseCnt*L2000_FACTOR;			
		}
		
		u16Water_Flow =(uint16_t)fWflow;
		
		return u16Water_Flow;	
}
//������ˮ���
void WaterOut_Key(void)
{
		extern sys_reg_st		g_sys;
		extern local_reg_st l_sys;	
		
		//��ˮ 1
		if((sys_get_di_sts(DI_Cold_1_BPOS)==1))
		{
				l_sys.OutWater_Key |=WATER_NORMAL_ICE;		
				l_sys.OutWater_Delay[0] =	WATER_MAXTIME;		
		}
		else
		{
				l_sys.OutWater_Key &=~WATER_NORMAL_ICE;		
				l_sys.OutWater_Delay[0] =	0;				
		}		

		//��ˮ 2
		if((sys_get_di_sts(DI_Cold_2_BPOS)==1))
		{
				l_sys.OutWater_Key |=WATER_NORMAL_ICE_2;		
				l_sys.OutWater_Delay[2] =	WATER_MAXTIME;		
		}
		else
		{
				l_sys.OutWater_Key &=~WATER_NORMAL_ICE_2;		
				l_sys.OutWater_Delay[2] =	0;				
		}			
		
//		//ͯ��
//		if((sys_get_di_sts(DI_K3_BPOS)==1))
//		{
//				l_sys.ChildLock_Cnt[0]++;				
//		}
//		else
//		{
//				l_sys.ChildLock_Cnt[0]=0;						
//		}		
		
		
		if(l_sys.ChildLock_Cnt[0]>=ChildKey_Cnt)
		{
				l_sys.ChildLock_Cnt[0]=0;		
				l_sys.ChildLock_Key =1;
				l_sys.ChildLock_Cnt[1]=ChildKey_Lose;
		}
		//ͯ��ʹ��
		if(l_sys.ChildLock_Key)
		{
				//��ˮ
				if((sys_get_di_sts(DI_Heat_BPOS)==1))
				{
						l_sys.OutWater_Key |=WATER_HEAT;		
						l_sys.OutWater_Delay[1] =	WATER_MAXTIME;						
				}
				else
				{
						l_sys.OutWater_Key &=~WATER_HEAT;		
						l_sys.OutWater_Delay[1] =	0;						
				}					
		}
		else
		{
				if((sys_get_di_sts(DI_Heat_BPOS)==1))//��Ч
				{
		
				}
				else
				{
						l_sys.OutWater_Key &=~WATER_HEAT;	
						l_sys.OutWater_Delay[1] =	0;					
				}					
		}
		
		//ͯ��ָʾ
		if(l_sys.ChildLock_Cnt[1])
		{
				req_bitmap_op(DO_LED_LOCK_BPOS,1);	//LEDָʾ,����
		}
		else
		{
				req_bitmap_op(DO_LED_LOCK_BPOS,0);	//LEDָʾ			
				l_sys.ChildLock_Key=0;
		}
		
		return;
}
//�رճ�ˮ
void WaterOut_Close(uint8_t u8Type,uint8_t u8Water)
{
		extern sys_reg_st		g_sys;
		extern local_reg_st l_sys;
		uint32_t u32CW;
		uint8_t u8Temp;
		static uint8_t u8CloseNum;	
		
		if(!u8Type)
		{
			u8CloseNum=0;
		}
		else
		{
			l_sys.comp_timeout[DO_DV_BPOS]=0;//��ˮ��ʱ
			l_sys.OutWater_Flag=FALSE;//�رճ�ˮ
			if(l_sys.OutWater_OK)
			{
				l_sys.OutWater_OK=WATER_OUT;
				u32CW=g_sys.status.ComSta.u16Cumulative_Water[0];
				u32CW+=g_sys.status.ComSta.u16Cur_Water;
				g_sys.status.ComSta.u16Cumulative_Water[0]=u32CW;
				g_sys.status.ComSta.u16Cumulative_Water[1]=u32CW>>16;
			
				g_sys.status.ComSta.u16Last_Water=g_sys.status.ComSta.u16Cur_Water;
				g_sys.status.ComSta.u16Cur_Water=0;
				g_sys.config.ComPara.u16Water_Mode=0;
				g_sys.config.ComPara.u16Water_Flow=0;
				
				if(u8Water==WATER_NORMAL_ICE)
				{
						//����ˮ���
						req_bitmap_op(DO_UV2_BPOS,0);//�����			
					
						req_bitmap_op(DO_WP_BPOS,0);//��ˮ��
						req_bitmap_op(DO_DV_BPOS,0);//��ˮ��	
					
						req_bitmap_op(DO_WP2_BPOS,0);//��ˮ��2
						req_bitmap_op(DO_DV2_BPOS,0);//��ˮ��2	
				}
				else
				if(u8Water==WATER_HEAT)
				{			
						//��ˮ���
						req_bitmap_op(DO_HWP_BPOS,0);//��ˮ��ˮ��
						req_bitmap_op(DO_DV_BPOS,0);//��ˮ��		
				}					
			}	
			
			if(u8Water==WATER_HEAT)
			{
				l_sys.HeatWater_Time=0;
				l_sys.HeatWater_Flow=0;
				//����ʽ��ˮ��
				if(u8CloseNum<CLOSEHEAT_MAX)//�رճ�ˮ
				{
						u8CloseNum++;
						u8Temp=g_sys.config.ComPara.u16NormalWater_Temp/10;			
						if(Heat_Send(HEAT_WRITEPARA,CLOSE_HEAT,u8Temp,g_sys.config.ComPara.u16Water_Flow))
						{
							
						}
				}	
			}
		}			
}
//��ˮ
void WaterOut_req_exe(void)
{
		extern sys_reg_st		g_sys;
		extern local_reg_st l_sys;
		uint16_t u16WL;
		uint32_t u32CW;
		uint16_t u16Pls_Cnt;
		uint8_t u8Temp;
		static uint8_t u8HeatNum;

		//ˮλ	
		u16WL=Get_Water_level();
//		rt_kprintf("u16Din_bitmap=%x,u16WL=%x,u16Water_Mode=%d,u16Water_Flow=%d\n",g_sys.status.ComSta.u16Din_bitmap[0],u16WL,g_sys.config.ComPara.u16Water_Mode,g_sys.config.ComPara.u16Water_Flow);			

//		if(((u16WL&D_L)==0))//��ˮ���ˮλ
//		{
//			return;
//		}		
		req_bitmap_op(DO_RH1_BPOS,1);	//�����	
//					rt_kprintf("HeatWater_st=%d,HeatWater_Flow=%d,OutWater_OK=%d,u8HeatNum=%d,u16Cur_Water=%d\n",l_sys.HeatWater_st,l_sys.HeatWater_Flow,l_sys.OutWater_OK,u8HeatNum,g_sys.status.ComSta.u16Cur_Water);			
		if(g_sys.config.ComPara.u16Water_Ctrl&HMI_KEY)//������ˮ
		{		
				WaterOut_Key();
				if((!(l_sys.OutWater_Key))||(!(u16WL&D_L)))//��ˮ���ˮλ
				{
						u8HeatNum=0;
//						l_sys.OutWater_OK=0x00;		
						l_sys.HeatWater_st=HEAT_NO;					
						l_sys.HeatWater_Flow=0;
						WaterOut_Close(1,WATER_NORMAL_ICE);
						WaterOut_Close(1,WATER_HEAT);
						return;			
				}	
		}
		else//HMI��ˮ
		{
				if((!(g_sys.config.ComPara.u16Water_Mode)&&!(g_sys.config.ComPara.u16Water_Flow))||(!(u16WL&D_L)))//��ˮ���ˮλ,�������ˮ
				{
						if(!(u16WL&D_L))//��ˮ���ˮλ
						{
							g_sys.config.ComPara.u16Water_Mode=0;	
							g_sys.config.ComPara.u16Water_Flow=0;
						}
						u8HeatNum=0;
//						l_sys.OutWater_OK=0x00;		
						l_sys.HeatWater_st=HEAT_NO;					
						l_sys.HeatWater_Flow=0;
						WaterOut_Close(1,WATER_NORMAL_ICE);
						WaterOut_Close(1,WATER_HEAT);
						return;			
				}				
		}
//		rt_kprintf("HeatWater_st=%d,HeatWater_Flow=%d,OutWater_OK=%d,u8HeatNum=%d,u16Cur_Water=%d,OutWater_Key=%x\n",l_sys.HeatWater_st,l_sys.HeatWater_Flow,l_sys.OutWater_OK,u8HeatNum,g_sys.status.ComSta.u16Cur_Water,l_sys.OutWater_Key);					
		if(((g_sys.config.ComPara.u16Water_Mode==WATER_HEAT)&&(g_sys.config.ComPara.u16Water_Flow))||(l_sys.OutWater_Key&WATER_HEAT))//��ˮ
		{
				l_sys.comp_timeout[DO_DV_BPOS]++;//��ˮ��ʱ
			
				if(!(g_sys.config.ComPara.u16Water_Ctrl&HEART_POT))//����ʽ��ˮ
				{		
//						rt_kprintf("g_ComStat=%d,StatckStatus=%d,g_ComBuff[0]=%d,[1]=%d,[14]=%d,[15]=%d,TEST=%x,TEST2=%x,TEST3=%x\n",g_ComStat[0],Protocol[0].StatckStatus,g_ComBuff[0][0],g_ComBuff[0][1],g_ComBuff[0][14],g_ComBuff[0][15],g_sys.status.ComSta.TEST,g_sys.status.ComSta.TEST2,g_sys.status.ComSta.TEST3);	
//						if(l_sys.OutWater_OK==WATER_READ)
//						{
//								if(l_sys.HeatWater_st==HEAT_OUTWATER)//��ʼ��ˮ
//								{
//									u8HeatNum=WRITEHEAT_MAX;
//									WaterOut_Close(0,WATER_HEAT);
//								}
//								else
//								{
//									l_sys.OutWater_OK=HEATER_IDLE;	
//								}
//						}
//						if(u8HeatNum==WRITEHEAT_MAX)//�Ѿ���ˮ
//						{
//								g_sys.status.ComSta.u16Cur_Water=l_sys.HeatWater_Flow;		
//								if(g_sys.status.ComSta.u16Cur_Water)
//								{
//									g_sys.status.ComSta.u16Last_Water=g_sys.status.ComSta.u16Cur_Water;
//								}
//								l_sys.OutWater_Flag=TRUE;//��ˮ��
//						}
//						
//						if((l_sys.OutWater_OK==HEATER_SEND)||(l_sys.OutWater_OK==WATER_READ))
//						{
//								if((!l_sys.comp_timeout[DO_RH1_BPOS])&&(!g_sys.status.ComSta.u16Cur_Water))	
//								{
//										//��ˮ����
//								}
//						}
//						
//		//				rt_kprintf("HeatWater_st=%d,HeatWater_Flow=%d,OutWater_OK=%d,u8HeatNum=%d,u16Cur_Water=%d\n",l_sys.HeatWater_st,l_sys.HeatWater_Flow,l_sys.OutWater_OK,u8HeatNum,g_sys.status.ComSta.u16Cur_Water);	
//						if((g_sys.status.ComSta.u16Cur_Water>=g_sys.config.ComPara.u16Water_Flow)&&(g_sys.status.ComSta.u16Cur_Water))
//						{
//									WaterOut_Close(1,WATER_HEAT);
//						}
//						else	
//						{
//								if(((g_sys.config.ComPara.u16Water_Mode)&&(g_sys.config.ComPara.u16Water_Flow))||(l_sys.OutWater_Key&WATER_HEAT))
//								{	
//										//����ͨ��
//										if((l_sys.OutWater_OK==HEATER_SEND)||(l_sys.OutWater_OK==WATER_READ))
//										{
//												l_sys.OutWater_OK=WATER_READ;	
//												if(Heat_Send(HEAT_READPARA,0,0,0))
//												{
//														g_ComStat[UART_HEAT] = SEND_Over;                        //�������
//														g_sys.status.ComSta.TEST=0;
//												}
//										}
//										else
//										{
//												if((u8HeatNum<WRITEHEAT_MAX)&&(l_sys.HeatWater_st==HEAT_NO))//δ��ˮ
//												{
//														u8HeatNum++;
//														l_sys.OutWater_OK=HEATER_SEND;	
//														l_sys.HeatWater_st=HEAT_NO;					
//														l_sys.HeatWater_Flow=0;
//														if(g_sys.config.ComPara.u16Water_Mode==WATER_NORMAL_ICE)
//														{
//																u8Temp=g_sys.config.ComPara.u16NormalWater_Temp/10;													
//														}
//														else
//														{
//																u8Temp=g_sys.config.ComPara.u16HotWater_Temp/10;															
//														}
//														if(Heat_Send(HEAT_WRITEPARA,OPEN_HEAT,u8Temp,g_sys.config.ComPara.u16Water_Flow))
//														{
//															
//														}
//														l_sys.comp_timeout[DO_RH1_BPOS]=RH_DEALY;
//												}
//												else
//												{
//													
//												}
//										}
//										
//								}
//								
//						}

						if(l_sys.OutWater_OK==WATER_READ)
						{
								if(l_sys.HeatWater_st==HEAT_OUTWATER)//��ʼ��ˮ
								{
									WaterOut_Close(0,WATER_HEAT);
									
									g_sys.status.ComSta.u16Cur_Water=l_sys.HeatWater_Flow;		
									if(g_sys.status.ComSta.u16Cur_Water)
									{
										g_sys.status.ComSta.u16Last_Water=g_sys.status.ComSta.u16Cur_Water;
									}
									l_sys.OutWater_Flag=TRUE;//��ˮ��						
								}
								else//ʱ�����,500ml/MIN,
								{
										l_sys.HeatWater_Time+=1;//500msִ��һ��
										if(l_sys.HeatWater_Time)//��ʱ2��
										{
												WaterOut_Close(0,WATER_HEAT);
												
												l_sys.HeatWater_Flow=(l_sys.HeatWater_Time-2)*HEAT_FACTOR_500MS;
												g_sys.status.ComSta.u16Cur_Water=l_sys.HeatWater_Flow;		
												if(g_sys.status.ComSta.u16Cur_Water)
												{
													g_sys.status.ComSta.u16Last_Water=g_sys.status.ComSta.u16Cur_Water;
												}
												l_sys.OutWater_Flag=TRUE;//��ˮ��													
										}
									
								}
						}
						
						if((l_sys.OutWater_OK==HEATER_SEND)||(l_sys.OutWater_OK==WATER_READ))
						{
								if((!l_sys.comp_timeout[DO_RH1_BPOS])&&(!g_sys.status.ComSta.u16Cur_Water))	
								{
										//��ˮ����
								}
						}
						
		//				rt_kprintf("HeatWater_st=%d,HeatWater_Flow=%d,OutWater_OK=%d,u8HeatNum=%d,u16Cur_Water=%d\n",l_sys.HeatWater_st,l_sys.HeatWater_Flow,l_sys.OutWater_OK,u8HeatNum,g_sys.status.ComSta.u16Cur_Water);	
						//HMI��ˮʱ���ж�����
						if(((g_sys.status.ComSta.u16Cur_Water>=g_sys.config.ComPara.u16Water_Flow)&&(g_sys.status.ComSta.u16Cur_Water))&&(!(g_sys.config.ComPara.u16Water_Ctrl&HMI_KEY)))//HMI��ˮ
						{
									WaterOut_Close(1,WATER_HEAT);
						}
						else	
						{
								
								if(((g_sys.config.ComPara.u16Water_Mode)&&(g_sys.config.ComPara.u16Water_Flow))||(l_sys.OutWater_Key&WATER_HEAT))
								{	
										//����ͨ��
										if((l_sys.OutWater_OK==HEATER_SEND)&&(u8HeatNum>=3))
										{
												l_sys.OutWater_OK=WATER_READ;	
												if(Heat_Send(HEAT_READPARA,0,0,0))
												{
														g_ComStat[UART_HEAT] = SEND_Over;                        //�������
														g_sys.status.ComSta.TEST=0;
												}
										}
										else
										{
												if(u8HeatNum<WRITEHEAT_MAX)//
												{
														u8HeatNum++;
														l_sys.OutWater_OK=HEATER_SEND;	
														l_sys.HeatWater_st=HEAT_NO;					
														if(g_sys.config.ComPara.u16Water_Mode==WATER_NORMAL_ICE)
														{
																u8Temp=g_sys.config.ComPara.u16NormalWater_Temp/10;													
														}
														else
														{
																u8Temp=g_sys.config.ComPara.u16HotWater_Temp/10;															
														}
														
														if(l_sys.OutWater_Key&WATER_HEAT)//������ˮ,����������
														{
																g_sys.config.ComPara.u16Water_Flow=5000;
														}
														if(Heat_Send(HEAT_WRITEPARA,OPEN_HEAT,u8Temp,g_sys.config.ComPara.u16Water_Flow))
														{
															
														}
														l_sys.comp_timeout[DO_RH1_BPOS]=RH_DEALY;
												}
												else
												{
													
												}
										}
										
								}								
						}		

				}
				else//�ȹ�
				{
						u16Pls_Cnt=Read_Pluse_Cnt();
						//��������
						if(l_sys.OutWater_OK!=WATER_READ)
						{
								Clear_Pluse_Cnt(&u16Pls_Cnt);//�������
						}
						g_sys.status.ComSta.u16Pluse_CNT=u16Pls_Cnt;
						g_sys.status.ComSta.u16Cur_Water=PluseCalc_Water(u16Pls_Cnt);
						if(g_sys.status.ComSta.u16Cur_Water)
						{
							g_sys.status.ComSta.u16Last_Water=g_sys.status.ComSta.u16Cur_Water;
						}
						if(l_sys.OutWater_OK==WATER_READ)
						{
								if((!l_sys.comp_timeout[DO_RH1_BPOS])&&(!g_sys.status.ComSta.u16Cur_Water))	
								{
										//��ˮ����
								}
								WaterOut_Close(0,WATER_HEAT);
						}
		//				rt_kprintf("u16Pluse_CNT=%d,u16Cur_Water=%d,u16Pls_Cnt=%d\n",g_sys.status.ComSta.u16Pluse_CNT,g_sys.status.ComSta.u16Cur_Water,u16Pls_Cnt);					
						if((g_sys.config.ComPara.u16Water_Mode==WATER_HEAT)||(l_sys.OutWater_Key&WATER_HEAT))//��ˮ
						{
								//HMI��ˮʱ���ж�����
								if(((g_sys.status.ComSta.u16Cur_Water>=g_sys.config.ComPara.u16Water_Flow)&&(g_sys.status.ComSta.u16Cur_Water))&&(!(g_sys.config.ComPara.u16Water_Ctrl&HMI_KEY)))//HMI��ˮ
								{
											WaterOut_Close(1,WATER_HEAT);
								}
								else
								{
										l_sys.OutWater_OK=WATER_READ;		
										req_bitmap_op(DO_HWP_BPOS,1);//��ˮ��ˮ��
//										req_bitmap_op(DO_DV_BPOS,1);//��ˮ��		
										l_sys.comp_timeout[DO_RH1_BPOS]=RH_DEALY;		
										l_sys.OutWater_Flag=TRUE;//��ˮ��									
								}			
						}			
					
				}			
		}
		else if(((g_sys.config.ComPara.u16Water_Mode==WATER_NORMAL_ICE)&&(g_sys.config.ComPara.u16Water_Flow))||(l_sys.OutWater_Key&WATER_NORMAL_ICE)||(l_sys.OutWater_Key&WATER_NORMAL_ICE_2))//����ˮ/��ˮ
		{
				//��Ǵ�ʱ��//�رճ�ˮ
				if(sys_get_di_sts(DI_OPEN_BPOS)==0)
				{
						WaterOut_Close(1,WATER_NORMAL_ICE);
						WaterOut_Close(1,WATER_HEAT);
						return;
				}	
				l_sys.comp_timeout[DO_DV_BPOS]++;//��ˮ��ʱ
			
				u16Pls_Cnt=Read_Pluse_Cnt();
//		    rt_kprintf("OutWater_OK=%d,u16Pls_Cnt=%d\n",l_sys.OutWater_OK,u16Pls_Cnt);		
				//��������
				if(l_sys.OutWater_OK!=WATER_READ)
				{
						Clear_Pluse_Cnt(&u16Pls_Cnt);//�������
				}
				g_sys.status.ComSta.u16Pluse_CNT=u16Pls_Cnt;
				g_sys.status.ComSta.u16Cur_Water=PluseCalc_Water(u16Pls_Cnt);
				if(g_sys.status.ComSta.u16Cur_Water)
				{
					g_sys.status.ComSta.u16Last_Water=g_sys.status.ComSta.u16Cur_Water;
				}		
//		    rt_kprintf("u16Last_Water=%d,u16Cur_Water=%d,OutWater_OK==%d,u16Pls_Cnt=%d\n",g_sys.status.ComSta.u16Last_Water,g_sys.status.ComSta.u16Cur_Water,l_sys.OutWater_OK,g_sys.status.ComSta.u16Pluse_CNT);						
				if(l_sys.OutWater_OK==WATER_READ)
				{
						if((!l_sys.comp_timeout[DO_RH1_BPOS])&&(!g_sys.status.ComSta.u16Cur_Water))	
						{
								//��ˮ����
						}
						WaterOut_Close(0,WATER_HEAT);
				}
				//HMI��ˮʱ���ж�����
				if(((g_sys.status.ComSta.u16Cur_Water>=g_sys.config.ComPara.u16Water_Flow)&&(g_sys.status.ComSta.u16Cur_Water))&&(!(g_sys.config.ComPara.u16Water_Ctrl&HMI_KEY)))//HMI��ˮ
				{
							WaterOut_Close(1,WATER_NORMAL_ICE);
				}
				else
				{
						//��ˮ1
						if(((g_sys.config.ComPara.u16Water_Mode==WATER_NORMAL_ICE)&&(g_sys.config.ComPara.u16Water_Flow))||(l_sys.OutWater_Key&WATER_NORMAL_ICE))
						{
								req_bitmap_op(DO_WP_BPOS,1);//��ˮ��							
								req_bitmap_op(DO_DV_BPOS,1);//��ˮ��								
						}
												
						if(l_sys.OutWater_Key&WATER_NORMAL_ICE_2)
						{
								req_bitmap_op(DO_WP2_BPOS,1);//��ˮ��2							
								req_bitmap_op(DO_DV2_BPOS,1);//��ˮ��2	
						}
						
						l_sys.OutWater_OK=WATER_READ;		
						req_bitmap_op(DO_UV2_BPOS,1);//�����								
						l_sys.comp_timeout[DO_RH1_BPOS]=RH_DEALY;		
						l_sys.OutWater_Flag=TRUE;//��ˮ��						
				}			
		}
		else//�رճ�ˮ
		{
				WaterOut_Close(1,WATER_NORMAL_ICE);
				WaterOut_Close(1,WATER_HEAT);
		}
		//����ˮ����
		if(l_sys.comp_timeout[DO_DV_BPOS]>=WATER_MAXTIME)
		{
				WaterOut_Close(1,WATER_NORMAL_ICE);
				WaterOut_Close(1,WATER_HEAT);			
		}
		return;
}

//ɱ��
void Sterilize_req_exe(void)
{
		extern sys_reg_st		g_sys;
		extern local_reg_st l_sys;
		uint16_t u16WL;
		static uint32_t u32Sterilize_Interval[2];
		static uint16_t u16Sterilize_Time[2];
		uint8_t u8STR,i;
		static 	uint8_t  u8CNT=0;

		//��Ǵ�ʱ���ر������
		if(sys_get_di_sts(DI_OPEN_BPOS)==0)
		{
				req_bitmap_op(DO_UV1_BPOS,0);//�����
				req_bitmap_op(DO_DWP_BPOS,0);//ɱ����		
				return;
		}		
		//ˮλ	
		u16WL=Get_Water_level();
		if(u16WL<=0x07)//С����ˮ���ˮλ
		{
			return;
		}
		
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
		
		
		//220V�����
			u8STR=0;
			u32Sterilize_Interval[u8STR]++;	
			
			if(u32Sterilize_Interval[u8STR]>=(g_sys.config.ComPara.u16Sterilize_Interval[u8STR]*60))
			{
					u16Sterilize_Time[u8STR]++;
					req_bitmap_op(DO_UV1_BPOS,1);//�����
					req_bitmap_op(DO_DWP_BPOS,1);//ɱ����
			}
			
			if((sys_get_remap_status(WORK_MODE_STS_REG_NO, OUTWATER_STS_BPOS) == FALSE))				//Water out
			{		
				if(u16Sterilize_Time[u8STR]>=g_sys.config.ComPara.u16Sterilize_Time[u8STR]*60)
				{
						u32Sterilize_Interval[u8STR]=0;
						u16Sterilize_Time[u8STR]=0;
						req_bitmap_op(DO_UV1_BPOS,0);//�����
						req_bitmap_op(DO_DWP_BPOS,0);//ɱ����
				}
			}
		
//		//24V�����
//		if(g_sys.config.ComPara.u16Sterilize_Mode&STERILIZE_BIT1)
//		{	
//			u8STR=1;
//			u32Sterilize_Interval[u8STR]++;	
//			
//			if(u32Sterilize_Interval[u8STR]>=(g_sys.config.ComPara.u16Sterilize_Interval[u8STR]*60))
//			{
//					u16Sterilize_Time[u8STR]++;
//					req_bitmap_op(DO_UV2_BPOS,1);//�����	2
//					req_bitmap_op(DO_DWP_BPOS,1);//ɱ����
//			}
//			if((sys_get_remap_status(WORK_MODE_STS_REG_NO, OUTWATER_STS_BPOS) == FALSE))				//Water out
//			{		
//				if(u16Sterilize_Time[u8STR]>=g_sys.config.ComPara.u16Sterilize_Time[u8STR]*60)
//				{
//						u32Sterilize_Interval[u8STR]=0;
//						u16Sterilize_Time[u8STR]=0;
//						req_bitmap_op(DO_UV2_BPOS,0);//�����	2
//						req_bitmap_op(DO_DWP_BPOS,0);//ɱ����
//				}
//			}
//		}
		
		return;
}

//��˪
void Defrost_req_exe(void)
{
		extern sys_reg_st		g_sys;	
		extern local_reg_st l_sys;
		int16_t i16NTC1;		
		int16_t i16NTC2;	
		
		i16NTC1 =(int16_t)g_sys.status.ComSta.u16Ain[AI_NTC1];
		i16NTC2 =(int16_t)g_sys.status.ComSta.u16Ain[AI_NTC2];
	
		//set Deforost status
		if(i16NTC1<(int16_t)g_sys.config.ComPara.u16Start_Defrost_Temp)
		{
				sys_set_remap_status(WORK_MODE_STS_REG_NO,DEFROST1_STS_BPOS,1);					
		}
		else if(i16NTC1>(int16_t)g_sys.config.ComPara.u16Stop_Defrost_Temp)
		{
				sys_set_remap_status(WORK_MODE_STS_REG_NO,DEFROST1_STS_BPOS,0);				
		}

		if(i16NTC2<(int16_t)g_sys.config.ComPara.u16Start_Defrost_Temp)
		{
				sys_set_remap_status(WORK_MODE_STS_REG_NO,DEFROST2_STS_BPOS,1);					
		}
		else if(i16NTC2>(int16_t)g_sys.config.ComPara.u16Stop_Defrost_Temp)
		{
				sys_set_remap_status(WORK_MODE_STS_REG_NO,DEFROST2_STS_BPOS,0);				
		}		
		
		return;
}

//���ˮԴ
void External_Water_exe(void)
{
		extern sys_reg_st		g_sys;	
		extern local_reg_st l_sys;
		uint16_t u16WL;

	
		if(g_sys.config.ComPara.u16WaterSource_Mode)//���ˮԴ
		{
					//ˮλ	
				u16WL=Get_Water_level();
				if(u16WL&D_U)
				{
						req_bitmap_op(DO_FV_BPOS,0);//���ˮԴ					
				}
				else
				{
						req_bitmap_op(DO_FV_BPOS,1);//���ˮԴ								
				}
				l_sys.Fan_Close |= FC_WS;
				l_sys.Comp_Close |= FC_WS;
				
		}
		else
		{
				l_sys.Fan_Close &= ~FC_WS;
				l_sys.Comp_Close &= ~FC_WS;	
				req_bitmap_op(DO_FV_BPOS,0);
		}		
		
		return;
}

//�Ʊ�ˮ
void Cold_Water_exe(void)
{
		extern sys_reg_st		g_sys;	
		extern local_reg_st l_sys;
		
		int16_t i16Water_Temp;
	
		i16Water_Temp =(int16_t)g_sys.status.ComSta.u16Ain[AI_NTC4];
		
		l_sys.Cold_Water=FALSE;
		if(g_sys.config.ComPara.u16ColdWater_Mode)//�Ʊ�ˮģʽ	
		{
			if((sys_get_di_sts(DI_DRINK_DOWN_BPOS))&&(sys_get_di_sts(DI_DRINK_MD_BPOS)))//��������ˮλ
			{
					if((i16Water_Temp!=ABNORMAL_VALUE)&&(i16Water_Temp>g_sys.config.ComPara.u16ColdWater_StartTemp))//��ʼ�Ʊ�ˮ
					{
						l_sys.Cold_Water=TRUE;
						req_bitmap_op(DO_WV_BPOS,1);//����
						req_bitmap_op(DO_CV_BPOS,1);//�Ʊ�ˮ							
					}
					else
					if((i16Water_Temp!=ABNORMAL_VALUE)&&(i16Water_Temp<g_sys.config.ComPara.u16ColdWater_StopTemp))//�ر� 
					{
						req_bitmap_op(DO_WV_BPOS,0);//����
						req_bitmap_op(DO_CV_BPOS,0);//�Ʊ�ˮ							
					}
					else
					{
						
					}
			}
			else
			{
				req_bitmap_op(DO_WV_BPOS,0);//����ر� 
				req_bitmap_op(DO_CV_BPOS,0);//�Ʊ�ˮ�ر� 		
			}	
		}
		else
		{
				req_bitmap_op(DO_WV_BPOS,0);//����
				req_bitmap_op(DO_CV_BPOS,0);//�Ʊ�ˮ	
		}		
		return;
}


//��������ִ���߼�
void req_execution(int16_t target_req_temp,int16_t target_req_hum)
{	
		//���,ѹ����������
		Sys_Fan_CP_WL();
    //�������
		fan_req_exe(target_req_temp,target_req_hum);	
    //ѹ��������
		compressor_req_exe(target_req_temp,target_req_hum);
		//������
		Pwp_req_exe();
		//��ˮ
		WaterOut_req_exe();
		//ɱ��
		Sterilize_req_exe();
		//��˪
		Defrost_req_exe();
		//���ˮԴ
		External_Water_exe();
		//�Ʊ�ˮ
		Cold_Water_exe();
	
		return;
}
