#ifndef	USER_APP
#define USER_APP
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mb_m.h"
#include "mbconfig.h"
#include "mbframe.h"
#include "mbutils.h"
#include "sys_conf.h"

#define MAX_DISCRETE_REG_CNT 10

/* -----------------------Slave Defines -------------------------------------*/
#define 				 S_REG_HOLDING_START           0
#define 				 S_REG_HOLDING_NREGS           110
#define 				 REG_HOLDING_WRITE_NREGS       5//��д��Χ+
//�ӻ�ģʽ���ڱ��ּĴ����У�������ַ��Ӧ�Ĺ��ܶ���
#define          S_HD_RESERVE                     0		  //����
#define          S_HD_CPU_USAGE_MAJOR             1         //��ǰCPU�����ʵ�����λ
#define          S_HD_CPU_USAGE_MINOR             2         //��ǰCPU�����ʵ�С��λ

//�ӻ�ģʽ��������Ĵ����У�������ַ��Ӧ�Ĺ��ܶ���
#define          S_IN_RESERVE                     0		  //����


/* -----------------------Master Defines -------------------------------------*/
#define 				 M_REG_HOLDING_START           0
#define 				 M_REG_HOLDING_NREGS           33
#define 				 M_REG_HOLDING_USR_START       20

//����ģʽ���ڱ��ּĴ����У�������ַ��Ӧ�Ĺ��ܶ���
#define          M_HD_RESERVE                     0		  //����

//����ģʽ��������Ĵ����У�������ַ��Ӧ�Ĺ��ܶ���
#define          M_IN_RESERVE                     0		  //����

//����ģʽ������Ȧ�У�������ַ��Ӧ�Ĺ��ܶ���
#define          M_CO_RESERVE                     2		  //����

//����ģʽ������ɢ�����У�������ַ��Ӧ�Ĺ��ܶ���
#define          M_DI_RESERVE                     1		  //����

enum
{
    MBM_DEV_A1_ADDR	=0,					//return air temp&hum board 1 modbus address
    MBM_DEV_A2_ADDR	, 					//supply air temp&hum board 1 modbus address
    MBM_DEV_A3_ADDR	,					  //return air temp&hum board 2 modbus address
    MBM_DEV_A4_ADDR	, 					//supply air temp&hum board 2 modbus address
    MBM_DEV_A5_ADDR	,					  //return air temp&hum board 3 modbus address
    MBM_DEV_A6_ADDR	,           //supply air temp&hum board 3 modbus address
    MBM_DEV_A7_ADDR	,					  //return air temp&hum board 4 modbus address
    MBM_DEV_A8_ADDR	,					  //supply air temp&hum board 4 modbus address
//    MBM_DEV_IPM_ADDR,					  //IPM board modbus address
//    MBM_DEV_UPS_ADDR	, 				//UPS modbus address
//    MBM_DEV_PDU0_ADDR,					  //PDU board modbus address
//    MBM_DEV_PDU1_ADDR,					  //PDU board modbus address
//    MBM_DEV_PDU2_ADDR,					  //PDU board modbus address
//    MBM_DEV_PDU3_ADDR,					  //PDU board modbus address
    MB_MASTER_TOTAL_SLAVE_NUM ,		//Total
};
#define MB_MASTER_DISCRETE_OFFSET    MBM_DEV_IPM_ADDR  //8

//����ʾ��ʪ�ȴ�����    XD400D-HT3010-485-D
#define MB_DEV_DISPLAY_TH1_ADDR    MBM_DEV_A5_ADDR  //5
#define MB_DEV_DISPLAY_TH2_ADDR    MBM_DEV_A6_ADDR  //6
#define MB_DEV_DISPLAY_TH3_ADDR    MBM_DEV_A7_ADDR  //7
#define MB_DEV_DISPLAY_TH4_ADDR    MBM_DEV_A8_ADDR  //8

//�ͷ��ַ
#define MB_DEV_TH_SUPPLY_START     MBM_DEV_A1_ADDR  //1
#define MB_DEV_TH_SUPPLY_END       MBM_DEV_A1_ADDR  //1

#define MB_DEV_TH_RETURN_START     MBM_DEV_A2_ADDR  //2
#define MB_DEV_TH_RETURN_END       MBM_DEV_A2_ADDR  //2

