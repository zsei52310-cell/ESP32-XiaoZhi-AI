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
#include "freertos/event_groups.h"
static char *TAG = "MAIN";
// 按键事件触发回调
void adc_button_callBack(void *button_handle, void *usr_data);

// 创建环形缓冲区方法
static void xiaozhi_ringbuf_create(void);

// 根组件提供方法,用于获取SR识别唤醒词结果,检测到唤醒词会执行
void xiaozhi_sr_wakeup_callback(void);
// 根组件提供方法,用于指导语音状态发生变化
void xiaozhi_sr_state_change_callback(void);
// 处理服务器返回文本消息
void process_text(char *text, int len);
// 处理服务器返回语音消息
void process_voice(char *voice, int len);
// 测试任务
void test(void *pvParameters);

void app_main(void)
{

    // 创建事件标志组:ws客户端使用
    xiaozhi_data.event_group = xEventGroupCreate();

    // 最早时间点创建环形缓冲区
    xiaozhi_ringbuf_create();

    // 0.任意组件想知道检测到唤醒词,仅需要再com/xiaozhi_data.c注册回调函数即可!
    xiaozhi_data.wakeup_callback = xiaozhi_sr_wakeup_callback;
    // 任意组件想知道语音状态发生变化,仅需要再com/xiaozhi_data.c注册回调函数即可!
    xiaozhi_data.vad_state_change_callback = xiaozhi_sr_state_change_callback;
    // 主函数:处理虾哥服务器返回文本消息、语音消息
    xiaozhi_data.ws_text_callback = process_text;
    xiaozhi_data.ws_binary_callback = process_voice;

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

    // ws还未初始化,服务器不可能给咱们返回数据,服务器处于空闲状态
    xiaozhi_data.server_state = SERVER_IDLE;

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

// 检测到唤醒词SR
void xiaozhi_sr_wakeup_callback(void)
{

    ESP_LOGE(TAG, "检测到唤醒词");
    // 检测到唤醒词:建立连接 + 发送唤醒词进行第一次对话!
    if (xiaozhi_data.server_state == SERVER_IDLE)
    {
        xiaozhi_ws_wakeup_connect();
    }
    else if (xiaozhi_data.server_state == SERVER_SPEAKING)
    {
        // 立马终止说话
        xiaozhi_ws_send_abort();
        // 再发发送"你好小智"
        xiaozhi_ws_send_wakeup();
    }
}

// SR检测到语音状态变化
// SR检测到语音状态:说话,静音。没办法判断谁说话!
void xiaozhi_sr_state_change_callback(void)
{

    // 想通过SR的VAD算法:区分出用户是否开始说话了,用户开始说话了,给服务器发送文本命令,让他开始监听
    // 如果用户说话完事,发送文本命令,停止监听!
    ESP_LOGI(TAG, "SR检测到语音状态变化");
    // 1.判断用户说话!
    // SR检测到有声音
    if (xiaozhi_data.vad_current_state == VAD_SPEECH)
    {
        // 只要服务器没有返回数据,小智没有说话,一定是用户!
        if (xiaozhi_data.server_state == SERVER_IDLE)
        {
            // 通知服务器开始监听用户音频数据
            xiaozhi_ws_send_start_listen();
            // 小智状态修改
            xiaozhi_data.server_state = SERVER_LISTENING;
            // 更新顶部标题
            xiaozhi_lvgl_update_top_content("聆听中......");
        }
    }

    // 2.判断用户说话完毕:还需要发送停止监听命令
    // SR检测结果没有声音
    if (xiaozhi_data.vad_current_state == VAD_SILENCE)
    {
        // 服务器也没有给咱们返回数据:小智状态,空闲,监听状态!
        if (xiaozhi_data.server_state != SERVER_SPEAKING)
        {
            // 发送停止监听命令
            xiaozhi_ws_send_stop_listen();
            // 修改小智状态
            xiaozhi_data.server_state = SERVER_IDLE;
            xiaozhi_lvgl_update_top_content("尚硅谷AI小智");
        }
    }
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

        // 什么时候将缓冲区数据发送给服务器!服务器状态应该为监听状态
        if (xiaozhi_data.server_state == SERVER_LISTENING)
        {

            xiaozhi_ws_send_binary(data, len);
        }

        // 编码器与ws客户端之间缓冲区提取出来数据用完释放
        vRingbufferReturnItem(xiaozhi_data.encoder_to_ws_handler, data);
    }
}

// 处理服务器返回文本消息
void process_text(char *text, int len)
{
    // ESP_LOGE(TAG, "main:%s", text);
    cJSON *root = cJSON_Parse(text);
    // 提取服务器返回数据字段:type
    char *type = cJSON_GetObjectItem(root, "type")->valuestring;

    // 1.根据服务器返回文本信息,type字段:hello!说明虾哥服务器与回复消息,确认建立连接成功
    if (strcmp(type, "hello") == 0)
    {
        xEventGroupSetBits(xiaozhi_data.event_group, RESPONSE_HELLO_MSG_BIT);
    }

    // 处理屏幕上需要展示小智表情包
    if (strcmp(type, "llm") == 0)
    {

        char *emotion = cJSON_GetObjectItem(root, "emotion")->valuestring;
        // 更新屏幕
        xiaozhi_lvgl_update_mid_content(emotion);
    }

    // 处理屏幕展示小智说话文字内容
    if (strcmp(type, "tts") == 0)
    {
        char *state = cJSON_GetObjectItem(root, "state")->valuestring;
        if (strcmp(state, "sentence_start") == 0 || strcmp(state, "sentence_end") == 0)
        {
            // 提取小智说话内容展示屏幕上
            char *text = cJSON_GetObjectItem(root, "text")->valuestring;
            // 更新屏幕
            xiaozhi_lvgl_update_bottom_content(text);
        }

        // 判断小智(服务器是的开始给咱们返回数据),小智就开始说话
        if (strcmp(state, "start") == 0)
        {
            xiaozhi_lvgl_update_top_content("正在说话中.......");
            // 修改小智状态
            xiaozhi_data.server_state = SERVER_SPEAKING;
        }

        // 判断小智(服务器返回数据结束),小智不在说话
        if (strcmp(state, "stop") == 0)
        {
            // 修改小智状态
            xiaozhi_data.server_state = SERVER_IDLE;
            xiaozhi_lvgl_update_top_content("尚硅谷AI小智");
        }
    }

    // 解析完毕删除
    cJSON_Delete(root);
}
// 处理服务器返回语音消息
void process_voice(char *voice, int len)
{
    // 将小智回复音频数据扔到环形缓冲区当中
    xRingbufferSend(xiaozhi_data.ws_to_decoder_handler, voice, len, 1000);
}
