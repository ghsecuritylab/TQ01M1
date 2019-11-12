/**
 ****************************************************************************
 * @Warning Without permission from the author,Not for commercial use
 * @File    ledkey_opt.c
 * @Author  xiaowine@cee0.com
 * @date    
 * @version V1.0
 *************************************************
 * @brief   ��עϵͳ��Ϣ
 ****************************************************************************
 * @attention 
 * Powered By Xiaowine
 * <h2><center>&copy;  Copyright(C) cee0.com 2015-2019</center></h2>
 * All rights reserved
 * 
**/

#include "ledkey_opt.h"
#include "local_status.h"
#define SAMPLE_UART_NAME "uart4"
/* ���ڽ�����Ϣ�ṹ*/
struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
};
/* �����豸��� */
static rt_device_t serial;
/* ��Ϣ���п��ƿ� */
static struct rt_messagequeue rx_mq;

unsigned char recOK = 0;
static unsigned char tx_buffer[10] = {0xa7, 0xf3, 0xaa, 0x04, 0x06};
static unsigned char rx_buffer[10] = {0xa7, 0xf3, 0xaa, 0x04, 0x06};

/**********************key led*********************************************************/
_TKS_FLAGA_type keyState[2];
volatile _TKS_FLAGA_type keyTrg[2];

#define KEY1 keyTrg[0].bits.b2
#define KEY2 keyTrg[0].bits.b3
#define KEY3 keyTrg[0].bits.b1
#define KEY4 keyTrg[0].bits.b0

#define KEY1Restain keyTrg[1].bits.b2
#define KEY2Restain keyTrg[1].bits.b3
#define KEY3Restain keyTrg[1].bits.b1
#define KEY4Restain keyTrg[1].bits.b0

_USR_FLAGA_type ledState[5];
#define led1State ledState[0].s4bits.s0
#define led2State ledState[0].s4bits.s1
#define led3State ledState[1].s4bits.s0
#define led4State ledState[1].s4bits.s1
#define led5State ledState[2].s4bits.s0
#define led6State ledState[2].s4bits.s1
#define led7State ledState[3].s4bits.s0
#define led8State ledState[3].s4bits.s1
#define led9State ledState[4].s4bits.s0

unsigned char beepCount = 0;

/*************************function******************************************************/

/* �������ݻص����� */
static rt_err_t
uart_input(rt_device_t dev, rt_size_t size)
{
    struct rx_msg msg;
    rt_err_t result;
    msg.dev = dev;
    msg.size = size;

    result = rt_mq_send(&rx_mq, &msg, sizeof(msg));
    if (result == -RT_EFULL)
    {
        /* ��Ϣ������ */
        rt_kprintf("message queue full��\n");
    }
    return result;
}
/***
 * 
 * 
 * 
 **/

