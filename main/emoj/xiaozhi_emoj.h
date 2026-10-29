#ifndef __XIAOZHI_EMOJ_H__
#define __XIAOZHI_EMOJ_H__

//用一个结构体表示表情
typedef struct 
{
    char *name;
    char *emoji;
}EMOJI;

//用结构体变量创造的结构体数组需要对外暴露,所以这个地方在头文件里加了extern
extern EMOJI emoji_array[21];

#endif /* __XIAOZHI_EMOJ_H__ */
