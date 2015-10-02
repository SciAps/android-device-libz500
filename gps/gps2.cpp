#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <termios.h>

#include "log.h"

#include "gps_parser.h"
#include "workqueue.h"
#include "gps.h"

#include <hardware/gps.h>

namespace gps {

#define	JF2_PULSE_HIGH	1
#define	JF2_PULSE_LOW	0

#define	KSP5012_CHANNEL_NAME  "/dev/ttyO0"
const char* GPS_SYSTEM_ON_FILE = "/sys/class/gpio/gpio177/value";
const char* GPS_ONOFF_FILE = "/sys/class/gpio/gpio172/value";

class ReportWorkUnit : public WorkQueue::WorkUnit {
  
  public:
    ReportWorkUnit(long delay)
    : WorkUnit(delay) { }
    
    virtual void run() {
      ALOGD("sending SV update");
      GpsSvStatus status = sGpsState.buildGpsSvStatus();
      sGpsWorkQueue->schedule(new SVWorkUnit(status));
    }
};

ReportWorkUnit* sReportTask = NULL;

static GpsParser* sGpsParser = NULL;
WorkQueue* sGpsWorkQueue = NULL;
WorkQueue* TimerWorkQueue = NULL;
GpsCallbacks* sAndroidCallbacks;
GpsSvStatus sGpsSvStatus;
GpsState sGpsState;
GpsStatus sStatus;

GpsState::SpaceVehicle::SpaceVehicle() { }
GpsState::SpaceVehicle::SpaceVehicle(const GpsSvInfo& sv)
 : mInfo(sv) { }

void GpsState::updateSV(const GpsSvInfo& sv) {
  AutoMutex l(mLock);
  ssize_t i = mSpaceVehicles.indexOfKey(sv.prn);
  if(i < 0) {
    i = mSpaceVehicles.add(sv.prn, SpaceVehicle(sv));
  } else {
	  mSpaceVehicles.editValueAt(i).mInfo = sv;
  }
}

GpsSvStatus GpsState::buildGpsSvStatus() {
  AutoMutex l(mLock);
  GpsSvStatus status;
  status.size = sizeof(GpsSvStatus);
  status.num_svs = mSpaceVehicles.size();
  status.ephemeris_mask = 0;
  status.almanac_mask = 0;
  status.used_in_fix_mask = 0;

  for(size_t i=0; i<mSpaceVehicles.size(); i++) {
    status.sv_list[i] = mSpaceVehicles.valueAt(i).mInfo;
  }
  
  mSpaceVehicles.clear();
  
  return status;
}



static
int read_int(const char* path) {
  //char path[VALUE_MAX];
  char value_str[3];
  int fd;

  //snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
  fd = open(path, O_RDONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio value for reading!\n");
    return(-1);
  }

  if (-1 == read(fd, value_str, 3)) {
    fprintf(stderr, "Failed to read value!\n");
    return(-1);
  }

  close(fd);

  return(atoi(value_str));
}

static
int write_int(char const* path, int value) {
  int fd;
  static int already_warned;

  fd = open(path, O_RDWR);
  if (fd >= 0) {
    char buffer[20];
    int bytes = sprintf(buffer, "%d\n", value);
    int amt = write(fd, buffer, bytes);
    close(fd);

    return amt == -1 ? -errno : 0;
  } else {
    if (already_warned == 0) {
      ALOGE("write_int failed to open %s\n", path);
      already_warned = 1;
    }

    return -errno;
  }
}

static void toggle_power() {
  write_int(GPS_ONOFF_FILE, JF2_PULSE_HIGH);
  usleep(110);
  write_int(GPS_ONOFF_FILE, JF2_PULSE_LOW);

  usleep(1000*1000);
}

static
void eventThreadStart(void* arg) {
  ALOGD("start event loop...");
  ((WorkQueue*)arg)->runLoop();
  ALOGD("exit event loop");
}

static
void timerThreadStart(void* arg) {
  ALOGD("start timer loop...");
  ((WorkQueue*)arg)->runLoop();
  ALOGD("exit timer loop");
}

static
void reportStatus() {
  sGpsWorkQueue->schedule(new GpsStatusWorkUnit(sStatus));
}

static
int ksp5012_gps_init(GpsCallbacks* callbacks) {
  TRACE();
  
  sStatus.size = sizeof(GpsStatus);

  memset(&sGpsSvStatus, 0, sizeof(GpsSvStatus));
  sGpsSvStatus.size = sizeof(GpsSvStatus);

  sAndroidCallbacks = callbacks;

  sGpsWorkQueue = new WorkQueue();
  sAndroidCallbacks->create_thread_cb("callback thread", eventThreadStart, sGpsWorkQueue);
  
  TimerWorkQueue = new WorkQueue();
  sAndroidCallbacks->create_thread_cb("timer thread", timerThreadStart, TimerWorkQueue);

  sGpsParser = new GpsParser();

  int uart_fd = open(KSP5012_CHANNEL_NAME, O_RDWR);
  if(uart_fd < 0) {
    ALOGE("could not open %s", KSP5012_CHANNEL_NAME);
    goto exit_fail;
  }

  struct termios options;
  //if(tcgetattr(uart_fd, &options) < 0) {
  //	ALOGE("could not get terminal options");
  //	goto exit_fail;
  //}

  cfmakeraw(&options);
  options.c_cflag |= (CLOCAL | CREAD | B9600 | CS8 | CSTOPB);
  options.c_cc[VMIN] = 1;
  options.c_cc[VTIME] = 0;

  if(tcsetattr(uart_fd, TCSANOW, &options) < 0) {
    ALOGE("could not set terminal options");
  }

  sGpsParser->setFD(uart_fd);
  sGpsParser->start();
  
  /*
  {
  int power = read_int(GPS_SYSTEM_ON_FILE);
  sStatus.status = power == 1 ? GPS_STATUS_ENGINE_ON : GPS_STATUS_ENGINE_OFF;
  reportStatus();
  }
  */

  return 0;

exit_fail:
  if(uart_fd >= 0) {
    close(uart_fd);
  }
  return -1;
}

static
void ksp5012_gps_cleanup() {
  TRACE();
  
  if(TimerWorkQueue != NULL) {
    TimerWorkQueue->cancel();
    TimerWorkQueue->finish();
    delete TimerWorkQueue;
    TimerWorkQueue = NULL;
  }
  
  if(sGpsWorkQueue != NULL) {
    sGpsWorkQueue->cancel();
    sGpsWorkQueue->finish();
    delete sGpsWorkQueue;
    sGpsWorkQueue = NULL;
  }

  if(sGpsParser != NULL) {
    sGpsParser->stop();
    delete sGpsParser;
    sGpsParser = NULL;
  }
  
  TRACE();

}

static
int ksp5012_gps_start() {
  TRACE();

  int power = read_int(GPS_SYSTEM_ON_FILE);

  ALOGI("gps power is: %d", power);
  if(power != 1) {
    ALOGI("turning on...");
    toggle_power();
  }

  return 0;
}

static
int ksp5012_gps_stop() {
  TRACE();
  
  if(sReportTask != NULL) {
    TimerWorkQueue->cancel(sReportTask);
    sReportTask = NULL;
  }
  
  int power = read_int(GPS_SYSTEM_ON_FILE);

  ALOGI("gps power is: %d", power);
  if(power == 1) {
    ALOGI("turning off...");
    toggle_power();
  }
  
  return 0;
}


static
int ksp5012_gps_inject_time(GpsUtcTime time, int64_t timeReference, int uncertanty) {
  TRACE();
  return 0;
}

static
int ksp5012_gps_inject_location(double latitude, double longitude, float accuracty) {
  TRACE();
  return 0;
}

static
void ksp5012_gps_delete_aiding_data(GpsAidingData flags) {
  TRACE();

}

static
int ksp5012_gps_set_position_mode(GpsPositionMode mode, GpsPositionRecurrence recurrence,
                                  uint32_t min_interval, uint32_t preferred_accuracy, uint32_t preferred_time) {
  TRACE("GpsPositionMode: %d GpsPositionRecurrence: %d min_interval: %d preferred_accuracy: %d preferred_time: %d",
    mode, recurrence, min_interval, preferred_accuracy, preferred_time);
  
  ALOGD("scheduling interval for: %d", min_interval);
  if(sReportTask != NULL) {
    TimerWorkQueue->cancel(sReportTask);
  }
  sReportTask = new ReportWorkUnit(2*min_interval);
  TimerWorkQueue->schedule(sReportTask);

  return 0;
}

static
const void* ksp5012_gps_get_extension(const char* name) {
  TRACE("name: %s", name);

  return NULL;
}


static const GpsInterface ksp5012GpsInterface = {
  sizeof(GpsInterface),
  ksp5012_gps_init,
  ksp5012_gps_start,
  ksp5012_gps_stop,
  ksp5012_gps_cleanup,
  ksp5012_gps_inject_time,
  ksp5012_gps_inject_location,
  ksp5012_gps_delete_aiding_data,
  ksp5012_gps_set_position_mode,
  ksp5012_gps_get_extension,
};

static const GpsInterface* ksp5012_get_gps_interface(struct gps_device_t* dev) {
  return &ksp5012GpsInterface;
}


} // namespace gps

extern "C" {

  int open_gps(const struct hw_module_t* module, char const* name, struct hw_device_t** device) {

    struct gps_device_t* dev = (struct gps_device_t*)malloc(sizeof(struct gps_device_t));
    memset(dev, 0, sizeof(*dev));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*)module;
    dev->get_gps_interface = gps::ksp5012_get_gps_interface;

    *device = (struct hw_device_t*)dev;
    return 0;
  }

  struct hw_module_methods_t gps_module_methods = {
open:
    open_gps
  };

  struct hw_module_t HAL_MODULE_INFO_SYM = {
tag:
    HARDWARE_MODULE_TAG,
    version_major: 1,
    version_minor: 0,
id:
    GPS_HARDWARE_MODULE_ID,
name: "KSP5012 JF2 GPS Module v2"
    ,
author: "The Android Open Source Project"
    ,
methods:
    & gps_module_methods,
dso:
    NULL,
reserved:
    {0}
  };

}; // extern "C"