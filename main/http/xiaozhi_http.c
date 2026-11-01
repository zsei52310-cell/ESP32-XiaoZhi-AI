#include "xiaozhi_http.h"
static const char *TAG = "xiaozhi_http";
// HTTP客户端句柄
esp_http_client_handle_t http_client;
// 存储mac地址:六个字节
uint8_t mac[6];
// mac地址字符串
uint8_t mac_addr_str[18];
// 存储UUID字符串
char my_uuid[37];
// 获取UUIDV4字符串方法
void generate_uuid_v4(char *uuid_str)
{
    uint8_t uuid[16];

    // 1. 用硬件随机数填充 16 字节
    esp_fill_random(uuid, sizeof(uuid));

    // 2. 设置版本位为 4（第 6 字节高 4 位）
    uuid[6] = (uuid[6] & 0x0F) | 0x40;

    // 3. 设置变体位为 RFC 4122（第 8 字节高 2 位为 10）
    uuid[8] = (uuid[8] & 0x3F) | 0x80;

    // 4. 格式化为 8-4-4-4-12 标准字符串
    snprintf(uuid_str, 37,
             "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             uuid[0], uuid[1], uuid[2], uuid[3],
             uuid[4], uuid[5], uuid[6], uuid[7],
             uuid[8], uuid[9], uuid[10], uuid[11],
             uuid[12], uuid[13], uuid[14], uuid[15]);
}

// 请求头携带参数
static void http_client_set_header(void);
// 请求体携带参数
static void http_client_set_post(void);

// JSON解析数据
static void http_client_parse_json(char *json_str);

