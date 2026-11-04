#ifndef __XIAOZHI_DATA_H__
#define __XIAOZHI_DATA_H__
#include "stdbool.h"
#include "esp_vad.h"
#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"
#include "freertos/event_groups.h"

// 定义服务器(小智智能体状态)
typedef enum
{
    // 空闲状态:服务器没有给我们返回任何数据
    SERVER_IDLE,
    // 说话状态:服务器给我们返回文本、音频消息
    SERVER_SPEAKING,
    // 监听状态:客户端给服务端发送语音数据,可以接受数据,识别音频数据!
    SERVER_LISTENING

} SERVER_STATE_T;

typedef struct
{
    // 存储websocket服务器信息
    char *socket_url;
    char *token;

    // 1.标记是否检测到唤醒词
    bool is_wakeup;
    // 2.注册检测到唤醒词回调
    void (*wakeup_callback)(void);

    // 2.存储SR检测语音当前状态
    vad_state_t vad_current_state;
    // 3.存储SR检测语音上一次状态
    vad_state_t vad_last_state;

    // 4.语音状态发生变化回调
    void (*vad_state_change_callback)(void);

    // 将语音识别与编码器任务通信的环形缓冲区句柄,挂在到全局上!
    RingbufHandle_t sr_to_encoder_handler;

    // 编码器与ws客户端组件使用的缓冲区
    RingbufHandle_t encoder_to_ws_handler;

    // ws客户端与解码器组件使用的缓冲区
    RingbufHandle_t ws_to_decoder_handler;

    // 事件标志组的句柄
    EventGroupHandle_t event_group;

    // 注册处理ws服务器返回文本信息回调
    void (*ws_text_callback)(char *text, int len);
    // 注册处理ws服务器返回二进制数据回调
    void (*ws_binary_callback)(char *data, int len);

    // 服务器状态:小智的状态(悟空智能体)
    SERVER_STATE_T server_state;

} xiaozhi_data_t;

extern xiaozhi_data_t xiaozhi_data;
#endif /* __XIAOZHI_DATA_H__ */