#ifndef __XIAOZHI_SR_H__
#define __XIAOZHI_SR_H__
#include "esp_afe_sr_models.h"
#include "freeRTOS/freeRTOS.h"
#include "freertos/idf_additions.h"
#include "esp_log.h"
#include  "esp_heap_caps.h"
#include "xiaozhi_audio.h"





// 1.初始化方法
void xiaozhi_sr_init(void);

#endif

/**
 此组件用途:使用SR框架中AFE声学前端算法:检测语音状态、与唤醒词检测!!!!!
 **/