// HTTP请求的回调
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{

    // HTTP客户端发送请求,服务器响应的数据存储到这个缓冲区,并没有指定缓冲区大小！不知道返回数据多少!
    // 到时候需要手动开辟空间
    static char *output_buffer;
    // 存储服务器端返回数据长度
    static int output_len;
    switch (evt->event_id)
    {
        // HTTP客户端发送请求失败:地址写错,WIFI没网等等！
    case HTTP_EVENT_ERROR:
        // ESP_LOGE(TAG, "HTTP请求失败!");
        break;
        // HTTP客户端真连接服务器
    case HTTP_EVENT_ON_CONNECTED:
        // ESP_LOGE(TAG, "HTTP连接虾哥服务器了!");
        break;
        // HTTP客户端发送请求头成功会执行这里
    case HTTP_EVENT_HEADER_SENT:
        // ESP_LOGE(TAG, "HTTP客户端将请求头三个带过去!");
        break;
        // HTTP客户端接收服务器响应头的数据会执行这里
    case HTTP_EVENT_ON_HEADER:
        // ESP_LOGE(TAG, "虾哥服务器返回响应头的数据 key=%s ---- value=%s", evt->header_key, evt->header_value);
        if (strcmp(evt->header_key, "Content-Length") == 0)
        {
            // output_buffer:开辟586+1,服务器返回数据个数总和
            output_buffer = heap_caps_malloc(atoi(evt->header_value) + 1, MALLOC_CAP_SPIRAM);
            if (output_buffer == NULL)
            {
                ESP_LOGI(TAG, "开辟内存失败!");
            }
            else
            {
                ESP_LOGI(TAG, "开辟内存成功! %d", atoi(evt->header_value) + 1);
            }
        }
        break;
        // HTTP客户端实时响应的数据
    case HTTP_EVENT_ON_DATA:
        // ESP_LOGE(TAG, "虾哥服务器返回实时数据:len=%d,val=%s", evt->data_len, (char *)evt->data);
        // 服务器返回数据:分批次返回!将分批次数据统一全部放到缓冲区里面
        memcpy(output_buffer + output_len, evt->data, evt->data_len);
        output_len += evt->data_len;
        break;
        // 客户端与服务器通信结束
    case HTTP_EVENT_ON_FINISH:
        // 解析JSON数据
        http_client_parse_json(output_buffer);
        if (output_buffer != NULL)
        {
            heap_caps_free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
        break;
        // 客户端与服务器断开连接
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGE(TAG, "HTTP_EVENT_DISCONNECTED");
        int mbedtls_err = 0;
        esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
        if (err != 0)
        {
            ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
            ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
        }
        if (output_buffer != NULL)
        {
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
        esp_http_client_set_header(evt->client, "From", "user@example.com");
        esp_http_client_set_header(evt->client, "Accept", "text/html");
        esp_http_client_set_redirection(evt->client);
        break;
    }
    return ESP_OK;
}

// 1.初始化方法
void xiaozhi_http_client_init(void)
{

    // 1.HTTP客户端参数
    esp_http_client_config_t client_config_params = {
        .host = "api.tenclass.net",                 // 虾哥他们HTTP服务器域名
        .path = "/xiaozhi/ota/",                    // 请求路径
        .method = HTTP_METHOD_POST,                 // 请求方法
        .transport_type = HTTP_TRANSPORT_OVER_SSL,  // 传输协议:HTTP_TRANSPORT_OVER_SSL HTTPS加密传输    HTTP_TRANSPORT_OVER_TCP HTTP明文传输数据没有进行加密
        .crt_bundle_attach = esp_crt_bundle_attach, // 证书:乐鑫科技提供证书
        .event_handler = _http_event_handler        // HTTP回调
    };

    // 2.创建HTTP客户端
    http_client = esp_http_client_init(&client_config_params);

    // 携带参数
    http_client_set_header();
    http_client_set_post();

    // 3.发送HTTP请求
    // 放置wifi网络不好,可以请求一次不成功,多次请求
    while (esp_http_client_perform(http_client) != ESP_OK)
    {
        // 网络不好持续发送HTTP请求,知道请求成功结束
        xiaozhi_lvgl_update_top_content("大哥,咱们换个运营商");
        xiaozhi_lvgl_update_mid_content("funny");
        xiaozhi_lvgl_update_bottom_content("网络太差,请检测网络信号");
    }
}

// 请求头携带参数
static void http_client_set_header(void)
{
    // 携带请求头参数
    // 客户端的名字和版本号
    esp_http_client_set_header(http_client, "User-Agent", "bread-compact-wifi-128x64/1.0.1");

    // 客户端唯一的表示:MAC地址进行拼接携带过去
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    sprintf((char *)mac_addr_str, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    esp_http_client_set_header(http_client, "Device-Id", (char *)mac_addr_str);

    // 客户端唯一标识符
    generate_uuid_v4(my_uuid);
    esp_http_client_set_header(http_client, "Client-Id", my_uuid);
}
// 请求体携带参数
static void http_client_set_post(void)
{
    char *body_params = "{\"application\":{\"version\":\"1.0.1\",\"elf_sha256\":\"c8a8ecb6d6fbcda682494d9675cd1ead240ecf38bdde75282a42365a0e396033\"},\"board\":{\"type\":\"bread-compact-wifi\",\"name\":\"bread-compact-wifi-128x64\",\"ssid\":\"卧室\",\"rssi\":-55,\"channel\":1,\"ip\":\"192.168.1.11\",\"mac\":\"11:22:33:44:55:66\"}}";

    // 请求体携带参数
    esp_http_client_set_post_field(http_client, body_params, strlen(body_params));
}

// JSON解析
static void http_client_parse_json(char *json_str)
{
    // 1.将JSON形式字符串,转换为CJSON结构体
    cJSON *root = cJSON_Parse(json_str);
    // 解析服务器信息字段websocket
    cJSON *websocket = cJSON_GetObjectItem(root, "websocket");
    cJSON *activation = cJSON_GetObjectItem(root, "activation");
    if (websocket != NULL)
    {
        // 获取socket服务器地址
        char *url = cJSON_GetObjectItem(websocket, "url")->valuestring;
        // token,令牌,身份唯一表示
        char *token = cJSON_GetObjectItem(websocket, "token")->valuestring;
        // 存储服务器信息
        xiaozhi_data.socket_url = url;
        xiaozhi_data.token = token;
        ESP_LOGE(TAG, "url:%s,token:%s", xiaozhi_data.socket_url, xiaozhi_data.token);
    }

    if (activation != NULL)
    {
        // 当前设备没有激活,拥有激活码
        char *code = cJSON_GetObjectItem(activation, "code")->valuestring;
        char info[50] = {0};
        sprintf(info, "激活码:%s", code);

        xiaozhi_lvgl_update_top_content(info);
        xiaozhi_lvgl_update_mid_content("confident");
        xiaozhi_lvgl_update_bottom_content("激活智能体以后,重启设备!");
    }
    else
    {
        // 当前设备已经激活
        xiaozhi_lvgl_update_top_content("尚硅谷AI小智");
        xiaozhi_lvgl_update_mid_content("kissy");
        xiaozhi_lvgl_update_bottom_content("你好呀,我是小智");
    }
}