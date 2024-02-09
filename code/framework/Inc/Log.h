#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>

#define LOGD(format, ...) printf("[D] " format "\n", ##__VA_ARGS__)
#define LOGI(format, ...) printf("[I] " format "\n", ##__VA_ARGS__)
#define LOGW(format, ...) printf("[W] " format "\n", ##__VA_ARGS__)
#define LOGE(format, ...) printf("[E] " format "\n", ##__VA_ARGS__)

#endif // __LOG_H__
