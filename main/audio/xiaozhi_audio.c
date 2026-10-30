#include "xiaozhi_audio.h"
// I2C句柄
i2c_master_bus_handle_t i2c_bus_handle;
// I2S收发数据句柄
i2s_chan_handle_t tx_handle;
i2s_chan_handle_t rx_handle;
// 播放与录制声音句柄
esp_codec_dev_handle_t codec_dev;
// 初始化I2C
static void xiaozhi_audio_i2c_init(void);
// 初始化I2S
static void xiaozhi_audio_i2s_init(void);

// 1.编码器ES8311相关模块初始化
void xiaozhi_audio_codec_init(void)
{
    // 1.I2C + I2S初始化
    xiaozhi_audio_i2c_init();
    xiaozhi_audio_i2s_init();

    // 2.为编解码器设备实现控制接口，数据接口和 GPIO 接口 (使用默认提供的接口实现)
    // 编码器数据接口:I2S传输音频数据
    audio_codec_i2s_cfg_t i2s_cfg = {
        .rx_handle = rx_handle,
        .tx_handle = tx_handle};
    const audio_codec_data_if_t *data_if = audio_codec_new_i2s_data(&i2s_cfg);
    // 编解码器控制接口:I2C传输命令
    audio_codec_i2c_cfg_t i2c_cfg = {
        .addr = ES8311_CODEC_DEFAULT_ADDR,
        .bus_handle = i2c_bus_handle};
    const audio_codec_ctrl_if_t *out_ctrl_if = audio_codec_new_i2c_ctrl(&i2c_cfg);
    // GPIO接口:P7GPIO引脚控制外放
    const audio_codec_gpio_if_t *gpio_if = audio_codec_new_gpio();

    // 3.基于控制接口和 ES8311 特有的配置实现 audio_codec_if_t 接口
    es8311_codec_cfg_t es8311_cfg = {
        .codec_mode = ESP_CODEC_DEV_WORK_MODE_BOTH, // ES8311工作模式:ADC,DAC
        .ctrl_if = out_ctrl_if,                     // es8311相关配置通过I2C传递给ES8311进行配置
        .gpio_if = gpio_if,                         // 通过I2C告诉ES8311P7控制外放
        .pa_pin = 7,
        .use_mclk = true,
    };
    const audio_codec_if_t *out_codec_if = es8311_codec_new(&es8311_cfg);

    // 4.通过函数esp_codec_dev_new 获取句柄,根据这个句柄实现录音与播放声音
    esp_codec_dev_cfg_t dev_cfg = {
        .codec_if = out_codec_if,             // es8311_codec_new 获取到的接口实现
        .data_if = data_if,                   // audio_codec_new_i2s_data 获取到的数据接口实现
        .dev_type = ESP_CODEC_DEV_TYPE_IN_OUT // 设备同时支持录制和播放
    };
    // 获取录制声音与播放声音句柄
    codec_dev = esp_codec_dev_new(&dev_cfg);

    // 播放声音相关的参数初始化一次
    esp_codec_dev_set_out_vol(codec_dev, 60.0);
    esp_codec_dev_sample_info_t fs = {
        .sample_rate = 16000,  // 播放声音采样频率
        .channel = 1,          // 播放声音单声道
        .bits_per_sample = 16, // 播放音频数据位深16位
    };
    esp_codec_dev_open(codec_dev, &fs);

    // 录制声音相关参数
    esp_codec_dev_set_in_gain(codec_dev, 30.0);
}

// 初始化I2C
static void xiaozhi_audio_i2c_init(void)
{

    // 初始化I2C驱动:新版本写法
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,    // I2C时钟源
        .i2c_port = I2C_NUM_0,                // I2C端口
        .scl_io_num = GPIO_NUM_1,             // P1引脚作为时钟线
        .sda_io_num = GPIO_NUM_0,             // P0引脚作为数据线
        .flags.enable_internal_pullup = true, // 需要上拉电阻
        .glitch_ignore_cnt = 7,               // 毛刺过滤阈值。当总线上的干扰脉冲宽度小于该设定值（单位为 I2C 模块时钟周期）时，会被硬件滤除。典型值设为 7，有助于提高通信抗干扰能力 。
    };
    i2c_new_master_bus(&i2c_bus_config, &i2c_bus_handle);
}

// 初始化I2S
static void xiaozhi_audio_i2s_init(void)
{
    // I2S相关配置参数
    // 1.I2S_0   2.I2S主设备
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),                            // 采样频率:16KHZ
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(16, I2S_SLOT_MODE_MONO), // 采样点位深16位,单声道
        .gpio_cfg = {
            // 通信协议需要使用引脚编号
            .mclk = 3,
            .bclk = 2,
            .ws = 5,
            .dout = 6,
            .din = 4,
        },
    };

    // 2.I2S初始化
    i2s_new_channel(&chan_cfg, &tx_handle, &rx_handle);
    i2s_channel_init_std_mode(tx_handle, &std_cfg);
    i2s_channel_init_std_mode(rx_handle, &std_cfg);

    // 3.I2S声道收发使能打开
    i2s_channel_enable(tx_handle);
    i2s_channel_enable(rx_handle);
}

// 2.播放声音
void xiaozhi_audio_play(uint8_t *buf, int len)
{
    // 播放声音
    esp_codec_dev_write(codec_dev, buf, len);
}

// 3.录制声音
void xiaozhi_audio_record(uint8_t *buf, int len)
{
    esp_codec_dev_read(codec_dev, buf, len);
}
