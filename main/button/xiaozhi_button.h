#ifndef __XIAOZHI_BUTTON_H__
#define __XIAOZHI_BUTTON_H__

#include "button_adc.h"
#include "iot_button.h"
#include "esp_log.h"

//1、创建按键
void xiaozhi_button_int(void);
//2、按键2注册事件
//参数解释：1、按键句柄 2、按键绑定事件 3、按键对应：长按时间、多次按下的次数 4、触发事件回调函数 5、用户自定义数据
void xiaozhi_button2_regist(button_event_t event,button_event_args_t *event_args,button_cb_t cb,void *usr_data);
//3、按键3注册事件
void xiaozhi_button3_regist(button_event_t event,button_event_args_t *event_args,button_cb_t cb,void *usr_data);
#endif /* __XIAOZHI_BUTTON_H__ */
