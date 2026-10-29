#ifndef __XIAOZHI_LVGL_H__
#define __XIAOZHI_LVGL_H__

#include "xiaozhi_lcd.h"
#include "esp_lvgl_port.h"
#include "font_emoji.h"
#include "xiaozhi_emoj.h"
//1.进行LVGL初始化
void xiaozhi_lvgl_init(void);

//显示二维码
void xiaozhi_lvgl_show_qr_code(char *text);

//配网完成之后删除二维码
void xiaozhi_lvgl_del_qr_code();

//布置液晶显示屏的布局
void xiaozhi_lvgl_layout();

void xiaozhi_lvgl_update_top_content(char *text);
   
void xiaozhi_lvgl_update_mid_content(char *text);
    
void xiaozhi_lvgl_update_bottom_content(char *text);
#endif /* __XIAOZHI_LVGL_H__ */
