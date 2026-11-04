#include "xiaozhi_websocket.h"
// ws客户端句柄
esp_websocket_client_handle_t ws_client;
// mac地址字符串
extern uint8_t mac_addr_str[18];
// 存储UUID字符串
extern char my_uuid[37];
static char *TAG = "WS";

// 函数:socket客户端请求头携带参数
static void xiaozhi_ws_set_header(void);

// ws客户端事件回调函数
static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id)
    {
        // ws客户端开始连接服务器
    case WEBSOCKET_EVENT_BEGIN:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_BEGIN");
        break;
        // ws客户端与虾哥服务器建立连接成功
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        // 如果客户端长时间不发送消息,客户端会与服务端断开连接
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
        break;
    case WEBSOCKET_EVENT_DATA:
        // 服务器端给ws客户端返回数据[文本、音频]
        // websocket通信时候,服务器端会给客户端返回op_code码:1,表示服务器给咱们返回数据 文本消息 2:表示服务器给咱们返回的时二进制数据[图像、音频、视频]数据!
        ESP_LOGI(TAG, "Received opcode=%d", data->op_code);
        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
        break;
    case WEBSOCKET_EVENT_FINISH:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_FINISH");
        break;
    }
}

// 1.初始化websocket客户端
void xiaozhi_ws_init(void)
{
    // 1.ws客户端相关参数配置
    esp_websocket_client_config_t ws_client_config = {
        .uri = xiaozhi_data.socket_url,             // ws服务器地址
        .transport = WEBSOCKET_TRANSPORT_OVER_SSL,  // 指定WS通信【明文】,还是WSS通信【加密】
        .crt_bundle_attach = esp_crt_bundle_attach, // 乐鑫科技提供证书
        .network_timeout_ms = 10000,                // 网络超时时间
        .reconnect_timeout_ms = 3000,               // 重连时间
    };

    // 2.初始化ws客户端
    ws_client = esp_websocket_client_init(&ws_client_config);

    // 3.注册ws客户端事件回调函数
    esp_websocket_register_events(ws_client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)ws_client);

    // 携带请求头
    xiaozhi_ws_set_header();

    // 4.ws客户端发起请求,要与服务器建立连接
    // 正常来说不应该,初始化立马建立连接!应该SR识别到"唤醒词在建立连接",咱们目前先书写在这里!到时候再说!
    esp_websocket_client_start(ws_client);
}

static void xiaozhi_ws_set_header(void)
{
    // 添加请求头
    esp_websocket_client_append_header(ws_client, "Protocol-Version", "1");
    esp_websocket_client_append_header(ws_client, "Client-Id", my_uuid);
    esp_websocket_client_append_header(ws_client, "Device-Id", (char *)mac_addr_str);

    // 携带:Bearer 服务器获取到token字符串
    uint32_t len = strlen("Bearer ") + strlen(xiaozhi_data.token) + 1;
    // 开辟动态空间存储这个完成字符串
    char *info = heap_caps_malloc(len, MALLOC_CAP_SPIRAM);

    sprintf(info, "Bearer %s", xiaozhi_data.token);
    ESP_LOGE(TAG, "Bearer %s", info);
    esp_websocket_client_append_header(ws_client, "Authorization", info);
}