#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>

#define LOG_TAG "DefaultTag"

#ifdef NDEBUG
#define LOGD(format, ...)
#else
#define LOGD(format, ...) printf("[D] " LOG_TAG ": " format "\n", ##__VA_ARGS__)
#endif

#define LOGI(format, ...) printf("[I] " LOG_TAG ": " format "\n", ##__VA_ARGS__)
#define LOGW(format, ...) printf("[W] " LOG_TAG ": " format "\n", ##__VA_ARGS__)
#define LOGE(format, ...) printf("[E] " LOG_TAG ": " format "\n", ##__VA_ARGS__)

#define LOGI_LIST(head, list)							\
printf("[D] " LOG_TAG ": " head "\n");					\
for (int i = 0; i < list.size(); i++) {					\
	std::string item(list[i]);							\
	printf("[D] " LOG_TAG ":   - %s\n", item.c_str());	\
}														\
printf("[D] " LOG_TAG ":   - end\n");

#define LOGI_LIST(head, list)							\
printf("[I] " LOG_TAG ": " head "\n");					\
for (int i = 0; i < list.size(); i++) {					\
	std::string item(list[i]);							\
	printf("[I] " LOG_TAG ":   - %s\n", item.c_str());	\
}														\
printf("[I] " LOG_TAG ":   - end\n");

#define LOGW_LIST(head, list)							\
printf("[W] " LOG_TAG ": " head "\n");					\
for (int i = 0; i < list.size(); i++) {					\
	std::string item(list[i]);							\
	printf("[W] " LOG_TAG ":   - %s\n", item.c_str());	\
}														\
printf("[W] " LOG_TAG ":   - end\n");

#define LOGE_LIST(head, list)							\
printf("[E] " LOG_TAG ": " head "\n");					\
for (int i = 0; i < list.size(); i++) {					\
	std::string item(list[i]);							\
	printf("[E] " LOG_TAG ":   - %s\n", item.c_str());	\
}														\
printf("[E] " LOG_TAG ":   - end\n");

#endif // __LOG_H__
