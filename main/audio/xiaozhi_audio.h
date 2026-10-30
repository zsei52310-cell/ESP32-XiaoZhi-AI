#ifndef __XIAOZHI_AUDIO_H__
#define __XIAOZHI_AUDIO_H__
#include "driver/i2c_master.h"
#include "driver/i2s_std.h"
#include "esp_codec_dev_defaults.h"
#include "esp_codec_dev.h"
// 1.编解码器初始化方法
void xiaozhi_audio_codec_init(void);

// 2.播放声音
void xiaozhi_audio_play(uint8_t *buf, int len);

// 3.录制声音
void xiaozhi_audio_record(uint8_t *buf, int len);

#endif /* __XIAOZHI_AUDIO_H__ */