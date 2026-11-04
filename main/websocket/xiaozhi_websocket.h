#ifndef __XIAOZHI_WEBSOCKET_H__
#define __XIAOZHI_WEBSOCKET_H__
#include "esp_websocket_client.h"
#include "xiaozhi_data.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define CONNECT_SUCCESS_BIT (1 << 0)
#define RESPONSE_HELLO_MSG_BIT (1 << 1)

// 1.初始化ws客户端
void xiaozhi_ws_init(void);
// 2.发送Hello文本消息方法
void xiaozhi_ws_send_hello(void);
// 3.发送唤醒词【你好,小智】方法
void xiaozhi_ws_send_wakeup(void);
//4.SR检测到唤醒词【你好,小智】,与服务器端建立连接->唤醒词
void xiaozhi_ws_wakeup_connect(void);
//5.ws客户端向服务器发送文本命令:开始监听用户音频数据
void xiaozhi_ws_send_start_listen(void);
//6.发送停止监听状态
void xiaozhi_ws_send_stop_listen(void);
//发送音频数据
void xiaozhi_ws_send_binary(char *data, int len);

//7.终止消息
void xiaozhi_ws_send_abort(void);
#endif /* __XIAOZHI_WEBSOCKET_H__ */