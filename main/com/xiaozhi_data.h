#ifndef __XIAOZHI_DATA_H__
#define __XIAOZHI_DATA_H__

typedef struct
{
    // socket服务器信息
    char *socket_url;
    // token
    char *token;
} xiaozhi_data_t;

extern xiaozhi_data_t xiaozhi_data;
#endif /* __XIAOZHI_DATA_H__ */