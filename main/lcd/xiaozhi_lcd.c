#include "xiaozhi_lcd.h"

// IO设备句柄
esp_lcd_panel_io_handle_t io_handle = NULL;
// 面板句柄
esp_lcd_panel_handle_t panel_handle = NULL;
// 指针数组:将来用于存储图像的数据->存储图像数据双缓冲区
static uint16_t *s_lines[2];
// 保存整张图像的数据
uint8_t *image_bin;
//获取图像的首地址和末地址
extern const uint8_t image_jpg_start[] asm("_binary_image_jpg_start");
extern const uint8_t image_jpg_end[] asm("_binary_image_jpg_end");

// 指针数组:将来用于存储图像的数据->存储图像数据双缓冲区
// static uint16_t *s_lines[2];


//1、LCD初始化
void xiaozhi_lcd_Init(void){
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

    // IO设备句柄
    // esp_lcd_panel_io_handle_t io_handle = NULL;
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

    // 面板句柄
    // esp_lcd_panel_handle_t panel_handle = NULL;
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
    // 5.将来面板颜色进行翻转
    //  esp_lcd_panel_invert_color(panel_handle, true);

    // 6是否交换X、Y轴
    esp_lcd_panel_swap_xy(panel_handle, true);
    // 7.镜像的设置
    esp_lcd_panel_mirror(panel_handle, false, true);
    // 8.打开背光灯
    gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);

    for (int i = 0; i < 2; i++)
    {
        s_lines[i] = heap_caps_malloc(EXAMPLE_LCD_H_RES * PARALLEL_LINES * sizeof(uint16_t), MALLOC_CAP_DMA);
    }
}

//2、获取图像数据
void xiaozhi_lcd_getImageData(void){
    //开辟字节数大小,使用的是外部的空间
    image_bin = heap_caps_malloc(EXAMPLE_LCD_H_RES*EXAMPLE_LCD_V_RES*sizeof(uint16_t),MALLOC_CAP_SPIRAM);

     // 将图像的数据存储到缓存区
    esp_jpeg_image_cfg_t jpeg_cfg = {
        .indata = (uint8_t *)image_jpg_start,//找到整张图片首个像素点地址
        .indata_size = image_jpg_end - image_jpg_start,//计算图像大小
        .outbuf = image_bin,//存储图像缓冲区
        .outbuf_size = EXAMPLE_LCD_H_RES*EXAMPLE_LCD_V_RES*sizeof(uint16_t),//缓冲区大小
        .out_format = JPEG_IMAGE_FORMAT_RGB565,//输出数据格式
        .out_scale = JPEG_IMAGE_SCALE_0,//原图不需要缩放
        .flags = {
            .swap_color_bytes = 1,//每一个字节存储图像数据高低位进行交换
        }};
    esp_jpeg_image_output_t outimg;
    //将整张图像的数据,生成二进制数据,保存到pixels,提供此函数外部使用
    esp_jpeg_decode(&jpeg_cfg, &outimg);
}

//3、显示图像数据
void xiaozhi_lcd_show_Image(void){
    //本次发送哪个缓存区,0是A，1是B
    int sending_line = 0;
    //本次往哪个缓存区填数据，初始为0
    int calc_line = 0;
    //每批复制的字节数
    uint16_t every_once_copy_byte_number = EXAMPLE_LCD_H_RES * PARALLEL_LINES *2;

    //每一次处理2行，到240行
    for (uint8_t y = 0; y < EXAMPLE_LCD_V_RES; y += PARALLEL_LINES)
    {
        memcpy(s_lines[calc_line],image_bin,every_once_copy_byte_number);
        image_bin += every_once_copy_byte_number;
        sending_line = calc_line;
        calc_line = !calc_line;
        // 通过SPI将缓冲区数据发送LCD
        esp_lcd_panel_draw_bitmap(panel_handle, 0, y, 0 + EXAMPLE_LCD_H_RES, y + PARALLEL_LINES, s_lines[sending_line]);
    }
    
}
