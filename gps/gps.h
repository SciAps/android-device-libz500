#ifndef GPS_H_
#define GPS_H_

#include <hardware/gps.h>

namespace gps {

extern WorkQueue* sGpsWorkQueue;
extern GpsCallbacks* sAndroidCallbacks;

extern GpsSvStatus sGpsSvStatus;


} // namespace gps


#endif // GPS_H_