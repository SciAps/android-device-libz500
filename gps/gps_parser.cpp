
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "log.h"
#include <utils/String8.h>
#include <utils/KeyedVector.h>


#include "gps_parser.h"
#include "workqueue.h"
#include "gps.h"



int nmea_lex(void*);


namespace gps {

#define IDLE 0
#define RUNNING 1
#define SHUTTING_DOWN 2

class ParserThread : public Thread {
  GpsParser* mParser;

public:
  ParserThread(GpsParser* parser)
    : Thread(false), mParser(parser) { }

  virtual ~ParserThread() { }

private:
  virtual bool threadLoop() {
    ALOGI("starting gps parser thread...");

    mParser->mState = RUNNING;
    mParser->init_scanner();

    int value;
    while(!exitPending()) {
      value = nmea_lex(mParser->mScanner);
      ALOGI("nmea_lex returned: %d", value);
      if(value == 0) {
        break;
      }
    }

    mParser->destroy_scanner();

    ALOGI("exiting gps parser thread");

    return false;
  }

};

GpsParser::GpsParser()
  : mState(IDLE), mFD(-1) { }

GpsParser::~GpsParser() {
  if(mFD >= 0) {
    close(mFD);
  }
}

void GpsParser::setFD(int fd) {
  mFD = fd;
}

int GpsParser::readBytes(char* buf, const unsigned int maxBytes) {
  TRACE("maxBytes: %d", maxBytes);
  if(mState == SHUTTING_DOWN) {
    return 0;
  }
  int bytesRead = read(mFD, buf, maxBytes);
  if(bytesRead < 0) {
    return 0;
  }
  return bytesRead;
}

void GpsParser::start() {
  mRecieveThread = new ParserThread(this);
  mRecieveThread->run("GPS Parser");
}

void GpsParser::stop() {
  mState = SHUTTING_DOWN;
  status_t result = mRecieveThread->requestExitAndWait();
  if (result) {
    ALOGW("could not stop parser thread. Error: %d", result);
  }
  ALOGI("parser stopped");
}

class SVWorkUnit : public WorkQueue::WorkUnit {
private:
  GpsSvStatus mSvStatus;

public:
  SVWorkUnit(const GpsSvStatus& status)
    : mSvStatus(status) { }

  virtual ~SVWorkUnit() {}

  virtual void run() {
    sAndroidCallbacks->sv_status_cb(&mSvStatus);
  }


};

static Vector<Slice> split(const Slice& data) {
  Vector<Slice> retval;

  char* ptr = data.ptr;
  size_t size = 0;

  for(size_t i=0; i<data.size; i++) {
    if(data.ptr[i] == ',') {
      retval.add(Slice(ptr, size));
      ptr = &data.ptr[i+1];
      size = 0;
    } else {
      size++;
    }
  }
  retval.add(Slice(ptr, size));

  return retval;
}

class NmeaSentence {

public:
  NmeaSentence(const Slice& data) {
    mData = new char[data.size+1];
    memcpy(mData, data.ptr, data.size);
    mData[data.size] = 0;

    char* ptr = mData;
    size_t size = 0;

    for(size_t i=0; i<data.size; i++) {
      if(data.ptr[i] == ',') {
        mData[i] = 0;
        mValues.add(Slice(ptr, size));
        ptr = &mData[i+1];
        size = 0;
      } else {
        size++;
      }
    }

    mValues.add(Slice(ptr, size));
  }

  ~NmeaSentence() {
    if(mData != NULL) {
      delete mData;
    }
  }

  const Slice& get(int i) {
    return mValues[i];
  }

  int asInt(int i) {
    const Slice& value = get(i);
    return atoi(value.ptr);
  }

  double asFloat(int i) {
    const Slice& value = get(i);
    return strtod(value.ptr, NULL);
  }

  const char* asStr(int i) {
    return get(i).ptr;
  }

  bool isNull(int i) {
    const Slice& value = get(i);
    return value.size == 0;
  }

  int size() {
    return mValues.size();
  }

private:
  char* mData;
  Vector<Slice> mValues;

};

//static Vector<GpsSvInfo> sSv;
static KeyedVector<int, GpsSvInfo> sSv;

static void updateSV(const GpsSvInfo& sv) {
  ssize_t i = sSv.indexOfKey(sv.prn);
  if(i < 0) {
    i = sSv.add(sv.prn, sv);
  }

  
}

#define MAX_NMEA_SENTENCE 80

void GpsParser::emitNMEASentence(const Slice& data) {

  {
    char sentence[MAX_NMEA_SENTENCE+1];
    memset(sentence, 0, MAX_NMEA_SENTENCE+1);
    memcpy(sentence, data.ptr, data.size);
    ALOGD("NMEA sentence %s", sentence);
  }


  NmeaSentence sentance(data);

  if(strcmp("GPGSV", sentance.asStr(0)) == 0) {

    if(sentance.asInt(2) == 1) {
      sSv.clear();
    }

    for(int i=4; i<sentance.size(); i+=4) {
      if(!sentance.isNull(i+3)) {
        GpsSvInfo sv;
        sv.size = sizeof(GpsSvInfo);
        sv.prn = sentance.asInt(i);
        sv.elevation = sentance.asFloat(i+1);
        sv.azimuth = sentance.asFloat(i+2);
        sv.snr = sentance.asFloat(i+3);
        updateSV(sv);
      }
    }

  }

  GpsSvStatus status;
  status.size = sizeof(GpsSvStatus);
  status.num_svs = sSv.size();
  status.ephemeris_mask = 0;
  status.almanac_mask = 0;
  status.used_in_fix_mask = 0;

  for(size_t i=0; i<sSv.size(); i++) {
    status.sv_list[i] = sSv[i];
  }

  sGpsWorkQueue->schedule(new SVWorkUnit(status));


}


} // namespace gps