#define 				 MBM_REG_ADDR_CNT_ADDR						0
#define 				 MBM_REG_DEV_TPYE_ADDR						1
#define 				 MBM_REG_SOFT_VER_ADDR						3
#define 				 MBM_REG_HARD_VER_ADDR						4
#define 				 MBM_REG_SERIAL_ADDR							5
#define 				 MBM_REG_MAN_DATE_ADDR						9
#define 				 MBM_REG_COMM_AD_ADDR							11
#define 				 MBM_REG_BAUDRATE_ADDR						12
#define 				 MBM_REG_STATUS_ADDR							18
#define 				 MBM_REG_CONF_ADDR								19

//humidifier control board reg definition
#define					 MBM_DEV_H_REG_STATUS_ADDR				20
#define					 MBM_DEV_H_REG_CONDUCT_ADDR				21
#define					 MBM_DEV_H_REG_HUMCUR_ADDR				22
#define					 MBM_DEV_H_REG_WT_LV_ADDR					23

//hum&temp control board reg definition
#define          MBM_DEV_H_REG_HT_STATUS_ADDR     20
#define					 MBM_DEV_H_REG_TEMP_ADDR					21
#define					 MBM_DEV_H_REG_HUM_ADDR						22
//power control board reg definition
#define          MBM_DEV_H_REG_P0WER_STATUS_ADDR  20
#define					 MBM_DEV_H_REG_PA_VOLT_ADDR				21
#define					 MBM_DEV_H_REG_PB_VOLT_ADDR				22
#define					 MBM_DEV_H_REG_PC_VOLT_ADDR				23
#define					 MBM_DEV_H_REG_FREQ_ADDR					24
#define					 MBM_DEV_H_REG_PE_ADDR						25
//outside control bord reg definition
#define          MBM_DEV_H_REG_OSC_STATUS_ADDR		    20
#define          MBM_DEV_H_REG_OSC_DI_BIT_MAP_ADDR		21
#define          MBM_DEV_H_REG_OSC_PRESSOR_ADDR		    22
#define          MBM_DEV_H_REG_OSC_TEMP_ADDR		      23
#define          MBM_DEV_H_REG_OSC_FAN_SPD_ADDR		    24
#define          MBM_DEV_H_REG_OSC_DO_ADDR		        25
#define          MBM_DEV_H_REG_OSC_CMP_SPD_ADDR		    26



//modbus master FSM definition
#define					MBM_FSM_IDLE			0x01
#define					MBM_FSM_SYNC			0x02
#define					MBM_FSM_UPDATE		0x04
#define					MBM_FSM_SEND      0x05


//
#define					MBM_POLL_TIMEOUT_THRESHOLD 				5
#define					MBM_INIT_TIMEOUT_THRESHOLD 				5
#define					MBM_UPDATE_TIMEOUT_THRESHOLD 			5

typedef struct
{
		uint16_t poll;
		uint16_t init;
		uint16_t update;
	  uint16_t discrete;
}mbm_bitmap_st;

typedef struct
{
		uint16_t poll;
		uint16_t init;
		uint16_t update;
}mbm_timeout_st;

typedef struct
{
		uint16_t poll;
		uint16_t init;
		uint16_t update;
}mbm_errcnt_st;

typedef struct
{
		uint8_t  mbm_addr;
		uint16_t reg_cnt;
		uint16_t reg_addr[MAX_DISCRETE_REG_CNT];	
}mbm_read_st;

typedef struct
{
		mbm_bitmap_st bitmap;
		mbm_errcnt_st errcnt;
		mbm_timeout_st timeout;
		uint16_t mbm_fsm;
}mbm_dev_st;

typedef struct
{
		uint8_t  mbm_addr;
		uint16_t reg_addr;
		uint16_t reg_value;	
		uint8_t  mbm_NRegs;
		uint8_t  mbm_WriteType;
}mbm_data_st;

typedef enum
{
		MB_WRITE_SINGLE, //д�����Ĵ���                  
		MB_WRITE_MULITE, //д����Ĵ���               
}eMBWriteMode;


void mbm_sts_update(sys_reg_st* gds_ptr);
void mbs_sts_update(void);// ���±��ر�����Э��ջ�Ĵ�����
#endif
