#ifndef WORKQUEUE_H_
#define WORKQUEUE_H_

#include <utils/Timers.h>
#include <utils/KeyedVector.h>
#include <utils/threads.h>

using namespace android;

namespace gps {

#define NO_DELAY 0

class WorkQueue {
public:
  class WorkUnit {
  public:
    WorkUnit(long delay = NO_DELAY) : mDelay(delay) {}
    virtual ~WorkUnit() {}

    virtual void run() = 0;
    
    /*
    * millisecond delay for repeating tasks.
    * <= 0 indicates a one-shot event;
    */
    long mDelay;
  };

  WorkQueue();
  ~WorkQueue();

  status_t schedule(WorkUnit* workUnit);
  
  /*
  * cancels a repeating WorkUnit
  */
  void cancel(WorkUnit* workUnit);

  /*
  * Cancels all pending work and prevents additional
  * work units from being scheduled.
  */
  status_t cancel();

  /*
  * waits for all work to complete.
  */
  status_t finish();


  void runLoop();

private:
  Mutex mLock;
  Condition mWorkQueueCondition;

  bool mCanceled;
  bool mRunning;
  KeyedVector<nsecs_t, WorkUnit*> mWorkUnits;

};

} // namespace gps

#endif // WORKQUEUE_H_