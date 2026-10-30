#ifndef __XIAOZHI_EMO_H__
#define __XIAOZHI_EMO_H__

// 用一个结构体表示表情:21
typedef struct
{
   char * name;
   char * emoji;
} EMOJI;

extern EMOJI emoji_array[21];

#endif /* __XIAOZHI_EMO_H__ */