#include "xiaozhi_http.h"
static const char *TAG = "xiaozhi_http";
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
        ESP_LOGE(TAG, "HTTP请求失败!");
        break;
        // HTTP客户端真连接服务器
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGE(TAG, "HTTP连接虾哥服务器了!");
        break;
        // HTTP客户端发送请求头成功会执行这里
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGE(TAG, "HTTP客户端将请求头三个带过去!");
        break;
        // HTTP客户端接收服务器响应头的数据会执行这里
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGE(TAG, "虾哥服务器返回响应头的数据 key=%s ---- value=%s", evt->header_key, evt->header_value);
        break;
        // HTTP客户端实时响应的数据
    case HTTP_EVENT_ON_DATA:
        ESP_LOGE(TAG, "虾哥服务器返回实时数据:len=%d,val=%s", evt->data_len, (char *)evt->data);
        break;
        // 客户端与服务器通信结束
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGE(TAG, "HTTP_EVENT_ON_FINISH");
        if (output_buffer != NULL)
        {
            free(output_buffer);
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
    esp_http_client_handle_t http_client = esp_http_client_init(&client_config_params);

    // 3.发送HTTP请求
    esp_http_client_perform(http_client);
}