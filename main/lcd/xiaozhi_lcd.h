#ifndef __XIAOZHI_LCD_H__
#define __XIAOZHI_LCD_H__

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_heap_caps.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "jpeg_decoder.h"
#include "string.h"
// 使用SPI2与液晶显示屏通信
#define LCD_HOST SPI2_HOST
// 表示主控SPI2每一次通信可以给多少行数据:示例书写16 * 15,这个数值可以修改的,但是务必能被240整除!
#define PARALLEL_LINES 2
#define ROTATE_FRAME 30

// 通信时钟频率20MHZ
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)
// 打开背光灯需要电平:高电平导通!
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL 1
// 关闭背光灯需要电平:低电平关闭!
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL

// SPI通信MOSI引脚编号
#define EXAMPLE_PIN_NUM_DATA0 48
// SPI通信时钟线的引脚编号
#define EXAMPLE_PIN_NUM_PCLK 47
// SPI通信片选引脚编号
#define EXAMPLE_PIN_NUM_CS 21
// DC引脚
#define EXAMPLE_PIN_NUM_DC 45
// 复位
#define EXAMPLE_PIN_NUM_RST 16
// 控制背光灯IO引脚编号
#define EXAMPLE_PIN_NUM_BK_LIGHT 40

// 液晶显示屏尺寸大小320 * 240
#define EXAMPLE_LCD_H_RES 320
#define EXAMPLE_LCD_V_RES 240
// 数据位与命令位都是八位的
#define EXAMPLE_LCD_CMD_BITS 8
#define EXAMPLE_LCD_PARAM_BITS 8


//1、LCD初始化
void xiaozhi_lcd_Init(void);

//2、获取图像数据
void xiaozhi_lcd_getImageData(void);

//3、显示图像数据
void xiaozhi_lcd_show_Image(void);

#endif /* __XIAOZHI_LCD_H__ */
