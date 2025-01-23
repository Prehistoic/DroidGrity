#ifdef ENABLE_LOGS

#include <android/log.h>

#define LOG_TAG "DROIDGRITY"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#else

#define LOGD(...) // No-op
#define LOGI(...) // No-op
#define LOGW(...) // No-op
#define LOGE(...) // No-op

#endif