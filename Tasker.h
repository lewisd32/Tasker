#ifndef _lewisd_Tasker
#define _lewisd_Tasker

#include <Arduino.h>

// Scheduling is all done in microseconds, so multiplying by this constant
// gives milliseconds, for where that's more convenient.
const uint32_t MS = 1000;

// Forward declaration of Task to allow TaskFunc and Task reference each other.
struct Task;

// Typedef to make casks in defines below clearer.
typedef void (*TaskFunc)(Task &Task, void *state);

/* Different ways of scheduling tasks:
 *   NONE: Disable running the task until it's re-scheduled.
 *   ONCE: Run once at the scheduled time.
 *   SLEEP: Currently the same as ONCE.
 *   PERIODIC: Run periodically, with the scheduled period/interval.
 *   WHEN: Runs when the bool pointer value is true.
 *   UNTIL: Runs when the bool pointer value is false.
 */
enum ScheduleType { NONE, ONCE, SLEEP, PERIODIC, WHEN, UNTIL };

struct PeriodicScheduleData {
  uint32_t requestedNextExecutionTimeUs;
  uint32_t lastScheduledTimeUs;
  uint32_t period;
};

struct OnceScheduleData {
  uint32_t requestedNextExecutionTimeUs;
  uint32_t lastScheduledTimeUs;
};

struct WhenScheduleData {
  bool* whenVar;
};

struct TaskSchedule {
  ScheduleType type;
  union {
    PeriodicScheduleData periodic;
    OnceScheduleData once;
    WhenScheduleData when;
  };
};

struct Task {
  uint8_t priority;
  uint32_t maxDurationUs;
  uint32_t lastExecutedTimeUs;
  TaskSchedule schedule;
  TaskFunc func;
  void* state;
};

struct Tasks {
  Task* tasks;
  uint8_t maxTasks;
};

const void* const NO_STATE = NULL;

void initTasks(struct Tasks& tasks, Task taskStructs[], uint8_t maxTasks);

void execNextTask(struct Tasks& tasks);

// ***** ScheduleOnce *********************

#define scheduleOnce(tasks, func, state, us) (_scheduleOnce(tasks, (TaskFunc)func, (void*)&state, us))
Task* _scheduleOnce(struct Tasks& tasks, TaskFunc func, void* state, uint32_t scheduledExecutionTimeUs);

void scheduleTaskOnce(struct Task* task, uint32_t scheduledExecutionTimeUs);

// ***** SchedulePeriodic *********************

#define schedulePeriodic(tasks, func, state, first, interval) (_schedulePeriodic(tasks, (TaskFunc)func, (void*)&state, first, interval))
Task* _schedulePeriodic(struct Tasks& tasks, TaskFunc func, void* state, uint32_t scheduledExecutionTimeUs, uint32_t intervalUs);

void scheduleTaskPeriodic(struct Task* task, int32_t scheduledExecutionTimeUs, uint32_t intervalUs);

// ***** ScheduleWhen *********************

#define scheduleWhen(tasks, func, state, whenVar) (_scheduleWhen(tasks, (TaskFunc)func, (void*)&state, whenVar))
Task* _scheduleWhen(struct Tasks& tasks, TaskFunc func, void* state, bool& whenVar);

void scheduleTaskWhen(struct Task* task, bool& whenVar);

// ***** ScheduleUntil *********************

#define scheduleUntil(tasks, func, state, whenVar) (_scheduleUntil(tasks, (TaskFunc)func, (void*)&state, untilVar))
Task* _scheduleUntil(struct Tasks& tasks, TaskFunc func, void* state, bool& untilVar);

void scheduleTaskUntil(struct Task* task, bool& untilVar);

// ***** Sleep *********************

void sleepMillis(uint16_t milliseconds);

void syncSleepMillis(uint16_t milliseconds);

void sleepUntil(bool& untilVar);

void sleepWhen(bool& whenVar);

#endif
// vim: set tabstop=2 shiftwidth=2 softtabstop=2 smarttab expandtab:
