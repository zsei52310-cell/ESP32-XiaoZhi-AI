#include "xiaozhi_lvgl.h"
static lv_display_t *lvgl_disp = NULL;
// IO设备句柄
extern esp_lcd_panel_io_handle_t io_handle;
// 面板句柄
extern esp_lcd_panel_handle_t panel_handle;
/*
    为什么放在外面，因为布局函数负责创建它们，后面的3个更新函数也需要访问它们，所以这个
    需要提升为全局变量
*/
// 屏幕组件
lv_obj_t *screen = NULL;

// 二维码组件
lv_obj_t *qrcode = NULL;

// 顶部文本组件
lv_obj_t *top = NULL;
// 中间文本组件
lv_obj_t *mid = NULL;
// 底部文本组件
lv_obj_t *bottom = NULL;

// 引用字体组件中的中文字体
extern const lv_font_t font_puhui_20_4;
extern const lv_font_t font_puhui_14_1;

// 二维码深色和浅色
#define QR_DARK lv_color_hex(0x000000)
#define QR_LIGHT lv_color_hex(0xffffff)

void xiaozhi_lvgl_init(void)
{
    // 1.初始化液晶显示屏
    xiaozhi_lcd_Init();

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

void xiaozhi_lvgl_show_qr_code(char *text)
{
    // 1、进入临界区
    lvgl_port_lock(0);

    // 在屏幕这个父对象上创建一个二维码组件，并把返回的对象指针保存到qrcode
    qrcode = lv_qrcode_create(screen);
    // 设置二维码的大小,150*150像素
    lv_qrcode_set_size(qrcode, 150);
    // 深色部分，黑色
    lv_qrcode_set_dark_color(qrcode, QR_DARK);
    // 浅色部分，白色
    lv_qrcode_set_light_color(qrcode, QR_LIGHT);
    // 把函数参数text指向的字符串编码成二维码
    lv_qrcode_set_data(qrcode, text);
    // 保留二维码四周的留白区域，方便扫码识别
    lv_qrcode_set_quiet_zone(qrcode, true);
    // 设置组件边框颜色
    lv_obj_set_style_border_color(qrcode, QR_DARK, 0);
    // 设置边框宽度为4像素
    lv_obj_set_style_border_width(qrcode, 4, 0);
    // 二维码在父对象中居中
    lv_obj_set_align(qrcode, LV_ALIGN_CENTER);
    // 退出临界区
    lvgl_port_unlock();
}

// 配网完成之后删除二维码
void xiaozhi_lvgl_del_qr_code()
{

    // 进入临界区
    lvgl_port_lock(0);

    lv_obj_delete(qrcode);
    // 退出临界区
    lvgl_port_unlock();
}

// 布置液晶显示屏的布局
void xiaozhi_lvgl_layout()
{
    // 1、进入临界区
    lvgl_port_lock(0);

    // 引用这个根组件,在布局函数里获取屏幕，并设置背景
    // 获取当前屏幕，把地址保存到全局screen
    screen = lv_screen_active();
    // 设置屏幕背景
    lv_obj_set_style_bg_color(screen,lv_color_hex(0x36BFC8),LV_PART_MAIN);
    // 设置字体颜色
    lv_obj_set_style_text_color(screen, lv_color_hex(0x17374F), LV_PART_MAIN);
    // 1、液晶显示屏顶部内容
    // 在screen上创建一个标签，把对象地址存入top
    top = lv_label_create(screen);
    // 标签已经创建，往里面放入显示的文字
    lv_label_set_text(top, "");
    // 让标签位于屏幕的顶部中央
    lv_obj_set_align(top, LV_ALIGN_TOP_MID);
    // 指定这套标签使用哪套字体
    lv_obj_set_style_text_font(top, &font_puhui_20_4, LV_PART_MAIN);
    // 2、显示屏幕中间部分
    mid = lv_label_create(screen);
    lv_label_set_text(mid, "");
    lv_obj_set_align(mid, LV_ALIGN_CENTER);
    // 这个标签设置表情
    lv_obj_set_style_text_font(mid, font_emoji_64_init(), LV_PART_MAIN);
    // 3、液晶显示屏底部部分
    bottom = lv_label_create(screen);
    lv_label_set_text(bottom, "");
    lv_obj_set_align(bottom, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_text_font(bottom, &font_puhui_14_1, LV_PART_MAIN);
    // 退出临界区
    lvgl_port_unlock();
}
// 屏幕上面
void xiaozhi_lvgl_update_top_content(char *text)
{
    lvgl_port_lock(0);
    lv_label_set_text(top, text);
    lvgl_port_unlock();
}
// 屏幕中间
void xiaozhi_lvgl_update_mid_content(char *text)
{
    lvgl_port_lock(0);
    // 屏幕中间放置表情
    for (uint8_t i = 0; i < 21; i++)
    {
        if (strcmp(text, emoji_array[i].name) == 0)
        {
            lv_label_set_text(mid, emoji_array[i].emoji);
        }
    }

    lvgl_port_unlock();
}

// 屏幕下面
void xiaozhi_lvgl_update_bottom_content(char *text)
{
    lvgl_port_lock(0);
    lv_label_set_text(bottom, text);
    lvgl_port_unlock();
}
