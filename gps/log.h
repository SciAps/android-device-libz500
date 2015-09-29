#ifndef GPS_LOG_H_
#define GPS_LOG_H_

#define  LOG_TAG  "gps_ksp5012"
#include <cutils/log.h>

#ifdef DEBUG_TRACE
# define TRACE(format, ...) ALOGD("[%s " __FILE__ ":%d] " format, __func__, __LINE__, ##__VA_ARGS__)
#else
# define TRACE(format, ...)
#endif


#endif //GPS_LOG_H_