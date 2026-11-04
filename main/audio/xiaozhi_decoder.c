#include "xiaozhi_decoder.h"
// OPUS解码器句柄
esp_audio_dec_handle_t decoder;
// 解码任务
void decoder_task(void *pvParameters);
void xiaozhi_opus_decoder_init(void)
{

    // 1.解码器相关配置参数与开启
    esp_opus_dec_cfg_t opus_cfg = {
        .sample_rate = ESP_AUDIO_SAMPLE_RATE_16K,            // 采样频率
        .channel = ESP_AUDIO_MONO,                           // 单声道
        .frame_duration = ESP_OPUS_DEC_FRAME_DURATION_60_MS, // 帧时长
        .self_delimited = false,                             // 界定每个包的边界
    };

    // 2.开启解码器
    esp_opus_dec_open(&opus_cfg, sizeof(opus_cfg), &decoder);

    // 3.创建解码器任务,只要环形缓冲区内部有数据,提取出来解码,进行播放!
    xTaskCreatePinnedToCoreWithCaps(decoder_task, "decoder_task", 32 * 1024, NULL, 5, NULL, 1, MALLOC_CAP_SPIRAM);
}

void decoder_task(void *pvParameters)
{

    // 存储解码音频数据:opus
    esp_audio_dec_in_raw_t opus_frame = {
        .buffer = NULL, // 存放需要解码数据
        .len = 0,       // 需要解码音频数据字节个数
                        // consumed成员能知道当前解码器已经解码字节个数
    };
    // 存储解码完成音频数据:PCM
    esp_audio_dec_out_frame_t pcm_frame = {
        .buffer = heap_caps_malloc(10 * 1024, MALLOC_CAP_SPIRAM), // 存放解码完成音频数据PCM缓冲
        .len = 10 * 1024,                                         // 缓冲区大小    //.decoded_size实际解码出来音频数据字节个数

    };
    esp_audio_dec_info_t aud_info;
    while (1)
    {
        // 1.从ws客户单与解码器环形缓冲区提取出需要解码数据

        void *data = xRingbufferReceive(xiaozhi_data.ws_to_decoder_handler, &opus_frame.len, portMAX_DELAY);
        opus_frame.buffer = data;

        // 持续解码
        // opus_frame.len 表示实际需要解码字节个数,只要大于零,说明数据需要解码
        while (opus_frame.len > 0)
        {
            // 解码一次
            esp_opus_dec_decode(decoder, &opus_frame, &pcm_frame, &aud_info);

            // 更新实际需要解码字节个数
            opus_frame.len -= opus_frame.consumed;
            // 更新解码数据位置
            opus_frame.buffer += opus_frame.consumed;

            // 通过功放播放出来
            xiaozhi_audio_play(pcm_frame.buffer, pcm_frame.decoded_size);
        }

        // 实际需要解码数据全部解码完成
        vRingbufferReturnItem(xiaozhi_data.ws_to_decoder_handler, data);
    }
}