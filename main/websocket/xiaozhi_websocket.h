#ifndef __XIAOZHI_WEBSOCKET_H__
#define __XIAOZHI_WEBSOCKET_H__
#include "esp_websocket_client.h"
#include "xiaozhi_data.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"
// 1.初始化ws客户端
void xiaozhi_ws_init(void);

#endif /* __XIAOZHI_WEBSOCKET_H__ */