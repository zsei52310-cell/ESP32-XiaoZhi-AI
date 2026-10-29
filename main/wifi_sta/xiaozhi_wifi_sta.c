#include "xiaozhi_wifi_sta.h"

// 事件标志组句柄
static EventGroupHandle_t s_wifi_event_group;
static const char *TAG = "wifi station";
// WIFI重连次数
static int s_retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        // 如果当前WIFI驱动处于STA模式,开始连接路由器
        esp_wifi_connect();
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        // 如果没有连上热点,重连五次
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            // 事件标志组:连接五次都没连接上,连接失败!!!!!
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        // 设备已经获取到IP地址,连接上热点,把获取到IP地址打印出来
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }

    /* ************************蓝牙配网新增事件处理************************************************************************************************ */
     if (event_base == WIFI_PROV_EVENT)
    {
        switch (event_id)
        {
        case WIFI_PROV_START:
            // 开始蓝牙配网
            ESP_LOGI(TAG, "Provisioning started");
            break;
        case WIFI_PROV_CRED_RECV:
        {
            // 蓝牙配网:ESP32主控蓝夜收到账号与密码执行这里
            wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
            ESP_LOGI(TAG, "Received Wi-Fi credentials"
                          "\n\tSSID     : %s\n\tPassword : %s",
                     (const char *)wifi_sta_cfg->ssid,
                     (const char *)wifi_sta_cfg->password);
            break;
        }
        case WIFI_PROV_CRED_FAIL:
        {
            // 蓝牙配网失败
            wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
            ESP_LOGE(TAG, "Provisioning failed!\n\tReason : %s"
                          "\n\tPlease reset to factory and retry provisioning",
                     (*reason == WIFI_PROV_STA_AUTH_ERROR) ? "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");
            break;
        }
        case WIFI_PROV_CRED_SUCCESS:
            // 蓝牙配网成功
            ESP_LOGI(TAG, "Provisioning successful");
            //配网成功之后，删除二维码
            xiaozhi_lvgl_del_qr_code();
            //删除顶部配网提示文字
            xiaozhi_lvgl_update_top_content("");
            break;
        case WIFI_PROV_END:
            // 蓝牙配网成功以后,释放蓝牙资源释放!!!!!!!
            wifi_prov_mgr_deinit();
            break;
        default:
            break;
        }
    }
    /* ******************************************************************************************************************************************** */

    /* *****************************新增的回调函数的处理************************************************************************************************************** */
        if (event_base == PROTOCOMM_SECURITY_SESSION_EVENT)
    {
        switch (event_id)
        {
        case PROTOCOMM_SECURITY_SESSION_SETUP_OK:
            ESP_LOGI(TAG, "Secured session established!");
            break;
        case PROTOCOMM_SECURITY_SESSION_INVALID_SECURITY_PARAMS:
            ESP_LOGE(TAG, "Received invalid security parameters for establishing secure session!");
            break;
        case PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH:
            ESP_LOGE(TAG, "Received incorrect username and/or PoP for establishing secure session!");
            break;
        default:
            break;
        }
    }

    /* ************************************************************************************************************************************************* */
}


/* ****************新增的，用于获取蓝牙的名字******************************************************************************************* */
// 获取ESP32开启蓝牙名字
static void get_device_service_name(char *service_name, size_t max)
{
    // 获取当前设备MAC地址
    uint8_t eth_mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    const char *ssid_prefix = "sz--sz";
    snprintf(service_name, max, "%s%02X%02X%02X",
             ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
}

/* ************************************************************************************************************************************ */


/* ***********新增的，用于生成二维码*************************************************************************************************** */
//本质向在控制台生成一个二维码图像
static void wifi_prov_print_qr(const char *name, const char *username, const char *pop, const char *transport)
{
    if (!name || !transport)
    {
        ESP_LOGW(TAG, "Cannot generate QR code payload. Data missing.");
        return;
    }
    char payload[150] = {0};
    if (pop)
    {

        snprintf(payload, sizeof(payload), "{\"ver\":\"%s\",\"name\":\"%s\""
                                           ",\"pop\":\"%s\",\"transport\":\"%s\"}",
                 PROV_QR_VERSION, name, pop, transport);
    }
    else
    {
        snprintf(payload, sizeof(payload), "{\"ver\":\"%s\",\"name\":\"%s\""
                                           ",\"transport\":\"%s\"}",
                 PROV_QR_VERSION, name, transport);
    }

    ESP_LOGI(TAG, "Scan this QR code from the provisioning application for Provisioning.");
    //底下这两行代码:起始使用组件qrcode,专门用来生成二维码
    // esp_qrcode_config_t cfg = ESP_QRCODE_CONFIG_DEFAULT();
    // esp_qrcode_generate(&cfg, payload);
    //上面注释掉的这2行是把二维码生成到控制台
    //下面需要把配网的二维码生成到屏幕
    xiaozhi_lvgl_update_top_content("请扫描二维码,给奶龙连接wifi");
    xiaozhi_lvgl_show_qr_code(payload);
  
}



/* ************************************************************************************************************* */



// 2.初始化WIFI+STA模式工作
void wifi_init_sta(void)
{
    // 1.创建事件标志组
    s_wifi_event_group = xEventGroupCreate();
    // 2.WIFI相关网络层协议:TCP、IP协议栈
    esp_netif_init();
    // 3.创建事件循环:底层源码,创建任务优先级20的,内核0持续执行的任务!
    // 设置是否连上用户热点、设备是否拥有IP、底层源码要持续判断,因此搞了一个任务持续判断!
    esp_event_loop_create_default();

    // 4.初始化WIFI驱动 + STA模式,可以连接热点能力!!!!!
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    // WIFI驱动注册事件:当某一个事件触发会执行对应回调函数
    // 连接路由器、IP相关事件触发,会执行event_handler
    esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &event_handler,
                                        NULL,
                                        &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT,
                                        IP_EVENT_STA_GOT_IP,
                                        &event_handler,
                                        NULL,
                                        &instance_got_ip);

/* **********************新增蓝牙配网事件三个************************************************************************************************ */
    esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);
    esp_event_handler_register(PROTOCOMM_TRANSPORT_BLE_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);
    esp_event_handler_register(PROTOCOMM_SECURITY_SESSION_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);
