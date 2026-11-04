#include "xiaozhi_sr.h"
static const char *TAG = "xiaozhi_sr";
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

        // 检测到唤醒词
        if (result->wakeup_state == WAKENET_DETECTED)
        {

            // 全局结构体唤醒词成员:标记
            xiaozhi_data.is_wakeup = true;
            // 检测到唤醒词,执行唤醒词回调函数
            if (xiaozhi_data.wakeup_callback != NULL)
            {
                xiaozhi_data.wakeup_callback();
            }
        }

        // 语音状态检测:说话,静音!!!!
        // 语音状态变化:持续静音,突然说话! (WS组件需要给虾哥服务器发送监听命令)     持续说话,突然静音!(WS组件需要给虾哥服务器发送停止监听)!
        if (xiaozhi_data.is_wakeup)
        {
            // 存储语音当前检测结果
            xiaozhi_data.vad_current_state = result->vad_state;
            // 当前状态与上一次状态不一样,说明语音状态发生变化
            if (xiaozhi_data.vad_current_state != xiaozhi_data.vad_last_state)
            {
                if (xiaozhi_data.vad_state_change_callback != NULL)
                {
                    xiaozhi_data.vad_state_change_callback();
                }
            }

            // 存储语音上一次状态结果
            xiaozhi_data.vad_last_state = xiaozhi_data.vad_current_state;
        }

        // 当语音识别模块识别到唤醒词且语音活动检测为说话状态,就可以将原始PCM数据发送到字节流缓冲区
        // 1.检测到唤醒词,这个用户喊过唤醒词,后续再说话!
        if (xiaozhi_data.is_wakeup && xiaozhi_data.vad_current_state == VAD_SPEECH)
        {
            // 缓存音频数据也需要发送缓冲区当中
            if (result->vad_cache_size > 0)
            {
                xRingbufferSend(xiaozhi_data.sr_to_encoder_handler, result->vad_cache, result->vad_cache_size, portMAX_DELAY);
            }

            // 将原始PCM数据发送到字节流缓冲区
            // 1.缓冲区句柄 2.向缓冲区发送原始语音音频数据  3.原始PCM数据大小 4.阻塞时间
            xRingbufferSend(xiaozhi_data.sr_to_encoder_handler, result->data, result->data_size, portMAX_DELAY);
        }
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
    // 声音检测灵敏度
    afe_config->vad_mode = VAD_MODE_3;
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