/**
 ****************************************************************************
 * @Warning :Without permission from the author,Not for commercial use
 * @File    :
 * @Author  :Seblee
 * @date    :2018-09-11 11:11:19
 * @version :V1.0.0
 *************************************************
 * @brief   :
 ****************************************************************************
 * @Last Modified by: Seblee
 * @Last Modified time: 2018-09-11 16:14:43
 ****************************************************************************
**/
#ifndef __MQTT_CLIENT_H_
#define __MQTT_CLIENT_H_
/* Private include -----------------------------------------------------------*/
#include "sim7600.h"
#include "MQTTPacket.h"
#include "sys_conf.h"
#include <rtthread.h>
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define DEVICE_ID "cdtest0041"

// #define DEVICE_NAME "HelloWorld"
// #define PRODUCT_KEY "a1YfNlQgDq0"
// #define DEVICE_SECRET "afbL7kf8MvUBFQN2"

// #define DEVICE_NAME "HelloWorld"
// #define PRODUCT_KEY "rl0bGtKFCYA"
// #define DEVICE_SECRET "gfp06h1QZxZXefWEEYweaMnsLxJU3lvp"
// #define DEVICENAME "cdtest004"
// #define PRODUCTKEY "a1JOOi3mNEf"
// #define SECRET  "WjzDAlsux7gBMfF31M9CSZ9LKmutISPe"
/***highLevel***/
#define DEVICE_NAME "device11111"
#define PRODUCT_KEY "a1dZFsDxBvp"
#define DEVICE_SECRET "o5vZRqFH7csXFMNO"
/******topics**********/  
#define TOPIC_SUB_INIT "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/sub/register_reply" /*{"TQ_PLATFORM_INIT"}*/

#define TOPIC_PLATFORM_INIT "/" PRODUCT_KEY "/" DEVICE_NAME "/pay/client2Cloud/init"                /*{"TQ_PLATFORM_INIT"}*/
#define TOPIC_WATER_NOTICE "/" PRODUCT_KEY "/" DEVICE_NAME "/pay/service2Cloud/get"                 /*{"TQ_WATER_NOTICE"}*/
#define TOPIC_WATER_STATUS "/" PRODUCT_KEY "/" DEVICE_NAME "/pay/client2Cloud/getWaterStatus"       /*{"TQ_WATER_STATUS"}*/
#define TOPIC_PARAMETER_SETUP "/" PRODUCT_KEY "/" DEVICE_NAME "/monitor/service2Cloud/setting"      /*{"TQ_PARAMETER_SETUP"}*/
#define TOPIC_PARAMETER_GET "/" PRODUCT_KEY "/" DEVICE_NAME "/monitor/service2Cloud/getSetting"     /*{"TQ_PARAMETER_GET"}*/
#define TOPIC_PARAMETER_REPORT "/" PRODUCT_KEY "/" DEVICE_NAME "/monitor/client2Cloud/putSetting"   /*{"TQ_PARAMETER_REPORT"}*/
#define TOPIC_REALTIME_REPORT "/" PRODUCT_KEY "/" DEVICE_NAME "/monitor/client2Cloud/putRealStatus" /*{"TQ_REALTIME_REPORT"}*/
#define TOPIC_HEART_BEAT "/" PRODUCT_KEY "/" DEVICE_NAME "/monitor/client2Cloud/putTimingStatus"    /*{"TQ_HEART_BEAT"}*/
#define TOPIC_DEVICE_UPGRADE "/" PRODUCT_KEY "/" DEVICE_NAME "/monitor/service2Cloud/upgrade"       /*{"TQ_DEVICE_UPGRADE"}*/
#define TOPIC_DEVICE_MOVE "/" PRODUCT_KEY "/" DEVICE_NAME "/move"                                   /*{"DEVICE_MOVE"}*/
#define TOPIC_DEVICE_UPDATE "/" PRODUCT_KEY "/" DEVICE_NAME "/update"                               /*{"DEVICE_UPDATE"}*/
#define TOPIC_DEVICE_ERR "/" PRODUCT_KEY "/" DEVICE_NAME "/update/error"                            /*{"DEVICE_ERR"}*/
#define TOPIC_DEVICE_GET "/" PRODUCT_KEY "/" DEVICE_NAME "/get"                                     /*{"DEVICE_GET"}*/
    /* Private macro -------------------------------------------------------------*/

    /* Private variables ---------------------------------------------------------*/

    /* Private function prototypes -----------------------------------------------*/

    /* Private functions ---------------------------------------------------------*/
    int
    mqtt_client_init(rt_device_t dev);
void mqtt_setup_connect_info(iotx_conn_info_t *conn, iotx_device_info_t *device_info);
int mqtt_client_connect(rt_device_t dev, MQTTPacket_connectData *conn);
rt_err_t mqtt_client_subscribe(const char *subsc, iotx_device_info_pt iotx_dev_info);
rt_err_t mqtt_client_subscribe_topics(void);
rt_err_t mqtt_packet_read_operation(void);
unsigned short mqtt_client_packet_id(void);
rt_err_t mqtt_client_ping(void);

/*----------------------------------------------------------------------------*/

#endif
