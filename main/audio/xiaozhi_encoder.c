#include "xiaozhi_encoder.h"
static const char *TAG = "xiaozhi_encoder";
esp_audio_enc_handle_t encoder = NULL;
// 存储原始PCM数据缓冲区:存储大小帧长60ms语音,字节1920个!
uint8_t *pcm_buffer = NULL;
// 存储压缩过后音频数据缓冲区:167字节
uint8_t *opus_buffer = NULL;
// 1.opus编码器进行原始数据PCM压缩,每一次压缩不是按一个字节一个字节压缩,按帧长【60ms】
// PCM:16KHZ->16000  60ms->16000*60/1000=960采样点    960*2=1920字节
// OPUS:比特率9000->167字节进行压缩
int pcm_size = 0, opus_size = 0;
// 编码回调函数
void encoder_task(void *pvParameters);

// 1.OPUS编码器初始化
void xiaozhi_opus_encoder_init(void)
{
    // 1.OPUS编码器相关参数配置
    esp_opus_enc_config_t opus_config = {
        .sample_rate = ESP_AUDIO_SAMPLE_RATE_16K,            // 采样频率16KHZ
        .channel = ESP_AUDIO_MONO,                           // 单声道
        .bits_per_sample = ESP_AUDIO_BIT16,                  // 每一个采样点位深16位表示一个声点
        .bitrate = 9000,                                     // 比特率
        .frame_duration = ESP_OPUS_ENC_FRAME_DURATION_60_MS, // 帧时间
        .application_mode = ESP_OPUS_ENC_APPLICATION_VOIP,   // 编码器应用模式
        .complexity = 5,                                     // 编码复杂度:0-10,数值越大，编码越复杂，编码质量越高，但编码速度越慢
        .enable_fec = false,                                 // 前向纠错,没有使用
        .enable_dtx = false,                                 // 连续编码,连续传输数据
        .enable_vbr = false,                                 // 比特率写死了不是可变的
    };

    // 2.根据上面参数信息开启opus编码器
    esp_opus_enc_open(&opus_config, sizeof(esp_opus_enc_config_t), &encoder);

    // 3.计算出需要原始音频字节个数与压缩过后opus音频字节个数

    esp_opus_enc_get_frame_size(encoder, &pcm_size, &opus_size);

    // 4.开辟缓冲区:分别存储PCM原始数据的缓冲区,存储压缩音频缓冲区大小
    pcm_buffer = heap_caps_malloc(pcm_size, MALLOC_CAP_SPIRAM);
    opus_buffer = heap_caps_malloc(opus_size, MALLOC_CAP_SPIRAM);

    // 5.开启一个freeRTOS任务,要进行原始音频数据编码
    xTaskCreatePinnedToCoreWithCaps(encoder_task, "encoder_task", 32 * 1024, NULL, 5, NULL, 1, MALLOC_CAP_SPIRAM);
}

void encoder_task(void *pvParameters)
{

    // 1.opus编码器需要配置:原始音频数据参数
    esp_audio_enc_in_frame_t pcm_frame = {
        .buffer = pcm_buffer, // 要进行压缩原始音频数据缓冲区
        .len = pcm_size,      // 原始音频数据缓冲区的大小
    };
    // 2.opus编码器需要配置的:压缩音频数据参数
    esp_audio_enc_out_frame_t opus_frame = {
        .buffer = opus_buffer, // 存储输出压缩音频数据缓冲区
        .len = opus_size,
    };

    while (1)
    {

        // 3.持续编码之前获取到原始PCM数据进行编码,放到pcm_buffer缓冲区当中
        esp_opus_enc_process(encoder, &pcm_frame, &opus_frame);
    }
}