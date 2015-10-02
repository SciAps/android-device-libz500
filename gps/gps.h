#ifndef GPS_H_
#define GPS_H_

#include <utils/Mutex.h>
#include <utils/KeyedVector.h>
#include <hardware/gps.h>

namespace gps {

extern WorkQueue* sGpsWorkQueue;
extern GpsCallbacks* sAndroidCallbacks;

extern GpsSvStatus sGpsSvStatus;

class GpsState {
public:

class SpaceVehicle {
	public:
	SpaceVehicle();
	SpaceVehicle(const GpsSvInfo&);
	
	GpsSvInfo mInfo;
	bool mUsedInFix;
};


	void updateSV(const GpsSvInfo& sv);
	GpsSvStatus buildGpsSvStatus();
	
private:
	Mutex mLock;
	KeyedVector<int, SpaceVehicle> mSpaceVehicles;
};

extern GpsState sGpsState;


class SVWorkUnit : public WorkQueue::WorkUnit {
  private:
    GpsSvStatus mSvStatus;
  
  public:
    SVWorkUnit(const GpsSvStatus& status)
    : mSvStatus(status) { }
  
    virtual void run() {
      sAndroidCallbacks->sv_status_cb(&mSvStatus);
    }
};

class GpsStatusWorkUnit : public WorkQueue::WorkUnit {
	private:
	GpsStatus mStatus;
	
	public:
	GpsStatusWorkUnit(const GpsStatus& status)
	: mStatus(status) { }
	
	virtual void run() {
		sAndroidCallbacks->status_cb(&mStatus);
	}
	
};


} // namespace gps


#endif // GPS_H_