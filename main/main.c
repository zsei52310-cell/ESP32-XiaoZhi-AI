#include <stdio.h>
#include "esp_log.h"
#include "xiaozhi_button.h"
#include "xiaozhi_wifi_sta.h"
#include "xiaozhi_lcd.h"
#include "xiaozhi_lvgl.h"
static char *TAG = "MAIN";
// 按键触发回调
void adc_button_callback(void *button_handle, void *usr_data);

void app_main(void)
{
    // 1、初始化按键
    xiaozhi_button_int();
    // 2、注册按键回调函数
    xiaozhi_button2_regist(BUTTON_SINGLE_CLICK, NULL, adc_button_callback, (void *)1);
    xiaozhi_button2_regist(BUTTON_DOUBLE_CLICK, NULL, adc_button_callback, (void *)2);

    // 初始化 LCD，并将屏幕接入 LVGL
    xiaozhi_lvgl_init();

    // 获取屏幕对象，设置背景
    xiaozhi_lvgl_layout();

    // 临时验证二维码显示，之后替换为真实配网数据
    // xiaozhi_lvgl_show_qr_code("https://lvgl.io");
    xiaozhi_wifi_sta_init();

    //更新主页面的顶部内容
    xiaozhi_lvgl_update_top_content("奶龙小智");
    //中间内容
    xiaozhi_lvgl_update_mid_content("shocked");
    //底部内容
    xiaozhi_lvgl_update_bottom_content("我是奶龙");
}

void adc_button_callback(void *button_handle, void *usr_data)
{
    uint8_t btn_tag = (uint8_t)usr_data;
    switch (btn_tag)
    {
    case 1:
        // 重置WiFi信息
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
