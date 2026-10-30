#ifndef __XIAOZHI_BUTTON_H__
#define __XIAOZHI_BUTTON_H__
#include "button_adc.h"
#include "iot_button.h"
#include "esp_log.h"

// 1.ADC按键的初始化方法
void xiaozhi_button_init(void);

// 2.按键2注册事件
void xiaozhi_button2_regist(button_event_t event, button_event_args_t *event_args, button_cb_t cb, void *usr_data);


//3.按键3注册事件
void xiaozhi_button3_regist(button_event_t event, button_event_args_t *event_args, button_cb_t cb, void *usr_data);
#endif /* __XIAOZHI_BUTTON_H__ */