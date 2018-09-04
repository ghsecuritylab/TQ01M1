#ifndef __REQ_EXE_H__
#define __REQ_EXE_H__
#include "stdint.h"

#define HUM_CURRENT_UNIT                  1.19
enum
{
		HUM_FSM_STATE_IDLE = 0,
		HUM_FSM_STATE_CHECK,
		HUM_FSM_STATE_WARM,
		HUM_FSM_STATE_DRAIN,//��ˮ
		HUM_FSM_STATE_HUM,//��ʪ
		HUM_FSM_STATE_FILL,//עˮ
		HUM_FSM_STATE_FLUSH,//��ˢ��ˮ
	
};

enum
{
		HUM_TYPE_FIX,
		HUM_TYPE_P,
		HUM_TYPE_INFRARED,//�����ʪ
};







enum
{
		COMPRESSOR_FSM_STATE_IDLE=0,
		COMPRESSOR_FSM_STATE_INIT,
		COMPRESSOR_FSM_STATE_STARTUP,
		COMPRESSOR_FSM_STATE_NORMAL,
		COMPRESSOR_FSM_STATE_SHUTING,
		COMPRESSOR_FSM_STATE_STOP,
};

//���״̬��
enum
{
		FSM_FAN_IDLE=0,
		FSM_FAN_INIT,
		FSM_FAN_START_UP,
		FSM_FAN_NORM,
		FSM_FAN_SHUT
};
//��ˮģʽ
enum
{
    WATER_NO = 0x00,
    WATER_NORMAL_ICE = 0x01,
    WATER_HEAT = 0x02,
    WATER_NORMAL_ICE_2 = 0x04,
};

//ˮ·���Ʒ���
enum
{
    HEART_POT = 0x01,//�ȹ�
    HMI_KEY = 0x02,
};
#define ChildKey_Cnt  3
#define ChildKey_Lose 5


//ˮλ
enum
{
    S_L = 0x01,
    S_M = 0x02,
    S_U = 0x04,
    D_L = 0x08,
    D_M = 0x10,
    D_U = 0x20,
};

//��������
enum
{
    L300  = 570,
    L500  = 1250,
    L1000 = 2750,
    L1500 = 4350,
    L2000 = 5450,
};
//��������
#define L300_FACTOR   0.47846889952153110047846889952153
//#define L300_FACTOR   0.52631578947368421052631578947368
#define L500_FACTOR   0.4
#define L1000_FACTOR  0.36363636363636363636363636363636
#define L1500_FACTOR  0.34482758620689655172413793103448
#define L2000_FACTOR  0.36697247706422018348623853211009

//����������,500ml/MIN,60s*HEAT_FACTOR
#define HEAT_FACTOR_S   8.3333333333333333333333333333333
#define HEAT_FACTOR_500MS   4.1666666666666666666666666666667

//��ˮ״̬
enum
{
    HEATER_IDLE = 0,
    HEATER_SEND,
    WATER_READ,
    WATER_OUT,
};
//����������
enum
{
    CLOSE_HEAT = 0,
    OPEN_HEAT,
};

//��������ˮ״̬
enum
{
    HEAT_NO = 0,
    HEAT_OUTWATER = 0x01,
};

#define  RH_DEALY  10
#define  WRITEHEAT_MAX  250
#define  CLOSEHEAT_MAX  3

//ɱ��
#define STERILIZE_BIT0   0x01
#define STERILIZE_BIT1   0x02
//���γ�ˮʱ������
#define WATER_MAXTIME   600*2  //10����

//��ʱ����ʱ��
#define FIXED_SAVETIME   900
enum
{
		FAN_MODE_FIX=0,//����ģʽ
		FAN_MODE_PRESS_DIFF,//ѹ��ģʽ
		FAN_MODE_AVR_RETURN,//�ط�ƽ��
		FAN_MODE_AVR_SUPPLY,//�ͷ�ƽ��
  	FAN_MODE_TEMP_DIFF,//�²�ƽ��
		FAN_MODE_MAX_RETURN,//�ط��ȵ�
		FAN_MODE_MAX_SUPPLY,//�ͷ��ȵ�
		FAN_MODE_TEMP_MAX_DIFF,//�²��ȵ�
    FAM_MODE_INV_COMP,//��Ƶ����
};
void hum_capacity_calc(void);
void req_execution(int16_t target_req_temp,int16_t target_req_hum);
void req_bitmap_op(uint8_t component_bpos, uint8_t action);
#endif //__REQ_EXE_H__

