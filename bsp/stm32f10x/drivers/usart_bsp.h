/*
 * File      : usart.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

#ifndef __USART_H__
#define __USART_H__

#include <rthw.h>
#include <rtthread.h>
#include <stdint.h>

#define HEATWRITE_NUM  18
#define HEATREAD_NUM   18
//��д����
enum
{
		HEAT_WRITEPARA=0,
		HEAT_READPARA,
};
enum
{
		HEAT_CLOSE=0,
		HEAT_OPEN,
};

enum
{
		UART_HEAT=0,
		UART_PM25,
		UART_NUM,
};
#define PROTOCOL_FRAME_MaxLen  20
typedef enum
{
    PROTOCOL_STACK_IDLE=0,              /*!< Receiver is in idle state. */
    PROTOCOL_STACK_CK,               /*!< Frame is beeing received. */
    PROTOCOL_STACK_RCV,               /*!< Frame is beeing received. */
    PROTOCOL_STACK_CS,               /*!< Frame is beeing received. */
    PROTOCOL_STACK_END,              /*!< If the frame is invalid. */
}UartRcvState;

	typedef enum  
	{
	    RECV_Wait,          //���յȴ�
	    RECV_Going,         //���ս���
	    RECV_Over,          //�������
	    SEND_Wait,          //���͵ȴ�
	    SEND_Going,         //���ͽ���
	    SEND_Over,          //�������
	    INIT_Apply          //��ʼ��
	}PROTOCOL_STATUS;

typedef struct
{
	uint8_t CommID;		  // ���ں�
	uint8_t Baudrate;      // ͨ��ͨѶ�Ĳ�����
	uint8_t StatckStatus;  // ָʾ��ǰЭ��ջ����״̬(����->��ַ����->)
	uint8_t StatckType;    // ��ַ����(˽�е�ַ[bit0],ȫAA�򲿷�AAΪ���ַ[bit1],ȫ99Ϊ�㲥��ַ[bit2],[bit3...bit6] δʹ��,����Ϊ0,֡���ճɹ�[bit7])
	uint8_t DataCount;     // ���յ�������֡����()
}ProtocolLayer;

#define PROTOCOL_FRAME_ByteGap     	250
#define PROTOCOL_FRAME_SendGap     	10

extern ProtocolLayer Protocol[UART_NUM];
extern uint8_t g_ComBuff[UART_NUM][PROTOCOL_FRAME_MaxLen];
extern PROTOCOL_STATUS   g_ComStat[UART_NUM];                 //ͨѶͨ������״̬

void drv_usart_init(void);
extern void xPort_Usart_Init(uint8_t ucMB_Number);
extern uint8_t Heat_Send(uint8_t u8Type,uint8_t u8OC,uint8_t u8Temp,uint16_t u16WL);
extern void Comm_Service(void);
#endif
