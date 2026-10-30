#include "xiaozhi_lvgl.h"
static lv_display_t *lvgl_disp = NULL;
// IO设备句柄
extern esp_lcd_panel_io_handle_t io_handle;
// 面板句柄
extern esp_lcd_panel_handle_t panel_handle;

// 屏幕组件
lv_obj_t *screen = NULL;
// 二维码组件
lv_obj_t *qrcode = NULL;
// 顶部文本组件
lv_obj_t *top = NULL;
lv_obj_t *mid = NULL;
lv_obj_t *bottom = NULL;

// 汉字字体:vscode编码格式一定要用utf-8
extern lv_font_t font_puhui_14_1;
extern lv_font_t font_puhui_16_4;
extern lv_font_t font_puhui_20_4;
extern lv_font_t font_puhui_30_4;

#define QR_DARK lv_color_hex(0x000000)
#define QR_LIGHT lv_color_hex(0xffffff)
void xiaozhi_lvgl_init(void)
{
    // 1.初始化液晶显示屏
    xiaozhi_lcd_init();

    // 2.lvgl与LCD进行关联
    //  1.初始化LVGL
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,       // 任务优先级
        .task_stack = 4096 * 2,   // 任务栈大小
        .task_affinity = -1,      // 使用哪一个内容执行这个任务,没有指定内核
        .task_max_sleep_ms = 500, // 此任务阻塞时间,最长500ms
        .timer_period_ms = 5,     // 每个5ms刷新一次数据
    };
    lvgl_port_init(&lvgl_cfg);

    // 2.lvgl与当前项目的液晶显示屏进行关联
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = EXAMPLE_LCD_H_RES * PARALLEL_LINES, // LVGL帮我们创建缓冲区大小->像素点不是字节数
        .double_buffer = false,
        .hres = EXAMPLE_LCD_H_RES,
        .vres = EXAMPLE_LCD_V_RES,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT_RGB565,
        .rotation = {
            .swap_xy = true,
            .mirror_x = false,
            .mirror_y = true,
        },
        .flags = {
            .buff_dma = true,
            .swap_bytes = true,
        }};
    lvgl_disp = lvgl_port_add_disp(&disp_cfg);
}

// // LVGL基本语法学习
void xiaozhi_lvgl_api(void)
{

    // 进入临界区
    lvgl_port_lock(0);

    // 1.LVGL开发单屏幕,整个项目只能有唯一的一个根组件 "屏幕"
    // lv_screen_act()!属于老版本写法
    lv_obj_t *screen = lv_screen_active();

    // 2.设置样式"外观"
    // 整个屏幕背景颜色:0x003a57
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x003a57), LV_PART_MAIN);
    // 整个屏幕文字颜色:0xffffff
    lv_obj_set_style_text_color(screen, lv_color_hex(0xffffff), LV_PART_MAIN);

    // 3.添加一个文本
    lv_obj_t *title = lv_label_create(screen);
    // 设置文本内容
    lv_label_set_text(title, "Hello LVGL");
    // 调整位置
    lv_obj_set_align(title, LV_ALIGN_CENTER);

    // 4.按键组件
    lv_obj_t *button = lv_button_create(screen);
    lv_obj_set_pos(button, lv_pct(50), 50);
    lv_obj_set_size(button, 100, 50);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x000000), LV_PART_MAIN);

    // 二维码组件
    lv_obj_t *qrcode = lv_qrcode_create(screen);
    lv_qrcode_set_size(qrcode, 150);
    lv_qrcode_set_dark_color(qrcode, QR_DARK);
    lv_qrcode_set_light_color(qrcode, QR_LIGHT);
    lv_qrcode_set_data(qrcode, "https://lvgl.io");
    lv_qrcode_set_quiet_zone(qrcode, true);
    lv_obj_set_style_border_color(qrcode, QR_DARK, 0);
    lv_obj_set_style_border_width(qrcode, 4, 0);
    lv_obj_set_align(qrcode, LV_ALIGN_CENTER);

    // 删除组件
    lv_obj_delete(qrcode);

    // 退出临界区
    lvgl_port_unlock();
}

// 3.展示二维码
void xiaozhi_lvgl_show_qr_code(char *text)
{

    // 进入临界区
    lvgl_port_lock(0);
    // 二维码组件
    qrcode = lv_qrcode_create(screen);
    lv_qrcode_set_size(qrcode, 150);
    lv_qrcode_set_dark_color(qrcode, QR_DARK);
    lv_qrcode_set_light_color(qrcode, QR_LIGHT);
    lv_qrcode_set_data(qrcode, text);
    lv_qrcode_set_quiet_zone(qrcode, true);
    lv_obj_set_style_border_color(qrcode, QR_DARK, 0);
    lv_obj_set_style_border_width(qrcode, 4, 0);
    lv_obj_set_align(qrcode, LV_ALIGN_CENTER);
    // 退出临界区
    lvgl_port_unlock();
}

void xiaozhi_lvgl_del_qr_code(void)
{

    // 进入临界区
    lvgl_port_lock(0);

    lv_obj_delete(qrcode);
    // 退出临界区
    lvgl_port_unlock();
}

void xiaozhi_lvgl_layout(void)
{

    // 进入临界区
    lvgl_port_lock(0);

    // 1.LVGL开发单屏幕,整个项目只能有唯一的一个根组件 "屏幕"
    // lv_screen_act()!属于老版本写法
    screen = lv_screen_active();
    //     // 整个屏幕背景颜色:0x003a57
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x003a57), LV_PART_MAIN);
    // 整个屏幕文字颜色:0xffffff
    lv_obj_set_style_text_color(screen, lv_color_hex(0xffffff), LV_PART_MAIN);

    // 2.液晶显示屏顶部内容
    top = lv_label_create(screen);
    lv_label_set_text(top, "");
    lv_obj_set_align(top, LV_ALIGN_TOP_MID);
    // 设置字体:lv_font类型结构体指针
    lv_obj_set_style_text_font(top, &font_puhui_20_4, LV_PART_MAIN);

    // 3.液晶显示屏中间部分
    mid = lv_label_create(screen);
    lv_label_set_text(mid, "");
    lv_obj_set_align(mid, LV_ALIGN_CENTER);
    lv_obj_set_style_text_font(mid, font_emoji_64_init(), LV_PART_MAIN);
    // 4.液晶显示屏底部内容
    bottom = lv_label_create(screen);
    lv_label_set_text(bottom, "");
    lv_obj_set_align(bottom, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_text_font(bottom, &font_puhui_14_1, LV_PART_MAIN);

    // 退出临界区
    lvgl_port_unlock();
}

// 6.更新顶部内容
void xiaozhi_lvgl_update_top_content(char *text)
{
    // 进入临界区
    lvgl_port_lock(0);
    lv_label_set_text(top, text);
    // 退出临界区
    lvgl_port_unlock();
}
// 7.更新中间部分内容
void xiaozhi_lvgl_update_mid_content(char *text)
{
    // 进入临界区
    lvgl_port_lock(0);
    for (uint8_t i = 0; i < 21; i++)
    {
        if (strcmp(text, emoji_array[i].name) == 0)
        {
            lv_label_set_text(mid, emoji_array[i].emoji);
        }
    }
    // 退出临界区
    lvgl_port_unlock();
}
// 8.更新底部内容
void xiaozhi_lvgl_update_bottom_content(char *text)
{
    // 进入临界区
    lvgl_port_lock(0);
    lv_label_set_text(bottom, text);
    // 退出临界区
    lvgl_port_unlock();
}