static void receiveData(unsigned char data)
{
    static unsigned char count = 0;
    static unsigned char checkSum = 0;
    if ((count == 0) && (data == 0xA7))
    {
        rx_buffer[count] = data;
        checkSum = data;
        count++;
    }
    else if ((count == 1) && (data == 0xF2))
    {
        checkSum += data;
        rx_buffer[count] = data;
        count++;
    }
    else if (count == 2)
    {
        checkSum += data;
        rx_buffer[count] = data;
        count++;
    }
    else if (count == 3)
    {
        checkSum += data;
        rx_buffer[count] = data;
        count++;
    }
    else if ((count == 4) && (checkSum == data))
    {
        keyState[0].byte = rx_buffer[2];
        keyState[1].byte = rx_buffer[3];
        count = 0;
        recOK = 1;
    }
    else
    {
        count = 0;
    }
}
/****
 * 
 * 
 * 
 * 
**/
#include "req_execution.h"
#include "local_status.h"
#include "global_var.h"
extern local_reg_st l_sys;
extern sys_reg_st g_sys;
void keyRecOperation(_TKS_FLAGA_type *keyState)
{

    static rt_uint8_t k_count[2] = {0};
    keyTrg[0].byte = keyState->byte & (keyState->byte ^ k_count[0]);
    k_count[0] = keyState->byte;
    keyTrg[1].byte = (keyState + 1)->byte & ((keyState + 1)->byte ^ k_count[1]);
    k_count[1] = (keyState + 1)->byte;
    if (KEY1)
    {
        rt_kprintf("key1\n");
    }
    if (KEY2)
    {
        l_sys.OutWater_Key ^= WATER_NORMAL_ICE;
    }
    if (KEY3)
    {
        if (g_sys.config.ComPara.u16ColdWater_Mode)
        {
            g_sys.config.ComPara.u16ColdWater_Mode = ICE_NO;
        }
        else
        {
            g_sys.config.ComPara.u16ColdWater_Mode = NORMAL_ICE;
        }
        RAM_Write_Reg(86, g_sys.config.ComPara.u16ColdWater_Mode, 1);
        rt_kprintf("key3\n");
    }
    if (KEY4)
    {
        rt_kprintf("key4\n");
    }

    if (KEY1Restain)
    {
        if (g_sys.config.ComPara.u16ExitWater_Mode == WATER_EXIT)
        {
            led5State = STATE_LED_OFF;
            g_sys.config.ComPara.u16ExitWater_Mode = WATER_AIR;
        }
        else
        {
            led5State = STATE_LED_ON;
            g_sys.config.ComPara.u16ExitWater_Mode = WATER_EXIT;
        }
        RAM_Write_Reg(EE_EXITWATER, g_sys.config.ComPara.u16ExitWater_Mode, 1);
        rt_kprintf("KEY1Restain\n");
    }
    if (KEY2Restain)
    {
        rt_kprintf("KEY2Restain\n");
    }
    if (KEY3Restain)
    {
        rt_kprintf("KEY3Restain\n");
    }

    if (KEY4Restain)
    {
        g_sys.config.ComPara.u16Power_Mode ^= 1;

        write_reg_map(POWER_ON_ADDR, g_sys.config.ComPara.u16Power_Mode);
        rt_kprintf("KEY4Restain\n");
    }
}
/**
 * 
 * 
 * 
**/
#include "sys_status.h"
void ledSendOperation(void)
{
    if (l_sys.makeWater && sys_get_do_sts(DO_COMP1_BPOS) && (l_sys.Cold_Water != TRUE))
    {
        led1State = STATE_LED_FLASH_1HZ;
    }
    else if (l_sys.makeWater)
    {
        led1State = STATE_LED_ON;
    }
    else
    {
        led1State = STATE_LED_OFF;
    }

    if (l_sys.Pwp_Open || l_sys.Purification)
    {
        led2State = STATE_LED_FLASH_1HZ;
    }
    else
    {
        led2State = STATE_LED_OFF;
    }

    if (WaterOut_level())
    {
        led3State = STATE_LED_OFF;
    }
    else
    {
        led3State = STATE_LED_FLASH_1HZ;
    }

    if (sys_get_do_sts(DO_UV1_BPOS))
    {
        led4State = STATE_LED_FLASH_1HZ;
    }
    else
    {
        led4State = STATE_LED_OFF;
    }

    if (g_sys.config.ComPara.u16ExitWater_Mode == WATER_EXIT)
    {
        led5State = STATE_LED_ON;
    }
    else
    {
        led5State = STATE_LED_OFF;
    }

    if (l_sys.OutWater_OK == WATER_READ)
    {
        led6State = STATE_LED_FLASH_2HZ;
    }
    else
    {
        led6State = STATE_LED_ON;
    }

    if (g_sys.config.ComPara.u16ColdWater_Mode == ICE_NO)
    {
        led7State = STATE_LED_OFF;
    }
    else
    {
        static int16_t ColdWaterStateBak = 0;
        if (l_sys.ColdWaterState == 1)
        {
            led7State = STATE_LED_FLASH_0_5HZ;
        }
        else if (l_sys.ColdWaterState == 2)
        {
            led7State = STATE_LED_FLASH_2HZ;
        }
        else if ((l_sys.ColdWaterState == 3) || (l_sys.ColdWaterState == 5))
        {
            led7State = STATE_LED_ON;
        }
        else if (l_sys.ColdWaterState == 4)
        {
            led7State = STATE_LED_ON;
            if (ColdWaterStateBak == 3)
            {
                beepCount += 3;
            }
        }
        ColdWaterStateBak = l_sys.ColdWaterState;
    }

    led8State = STATE_LED_OFF;

    if (g_sys.config.ComPara.u16Power_Mode)
    {
        led9State = STATE_LED_ON;
    }
    else
    {
        led9State = STATE_LED_OFF;
    }

    tx_buffer[0] = 0xa7;
    tx_buffer[1] = 0xf6;
    tx_buffer[2] = ledState[0].byte;
    tx_buffer[3] = ledState[1].byte;
    tx_buffer[4] = ledState[2].byte;
    tx_buffer[5] = ledState[3].byte;
    tx_buffer[6] = ledState[4].byte;
    tx_buffer[7] = beepCount & 0x0f;
    beepCount = 0;

    tx_buffer[8] = tx_buffer[0];
    tx_buffer[8] += tx_buffer[1];
    tx_buffer[8] += tx_buffer[2];
    tx_buffer[8] += tx_buffer[3];
    tx_buffer[8] += tx_buffer[4];
    tx_buffer[8] += tx_buffer[5];
    tx_buffer[8] += tx_buffer[6];
    tx_buffer[8] += tx_buffer[7];
}
/***
 * 
 * 
 * 
 */
