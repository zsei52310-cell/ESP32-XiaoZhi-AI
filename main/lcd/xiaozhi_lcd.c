#include "xiaozhi_lcd.h"
// IO设备句柄
esp_lcd_panel_io_handle_t io_handle = NULL;
// 面板句柄
esp_lcd_panel_handle_t panel_handle = NULL;
// 保存整张图像的数据
uint8_t *image_bin;
// 获取图像首地址与末地址
extern const uint8_t image_jpg_start[] asm("_binary_image_jpg_start");
extern const uint8_t image_jpg_end[] asm("_binary_image_jpg_end");

// 用于存储每一次展示图像部分数据缓冲区-双缓冲区
static uint16_t *s_lines[2];
// 1.初始化方法
void xiaozhi_lcd_init(void)
{

    // 1.初始化P40引脚,控制LCD背光灯
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_BK_LIGHT};
    gpio_config(&bk_gpio_config);

    // 2.主控是通过SPI2与LCD通信,SPI驱动初始化
    spi_bus_config_t buscfg = {
        .sclk_io_num = EXAMPLE_PIN_NUM_PCLK,
        .mosi_io_num = EXAMPLE_PIN_NUM_DATA0,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = PARALLEL_LINES * EXAMPLE_LCD_H_RES * 2 + 8};
    spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);

    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = EXAMPLE_PIN_NUM_DC,
        .cs_gpio_num = EXAMPLE_PIN_NUM_CS,
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,
        .lcd_param_bits = EXAMPLE_LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle);

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };
    // 初始化驱动芯片
    esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle);

    // 1.拉低P40引脚,先把背光灯关闭
    gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL);
    // 2.LCD复位
    esp_lcd_panel_reset(panel_handle);
    // 3.初始化面板
    esp_lcd_panel_init(panel_handle);
    // 4.面板开启
    esp_lcd_panel_disp_on_off(panel_handle, true);
    // 6是否交换X、Y轴
    esp_lcd_panel_swap_xy(panel_handle, true);
    // 7.镜像的设置
    esp_lcd_panel_mirror(panel_handle, false, true);
    // 8.打开背光灯
    gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);

    // 9.双缓冲区空间开辟大小
    // for (uint8_t i = 0; i < 2; i++)
    // {
    //     // 320 * PARALLEL_LINES * 2
    //     s_lines[i] = heap_caps_malloc(EXAMPLE_LCD_H_RES * PARALLEL_LINES * 2, MALLOC_CAP_SPIRAM);
    // }
}

// 2.获取图像的数据
void xiaozhi_lcd_getImageData(void)
{
    // 1.开辟字节个数  第二个参数:空间使用外部8M
    image_bin = heap_caps_malloc(EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * sizeof(uint16_t), MALLOC_CAP_SPIRAM);

    // 2.将图像的数据存储到缓冲区image_bin当中
    esp_jpeg_image_cfg_t jpeg_cfg = {
        .indata = (uint8_t *)image_jpg_start,
        .indata_size = image_jpg_end - image_jpg_start,
        .outbuf = image_bin,
        .outbuf_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * sizeof(uint16_t),
        .out_format = JPEG_IMAGE_FORMAT_RGB565,
        .out_scale = JPEG_IMAGE_SCALE_0,
        .flags = {
            .swap_color_bytes = 1,
        }};

    esp_jpeg_image_output_t outimg;
    esp_jpeg_decode(&jpeg_cfg, &outimg);
}

// 3.用于展示图像的数据
void xiaozhi_lcd_show_Image(void)
{
    // 局部变量:双缓冲区使用角标
    int sending_line = 0; // 发送角标
    int calc_line = 0;    // 计算角标

    uint16_t evey_once_copy_byte_number = EXAMPLE_LCD_H_RES * PARALLEL_LINES * 2;

    for (uint8_t y = 0; y < EXAMPLE_LCD_V_RES; y += PARALLEL_LINES)
    {

        memcpy(s_lines[calc_line], image_bin, evey_once_copy_byte_number);
        image_bin += evey_once_copy_byte_number;
        sending_line = calc_line;
        calc_line = !calc_line;
        // 通过SPI将缓冲区数据发送LCD
        esp_lcd_panel_draw_bitmap(panel_handle, 0, y, 0 + EXAMPLE_LCD_H_RES, y + PARALLEL_LINES, s_lines[sending_line]);
    }
}