#include "xiaozhi_sr.h"
// AFE句柄
const esp_afe_sr_iface_t *afe_handle;
// AFE实例
esp_afe_sr_data_t *afe_data;
// 给AFE持续喂音频数据任务
void feed_task(void *params)
{
    // AFE喂数据的采样点的个数
    int audio_chunksize = afe_handle->get_feed_chunksize(afe_data);
    // AFE喂数据使用几个通道
    int nch = afe_handle->get_feed_channel_num(afe_data);
    ESP_LOGE("SR", "audio_chunksize:%d,nch:%d", audio_chunksize, nch);
    int16_t *i2s_buff = heap_caps_malloc(audio_chunksize * sizeof(int16_t) * nch, MALLOC_CAP_SPIRAM);

    // 持续喂数据
    while (1)
    {
        // 录制用户声音
        xiaozhi_audio_record((uint8_t *)i2s_buff, audio_chunksize * sizeof(int16_t) * nch);

        // 给AFE喂PCM原始音频数据
        afe_handle->feed(afe_data, i2s_buff);
    }
}

// AFE提取语音识别结果任务
void detect_Task(void *params)
{

    while (1)
    {
        // 通过AFE算法持续获取语音识别结果
        afe_fetch_result_t *result = afe_handle->fetch(afe_data);

        // 判断唤醒词检测结果
        if (result->wakeup_state == WAKENET_DETECTED)
        {
            ESP_LOGE("SR", "检测唤醒词小智++++++++++++++++++++++++++++++++++++++");
        }
        else
        {
            ESP_LOGW("SR", "未检测到唤醒词---------------------------------------");
        }

        // 判断语音状态:说话、没说话
        // if (result->vad_state == VAD_SILENCE)
        // {
        //     ESP_LOGI("SR", "检测到静音!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        // }
        // else
        // {
        //     ESP_LOGE("SR", "检测到说话@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
        // }
    }
}

void xiaozhi_sr_init(void)
{

    // ES8311编码器:录制声音、播放声音功能
    xiaozhi_audio_codec_init();

    // 1.第一步,初始化AFE配置
    srmodel_list_t *models = esp_srmodel_init("model");
    // 1.AFE声学前端使用单MIC输入音频数据 2.models神经网络模型算法->你好,小智!  3.AFE声学前端使用场景语音识别  4.性能选择
    afe_config_t *afe_config = afe_config_init("M", models, AFE_TYPE_SR, AFE_MODE_HIGH_PERF);
    //  关闭一些硬件上本来就不支持的功能 免得出现反效果
    afe_config->aec_init = false; // 禁用回声消除
    afe_config->se_init = false;  // 禁用人声增强
    afe_config->ns_init = false;  // 禁用噪声消除
    afe_config->wakenet_init = true;
    // 提升了唤醒的灵敏度
    afe_config->wakenet_mode = DET_MODE_90;
    //声音检测灵敏度
    afe_config->vad_mode =  VAD_MODE_3;
    afe_config->afe_ringbuf_size = 4 * 1024;
    // 更多的内存分配到外部的sram里去
    afe_config->memory_alloc_mode = AFE_MEMORY_ALLOC_MORE_PSRAM;


    // 2.创建AFE实例
    //  获取句柄
    afe_handle = esp_afe_handle_from_config(afe_config);
    // 创建实例
    afe_data = afe_handle->create_from_config(afe_config);
    // 释放afe_config
    afe_config_free(afe_config);

    // AFE声学前端语音识别:任务栈的大小一定要大一些
    // 处理音频相关任务,都是用内核1!处理音频相关任务都交给内核1!

    // 第三步:喂数据AFE
    xTaskCreatePinnedToCoreWithCaps(feed_task, "feed_task", 32 * 1024, NULL, 5, NULL, 1, MALLOC_CAP_SPIRAM);

    // 第四步:提取结果
    xTaskCreatePinnedToCoreWithCaps(detect_Task, "detect_Task", 32 * 1024, NULL, 5, NULL, 1, MALLOC_CAP_SPIRAM);
}