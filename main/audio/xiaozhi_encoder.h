#ifndef __XIAOZHI_ENCODER_H__
#define __XIAOZHI_ENCODER_H__
#include "esp_opus_enc.h"
#include "stdio.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
// 1.opus编码器初始化
void xiaozhi_opus_encoder_init(void);
#endif

/**
 1.此组件目的,利用esp_audio_codec编码器能力进行音频数据压缩
 2.压缩目的,后期网络通信时候,传输速度会快一些!提高用户体验
 3.服务器:压缩 OPUS格式    16KHZ 16BIT  单声道
 **/