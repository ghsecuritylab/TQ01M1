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

unsigned char keyState[3] = {0};
unsigned char recOK = 0;

/* �������ݻص����� */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
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
static void receiveData(unsigned char data)
{
    static unsigned char count = 0;
    if ((count == 0) && (data == 0xa5))
    {
        count++;
    }
    else if ((count == 1) && (data == 0x52))
    {
        count++;
    }
    else if (count == 2)
    {
        keyState[0] = data;
        count++;
    }
    else if (count == 3)
    {
        keyState[1] = data;
        count = 0;
        recOK = 1;
    }
    else
    {
        count = 0;
    }
}

static void serial_thread_entry(void *parameter)
{
    struct rx_msg msg;
    rt_err_t result;
    rt_uint32_t rx_length;
    static unsigned char rx_buffer[RT_SERIAL_RB_BUFSZ + 1];
    static unsigned char tx_buffer[10] = {0xa5, 0x53, 0x03, 0x04, 0x06};

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
                rt_device_write(serial, 0, tx_buffer, 5);
                recOK = 0;
                rt_kprintf("keyState:%02x,%02x\n", keyState[0], keyState[1]);
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
