#include "workqueue.h"


namespace gps {

WorkQueue::WorkQueue()
  : mCanceled(false), mRunning(false) {}

WorkQueue::~WorkQueue() {

}

status_t WorkQueue::schedule(WorkUnit* workUnit) {
  AutoMutex l(mLock);
  if(mCanceled) {
    return INVALID_OPERATION;
  }
  if(workUnit->mDelay > 0) {
    nsecs_t timeNow = systemTime();
    mWorkUnits.add(timeNow + ms2ns(workUnit->mDelay), workUnit);
  } else {
    mWorkUnits.add(0, workUnit);
  }
  mWorkQueueCondition.broadcast();
  return OK;
}

void WorkQueue::runLoop() {
  mLock.lock();
  if(mRunning) {
    ALOGE("work queue already running");
    return;
  }
  mRunning = true;

  while(!mCanceled) {
    nsecs_t timeNow = systemTime();
    if(!mWorkUnits.isEmpty()) {
      WorkUnit* workUnit = mWorkUnits.valueAt(0);
      nsecs_t runTime = mWorkUnits.keyAt(0);
      if(workUnit->mDelay == 0 || runTime <= timeNow) {
        mWorkUnits.removeItemsAt(0);
        
        mLock.unlock();
        workUnit->run();
        mLock.lock();
        
        if(workUnit->mDelay > 0) {
          //re-schedule
          mWorkUnits.add(timeNow + ms2ns(workUnit->mDelay), workUnit);
        } else {
          delete workUnit;
        }
        
      } else {
        mWorkQueueCondition.waitRelative(mLock, runTime-timeNow);
      }
      
    } else {
      mWorkQueueCondition.wait(mLock);
    }
  }

  mRunning = false;
  mWorkQueueCondition.broadcast();

  mLock.unlock();
}

void WorkQueue::cancel(WorkUnit* workUnit) {
  AutoMutex l(mLock);
  workUnit->mDelay = NO_DELAY;
  for(size_t i=0;i<mWorkUnits.size();i++) {
    if(workUnit == mWorkUnits.valueAt(i)) {
      mWorkUnits.removeItemsAt(i);
      break;
    }
  }
}

status_t WorkQueue::cancel() {
  AutoMutex l(mLock);
  mCanceled = true;
  return OK;
}

status_t WorkQueue::finish() {
  AutoMutex l(mLock);

  while(mRunning) {
    mWorkQueueCondition.wait(mLock);
  }
  return OK;
}


} // namespace gps