static void serial_thread_entry(void *parameter)
{
    struct rx_msg msg;
    rt_err_t result;
    rt_uint32_t rx_length;
    static unsigned char rx_buffer[RT_SERIAL_RB_BUFSZ + 1];

    rt_kprintf("**************************************start ledkey thread**************************************\r\n");

    while (1)
    {
        rt_memset(&msg, 0, sizeof(msg));
        /* ����Ϣ�����ж�ȡ��Ϣ*/
        result = rt_mq_recv(&rx_mq, &msg, sizeof(msg), 1000);
        if (result == RT_EOK)
        {
            /* �Ӵ��ڶ�ȡ����*/
            rx_length = rt_device_read(msg.dev, 0, rx_buffer, msg.size);
            for (char i = 0; i < rx_length; i++)
            {
                receiveData(rx_buffer[i]);
            }
            if (recOK)
            {
                keyRecOperation(keyState);
                ledSendOperation();
                rt_device_write(serial, 0, tx_buffer, 9);
                recOK = 0;
            }

            /* ��ӡ���� */
            rt_memset(rx_buffer, 0, sizeof(rx_buffer));
        }
    }
}

int ledKeyStart(void)
{
    rt_err_t ret = RT_EOK;
    char uart_name[RT_NAME_MAX];
    static char msg_pool[256];
    char str[] = "hello RT-Thread!\r\n";

    rt_strncpy(uart_name, SAMPLE_UART_NAME, RT_NAME_MAX);

    /* ���Ҵ����豸 */
    serial = rt_device_find(uart_name);
    if (!serial)
    {
        rt_kprintf("find %s failed!\n", uart_name);
        return RT_ERROR;
    }

    /* ��ʼ����Ϣ���� */
    rt_mq_init(&rx_mq, "rx_mq",
               msg_pool,              /* �����Ϣ�Ļ����� */
               sizeof(struct rx_msg), /* һ����Ϣ����󳤶� */
               sizeof(msg_pool),      /* �����Ϣ�Ļ�������С */
               RT_IPC_FLAG_FIFO);     /* ����ж���̵߳ȴ������������ȵõ��ķ���������Ϣ */

    /* �� DMA ���ռ���ѯ���ͷ�ʽ�򿪴����豸 */
    rt_device_open(serial, RT_DEVICE_FLAG_DMA_RX);
    /* ���ý��ջص����� */
    rt_device_set_rx_indicate(serial, uart_input);
    /* �����ַ��� */
    rt_device_write(serial, 0, str, (sizeof(str) - 1));

    /* ���� serial �߳� */
    rt_thread_t thread = rt_thread_create("serial", serial_thread_entry, RT_NULL, 1024, 25, 10);
    /* �����ɹ��������߳� */
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        rt_kprintf("serial thread_startup failed!\n");
        ret = RT_ERROR;
    }

    return ret;
}
