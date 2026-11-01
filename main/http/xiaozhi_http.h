#ifndef __XIAOZHI_HTTP_H__
#define __XIAOZHI_HTTP_H__
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"
#include "esp_tls.h"
#include "esp_wifi.h"
#include <esp_random.h>
#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include "xiaozhi_lvgl.h"
#include "xiaozhi_data.h"

// 1.HTTP客户端初始化:POST请求、获取到激活码与scoket服务器信息
void xiaozhi_http_client_init(void);

#endif /* __XIAOZHI_HTTP_H__ */