#include "xiaozhi_emo.h"

/**
 1. 😶 - neutral
2. 🙂 - happy
3. 😆 - laughing
4. 😂 - funny
5. 😔 - sad
6. 😠 - angry
7. 😭 - crying
8. 😍 - loving
9. 😳 - embarrassed
10. 😲 - surprised
11. 😱 - shocked
12. 🤔 - thinking
13. 😉 - winking
14. 😎 - cool
15. 😌 - relaxed
16. 🤤 - delicious
17. 😘 - kissy
18. 😏 - confident
19. 😴 - sleepy
20. 😜 - silly
21. 🙄 - confused
 * **/

// 定义一个结构体数组,数组里面拥有21成员
EMOJI emoji_array[21] = {
    {.name = "neutral",
     .emoji = "😶"},
    {.name = "happy",
     .emoji = "🙂"},
    {.name = "laughing",
     .emoji = "😆"},
    {.name = "funny",
     .emoji = "😂"},
    {.name = "sad",
     .emoji = "😔"},
    {.name = "angry",
     .emoji = "😠"},
    {.name = "crying",
     .emoji = "😭"},
    {.name = "loving",
     .emoji = "😍"},
    {.name = "embarrassed",
     .emoji = "😳"},
    {.name = "surprised",
     .emoji = "😲"},
    {.name = "shocked",
     .emoji = "😱"},
    {.name = "thinking",
     .emoji = "🤔"},
    {.name = "winking",
     .emoji = "😉"},
    {.name = "cool",
     .emoji = "😎"},
    {.name = "relaxed",
     .emoji = "😌"},
    {.name = "delicious",
     .emoji = "🤤"},
    {.name = "kissy",
     .emoji = "😘"},
    {.name = "confident",
     .emoji = "😏"},
    {.name = "sleepy",
     .emoji = "😴"},
    {.name = "silly",
     .emoji = "😜"},
    {.name = "confused",
     .emoji = "🙄"}};