#include <stdio.h>
#include "xiaozhi_button.h"
#include "xiaozhi_wifi_sta.h"
#include "xiaozhi_lvgl.h"
#include "xiaozhi_audio.h"
#include "xiaozhi_http.h"
#include "xiaozhi_sr.h"
#include "xiaozhi_encoder.h"
#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"
#include "xiaozhi_decoder.h"
#include "xiaozhi_websocket.h"
static char *TAG = "MAIN";
// 按键事件触发回调
void adc_button_callBack(void *button_handle, void *usr_data);

// 创建环形缓冲区方法
static void xiaozhi_ringbuf_create(void);

// 根组件提供方法,用于获取SR识别唤醒词结果,检测到唤醒词会执行
void xiaozhi_sr_wakeup_callback(void);
// 根组件提供方法,用于指导语音状态发生变化
void xiaozhi_sr_state_change_callback(void);
// 测试任务
void test(void *pvParameters);

void app_main(void)
{

    // 最早时间点创建环形缓冲区
    xiaozhi_ringbuf_create();

    // 0.任意组件想知道检测到唤醒词,仅需要再com/xiaozhi_data.c注册回调函数即可!
    xiaozhi_data.wakeup_callback = xiaozhi_sr_wakeup_callback;
    // 任意组件想知道语音状态发生变化,仅需要再com/xiaozhi_data.c注册回调函数即可!
    xiaozhi_data.vad_state_change_callback = xiaozhi_sr_state_change_callback;

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

    // 5.ws客户端初始化[上来就要连接服务器]!
    xiaozhi_ws_init();

    // 5.初始化语音识别
    xiaozhi_sr_init();

    // 6.编码器初始化
    xiaozhi_opus_encoder_init(); // 编码器开启

    // 7.解码器初始化
    xiaozhi_opus_decoder_init(); // 解码器开启

    // 8.测试任务
    xTaskCreatePinnedToCoreWithCaps(test, "test", 32 * 1024, NULL, 5, NULL, 1, MALLOC_CAP_SPIRAM);
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

void xiaozhi_sr_wakeup_callback(void)
{
    ESP_LOGE(TAG, "MAIN SR WAKEUP CALLBACK");
}

void xiaozhi_sr_state_change_callback(void)
{
    ESP_LOGI(TAG, "MAIN SR STATE CHANGE CALLBACK");
}

// 创建环形缓冲区
static void xiaozhi_ringbuf_create(void)
{

    // 创建环形缓冲区
    // 1.语音识别项目,用户一般能接受延迟时间300ms以内用户根本感知不到延迟
    // 市面上语音识别延迟时间,用户感知时间300ms以后,用户根本感觉不到延迟!
    // 1920->60ms     1920 * 5 = 9600字节数,缓冲区大小9600/1024 = 8.9k
    xiaozhi_data.sr_to_encoder_handler = xRingbufferCreateWithCaps(10 * 1024, RINGBUF_TYPE_BYTEBUF, MALLOC_CAP_SPIRAM);

    // 2.创建环形缓冲区,编码器与websocket组件使用的!
    xiaozhi_data.encoder_to_ws_handler = xRingbufferCreateWithCaps(10 * 1024, RINGBUF_TYPE_NOSPLIT, MALLOC_CAP_SPIRAM);

    // 3.创建环形缓冲区,ws与解码器组件使用的!
    xiaozhi_data.ws_to_decoder_handler = xRingbufferCreateWithCaps(10 * 1024, RINGBUF_TYPE_NOSPLIT, MALLOC_CAP_SPIRAM);
}

// 测试代码:后续不用再干掉！
void test(void *pvParameters)
{

    while (1)
    {
        size_t len = 0;
        // 先把编码完成音频数据,发送解码对应缓冲区当中播放看一看
        void *data = xRingbufferReceive(xiaozhi_data.encoder_to_ws_handler, &len, portMAX_DELAY);
        xRingbufferSend(xiaozhi_data.ws_to_decoder_handler, data, len, portMAX_DELAY);
        // 编码器与ws客户端之间缓冲区提取出来数据用完释放
        vRingbufferReturnItem(xiaozhi_data.encoder_to_ws_handler, data);
    }
}
