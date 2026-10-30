#ifndef __XIAOZHI_LVGL_H__
#define __XIAOZHI_LVGL_H__
#include "xiaozhi_lcd.h"
#include "esp_lvgl_port.h"
#include  "font_emoji.h"
#include "xiaozhi_emo.h"

// 1.进行LVGL初始化
void xiaozhi_lvgl_init(void);

// 2.学习LVGL的API语法
//  void xiaozhi_lvgl_api(void);

// 3.展示二维码的方法->未配网的时候,需要展示二维码
void xiaozhi_lvgl_show_qr_code(char *text);
// 4.删除二维码
void xiaozhi_lvgl_del_qr_code(void);
// 5LCD液晶显示屏布局
void xiaozhi_lvgl_layout(void);

// 6.更新顶部内容
void xiaozhi_lvgl_update_top_content(char *text);
// 7.更新中间部分内容
void xiaozhi_lvgl_update_mid_content(char *text);
// 8.更新底部内容
void xiaozhi_lvgl_update_bottom_content(char *text);

#endif /* __XIAOZHI_LVGL_H__ */