/* ******************************************************************************************************************************************** */

/* *********************新增的配网的功能********************************************************************************************************************** */
// 配网相关配置
    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_ble,                                       // 配网模式选择:BLE配网
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM, // 蓝牙配网完成以后,将ESP32蓝牙关闭!
    };
    // 初始化蓝牙配网
    wifi_prov_mgr_init(config);

    // 查看当前设备是否陪过网
    bool provisioned = false;
    wifi_prov_mgr_is_provisioned(&provisioned);

    // 未配网
    if (!provisioned)
    {
        ESP_LOGI(TAG, "Starting provisioning");
        // 蓝牙配网:组装ESP32开启蓝牙名字:蓝牙名字 prov_mac地址
        char service_name[12];
        get_device_service_name(service_name, sizeof(service_name));

        // 蓝牙配网时候,蓝牙技术传输数据使用加密算法
        // 加密算法1
        wifi_prov_security_t security = WIFI_PROV_SECURITY_1;
        const char *pop = "abcd1234";
        wifi_prov_security1_params_t *sec_params = pop;
        const char *username = NULL;
        const char *service_key = NULL;

        // 蓝牙配网UUID
        uint8_t custom_service_uuid[] = {
            /* LSB <---------------------------------------
             * ---------------------------------------> MSB */
            0xb4,
            0xdf,
            0x5a,
            0x1c,
            0x3f,
            0x6b,
            0xf4,
            0xbf,
            0xea,
            0x4a,
            0x82,
            0x03,
            0x04,
            0x90,
            0x1a,
            0x02,
        };
        wifi_prov_scheme_ble_set_service_uuid(custom_service_uuid);

        // 蓝牙配网:算法1
        wifi_prov_mgr_start_provisioning(security, (const void *)sec_params, service_name, service_key);
        //在控制台输出一个二维码图像,用于配网
        wifi_prov_print_qr(service_name, username, pop, PROV_TRANSPORT_BLE);
    }
    else
    {
        ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi STA");
        // 既然配过网,没必要在使用蓝牙资源,蓝牙资源关闭!
        wifi_prov_mgr_deinit();

/* ****************这个是新增的，原来的注释掉了************************************************************************************ */        
        // 直接连接热点
        // wifi_init_sta();
    // WIFI驱动处于STA模式,主动连接热点
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();


/* ********************************************************************************************************** */
    }
/* ******************************************************************************************************************************************* */
    //蓝牙配网这个不需要
    /* //5.配置连接热点参数
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,//连接热点名字
            .password = EXAMPLE_ESP_WIFI_PASS,//连接热点密码
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
            .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
        },
    };
    //设置WIFI驱动工作模式
    esp_wifi_set_mode(WIFI_MODE_STA);
    //设置WIFI STA模式连接热点参数
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    //开始连接热点
    esp_wifi_start(); */

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

//初始化
void xiaozhi_wifi_sta_init(void)
{

    // 1.初始化FLASH:持久化保存热点:名字、密码
    esp_err_t ret = nvs_flash_init();
    // 2.flash初始化失败判断
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // 将flash擦除
        nvs_flash_erase();
        // 重新初始化
        nvs_flash_init();
    }

    // ESP32 WIFI驱动:STA、AP
    wifi_init_sta();
}

/* *************新增的函数**************** */
void xiaozhi_wifi_sta_reset(void){
    //擦除掉flash wifi账号和密码
    esp_wifi_restore();
    //软件复位，没配网的配网
    esp_restart();

}
/* ****************************************** */
