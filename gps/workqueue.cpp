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
  mWorkUnits.add(workUnit);
  mWorkQueueCondition.broadcast();
  return OK;
}

void WorkQueue::runLoop() {
  mLock.lock();
  mRunning = true;
  
  while(!mCanceled) {
	  if(!mWorkUnits.isEmpty()) {
	    WorkUnit* workUnit = mWorkUnits.itemAt(0);
	    mWorkUnits.removeAt(0);
		
		  mLock.unlock();
		  workUnit->run();
      delete workUnit;
		  mLock.lock();
	  } else {
      mWorkQueueCondition.wait(mLock);
    }
  }
  
  mRunning = false;
  mWorkQueueCondition.broadcast();
  
  mLock.unlock();
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