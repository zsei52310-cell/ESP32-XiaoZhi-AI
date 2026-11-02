#ifndef __XIAOZHI_DATA_H__
#define __XIAOZHI_DATA_H__
#include "stdbool.h"
#include "esp_vad.h"
#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"
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

} xiaozhi_data_t;

extern xiaozhi_data_t xiaozhi_data;
#endif /* __XIAOZHI_DATA_H__ */