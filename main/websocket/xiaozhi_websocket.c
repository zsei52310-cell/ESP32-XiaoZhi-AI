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

// 此方法用于ws客户端发送文本消息
void xiaozhi_ws_send_text(char *text, int len);
// 此方法用于ws客户端发送二进制数据
void xiaozhi_ws_send_binary(char *data, int len);

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
        // 当ws客户单连接服务器成功以后,修改事件标志组表示连接成功
        xEventGroupSetBits(xiaozhi_data.event_group, CONNECT_SUCCESS_BIT);
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        // 如果客户端长时间不发送消息,客户端会与服务端断开连接
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
        break;
    case WEBSOCKET_EVENT_DATA:
        // 服务器端给ws客户端返回数据[文本、音频]
        // websocket通信时候,服务器端会给客户端返回op_code码:1,表示服务器给咱们返回数据 文本消息 2:表示服务器给咱们返回的时二进制数据[图像、音频、视频]数据!
        // ESP_LOGI(TAG, "Received opcode=%d", data->op_code);
        // ESP_LOGI(TAG, "Received data len=%s", data->data_ptr);

        // 处理服务器响应文本数据
        if (data->op_code == 1)
        {

            if (xiaozhi_data.ws_text_callback != NULL)
            {
                xiaozhi_data.ws_text_callback(data->data_ptr, data->data_len);
            }
        }

        // 处理服务器返回音频数据
        if (data->op_code == 2)
        {
            if (xiaozhi_data.ws_binary_callback != NULL)
            {
                xiaozhi_data.ws_binary_callback(data->data_ptr, data->data_len);
            }
        }

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

// 此方法用于ws客户端发送文本消息
void xiaozhi_ws_send_text(char *text, int len)
{
    // ws客户端发送文本消息
    esp_websocket_client_send_text(ws_client, text, len, 1000);
}
// 此方法用于ws客户端发送二进制数据
void xiaozhi_ws_send_binary(char *data, int len)
{
    // ws客户端发送二进制数据(音频数据:opus)
    esp_websocket_client_send_bin(ws_client, data, len, 1000);
}

// ws客户端向服务器端发送Hello文本消息
void xiaozhi_ws_send_hello(void)
{

    // 发送文本消息:Hello
    char *hello = "{\"type\":\"hello\",\"version\":1,\"transport\":\"websocket\",\"features\":{\"mcp\":true},\"audio_params\":{\"format\":\"opus\",\"sample_rate\":16000,\"channels\":1,\"frame_duration\":60}}";
    xiaozhi_ws_send_text(hello, strlen(hello));
}
// ws客户端向服务器发送唤醒词命令
void xiaozhi_ws_send_wakeup(void)
{
    char *wakeup = "{\"type\":\"listen\",\"state\":\"detect\",\"text\":\"你好,小智\"}";
    xiaozhi_ws_send_text(wakeup, strlen(wakeup));
}

void xiaozhi_ws_wakeup_connect(void)
{
    // 4.ws客户端发起请求,要与服务器建立连接
    // 正常来说不应该,初始化立马建立连接!应该SR识别到"唤醒词在建立连接",咱们目前先书写在这里!到时候再说!

    // 1.连接服务器
    esp_websocket_client_start(ws_client);

    // 2.等待连接服务器成功以后,再发送Hello消息
    xEventGroupWaitBits(xiaozhi_data.event_group, CONNECT_SUCCESS_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
    xiaozhi_ws_send_hello();

    // 3.等待服务器回复Hello消息以后,确认建立连接成功以后再发送唤醒词进行第一次对话!
    xEventGroupWaitBits(xiaozhi_data.event_group, RESPONSE_HELLO_MSG_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
    xiaozhi_ws_send_wakeup();
}

// 开启监听
void xiaozhi_ws_send_start_listen(void)
{
    char *start_listen = "{\"type\":\"listen\",\"state\":\"start\",\"mode\":\"manual\"}";
    xiaozhi_ws_send_text(start_listen, strlen(start_listen));
}

// 停止监听
void xiaozhi_ws_send_stop_listen(void)
{
    char *stop_listen = "{\"type\":\"listen\",\"state\":\"stop\"}";
    xiaozhi_ws_send_text(stop_listen, strlen(stop_listen));
}

// 终止消息
void xiaozhi_ws_send_abort(void)
{

    // 发送终止命令
    char *abort = "{\"type\":\"abort\",\"reason\":\"wake_word_detected\"}";
    xiaozhi_ws_send_text(abort, strlen(abort));
}