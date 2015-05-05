#ifndef _MEDIA_LOG_H
#define _MEDIA_LOG_H

#include <stdio.h>

// enable log or disable log
#define VERBOSE 1
#define DEBUG 1
#define INFO 1
#define WARN 1
#define ERROR 1

#if VERBOSE
#define LOGV(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#else
#define LOGV(fmt, ...)
#endif

#if DEBUG
#define LOGD(fmt, ...) printf("%s(%d)-<%s>: " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define LOGD(fmt, ...)
#endif

#if INFO
#define LOGI(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#else
#define LOGI(fmt, ...)
#endif

#if WARN
#define LOGW(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#else
#define LOGW(fmt, ...)
#endif

#if ERROR
#define LOGE(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#else
#define LOGE(fmt, ...)
#endif

#endif
