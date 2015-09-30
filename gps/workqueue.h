#ifndef WORKQUEUE_H_
#define WORKQUEUE_H_

#include <utils/Vector.h>
#include <utils/threads.h>

using namespace android;

namespace gps {

class WorkQueue {
public:
  class WorkUnit {
  public:
    WorkUnit() {}
    virtual ~WorkUnit() {}

    virtual void run() = 0;
  };

  WorkQueue();
  ~WorkQueue();

  status_t schedule(WorkUnit* workUnit);

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
  Vector<WorkUnit*> mWorkUnits;

};

} // namespace gps

#endif // WORKQUEUE_H_