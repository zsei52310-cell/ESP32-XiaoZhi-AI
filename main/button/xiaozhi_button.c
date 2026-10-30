#include "xiaozhi_button.h"
// 按键2句柄
button_handle_t btn2_handle = NULL;
// 按键3句柄
button_handle_t btn3_handle = NULL;
static char *TAG = "xiaozhi_button";
// 1.ADC按键初始化方法
void xiaozhi_button_init(void)
{
    // 1.创建按键
    //  注册ADC按键2,相关参数
    button_config_t btn2_config = {0};

    button_adc_config_t btn2_adc_config = {
        .unit_id = ADC_UNIT_1, // 将来使用ADC1检测P8引脚电压变化
        .adc_channel = 7,      // 将来使用ADC1通道7检测P8引脚电压变化
        .button_index = 0,     // 按键索引
        .min = 0,              // 将来ADC1通道7检测P8引脚电压:0-300MV认为按键2按下
        .max = 300};
    // 创建ADC按键2
    esp_err_t ret = iot_button_new_adc_device(&btn2_config, &btn2_adc_config, &btn2_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "iot_button_new_adc_device failed");
    }
    else
    {
        ESP_LOGI(TAG, "iot_button_new_adc_device success");
    }

    // 注册ADC按键3,相关参数
    button_config_t btn3_config = {0};
    button_adc_config_t btn3_adc_config = {
        .unit_id = ADC_UNIT_1, // 将来使用ADC1检测P8引脚电压变化
        .adc_channel = 7,      // 将来使用ADC1通道7检测P8引脚电压变化
        .button_index = 1,     // 按键索引
        .min = 1300,           // 将来ADC1通道7检测P8引脚电压:0-300MV认为按键2按下
        .max = 1800};
    // 创建ADC按键2
    esp_err_t ret1 = iot_button_new_adc_device(&btn3_config, &btn3_adc_config, &btn3_handle);
    if (ret1 != ESP_OK)
    {
        ESP_LOGI(TAG, "iot_button_new_adc_device failed");
    }
    else
    {
        ESP_LOGI(TAG, "iot_button_new_adc_device success");
    }
}
// 按键2注册事件
void xiaozhi_button2_regist(button_event_t event, button_event_args_t *event_args, button_cb_t cb, void *usr_data)
{
    // 对应按键注册事件
    // 1.按键句柄 2.按键绑定事件 3.按键对应:长按时间、多次按下次数  4.第四个参数触发事件回调函数 5.第五个参数:用户自定义数据
    iot_button_register_cb(btn2_handle, event, event_args, cb, usr_data);
}
// 按键3注册事件
void xiaozhi_button3_regist(button_event_t event, button_event_args_t *event_args, button_cb_t cb, void *usr_data)
{
    iot_button_register_cb(btn3_handle, event, event_args, cb, usr_data);
}