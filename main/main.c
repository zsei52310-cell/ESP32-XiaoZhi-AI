#include <stdio.h>
#include "xiaozhi_button.h"
#include "xiaozhi_wifi_sta.h"
#include "xiaozhi_lvgl.h"
#include "xiaozhi_audio.h"
#include "xiaozhi_http.h"
#include "xiaozhi_sr.h"
static char *TAG = "MAIN";
// 按键事件触发回调
void adc_button_callBack(void *button_handle, void *usr_data);

void app_main(void)
{
    // 1.初始化ADC按键
    xiaozhi_button_init();
    // 2.按键2,3注册回调
    xiaozhi_button2_regist(BUTTON_SINGLE_CLICK, NULL, adc_button_callBack, (void *)1);
    xiaozhi_button2_regist(BUTTON_DOUBLE_CLICK, NULL, adc_button_callBack, (void *)2);
    xiaozhi_button3_regist(BUTTON_SINGLE_CLICK, NULL, adc_button_callBack, (void *)3);

    // 3.初始化LVGL
    xiaozhi_lvgl_init();

    // 4.屏幕布局
    xiaozhi_lvgl_layout();

    // 3.WIFI驱动_STA模式连接豪哥热点
    xiaozhi_wifi_sta_init();

    // 4.HTTP客户端初始化+发送一个HTTP的POST请求(务必等设备配网成功以后有了wifi功能在发送)
    xiaozhi_http_client_init();

    // 5.初始化语音识别
    xiaozhi_sr_init();
}
void adc_button_callBack(void *button_handle, void *usr_data)
{
    uint8_t btn_tag = (uint8_t)usr_data;
    switch (btn_tag)
    {
    case 1:
        // 重置WIFI信息
        xiaozhi_wifi_sta_reset();
        break;
    case 2:
        ESP_LOGE(TAG, "button2 double click");
        break;
    case 3:
        ESP_LOGE(TAG, "button3 single click");
        break;

    default:
        break;
    }
}