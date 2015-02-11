#include <Tasker.h>
#include "debug.h"

// TODO I'm not sure why the Arduino IDE compiler keeps warning about this.
const uint32_t MAX_LONG = 4294967295;
const int32_t MAX_SIGNED_LONG = MAX_LONG/2-1;

#define never(now) ((uint32_t)now + MAX_SIGNED_LONG)

struct CurrentTaskInfo {
  Task* task;
  uint32_t scheduledExecutionTime;
} currentTaskInfo;


void initTasks(struct Tasks& tasks, Task taskStructs[], uint8_t maxTasks) {
  tasks.tasks = taskStructs;
  tasks.maxTasks = maxTasks;
  for (uint8_t i = 0; i < tasks.maxTasks; ++i) {
    memset(&(tasks.tasks[i]), 0, sizeof(Task));
    tasks.tasks[i].schedule.type = NONE;
  }
}

int32_t getTimeUntilExecution(struct Task& task, uint32_t now) {
  TaskSchedule& schedule = task.schedule;
  switch (schedule.type) {
    case ONCE:
    case SLEEP:
      return schedule.once.requestedNextExecutionTimeUs - now;
      break;
    case PERIODIC:
      return schedule.periodic.requestedNextExecutionTimeUs - now;
      break;
    case UNTIL:
      if (!*schedule.when.whenVar) {
        return 0;
      }
      break;
    case WHEN:
      if (*schedule.when.whenVar) {
        return 0;
      }
      break;
    case NONE:
      break;
    default:
      serWarn("Unknown task schedule type ");
      serWarnln(schedule.type);
  }
  serTrace("returning never (");
  serTrace(MAX_LONG);
  serTrace(")");
  return never(now);
}

Task* findNextTask(struct Tasks& tasks, uint32_t now) {
  // TODO use priority
  Task* selectedTask = NULL;
  int32_t timeUntilExec = never(now);
  int32_t neverUs = never(now);
  serDebug("timeUntilExec ");
  if (timeUntilExec == neverUs) {
    serDebug("never (");
    serDebug(timeUntilExec);
    serDebugln(")");
  } else {
    serDebugln(timeUntilExec);
  }
  for (uint8_t i = 0; i < tasks.maxTasks; ++i) {
    Task& task = tasks.tasks[i];
    int32_t taskTimeUntilExec = getTimeUntilExecution(task, now);
    serDebug(i);
    serDebug(" taskTimeUntilExec ");
    if (taskTimeUntilExec == neverUs) {
      serDebug("never (");
      serDebug(taskTimeUntilExec);
      serDebugln(")");
    } else {
      serDebugln(taskTimeUntilExec);
    }
    if (taskTimeUntilExec != neverUs &&
        (taskTimeUntilExec - timeUntilExec < 0 || timeUntilExec == neverUs)) {
      serDebug(i);
      serDebug(" selecting ");
      serDebugln(i);
      selectedTask = &task;
      timeUntilExec = taskTimeUntilExec;
      currentTaskInfo.scheduledExecutionTime = now + taskTimeUntilExec;
    }
  }
  currentTaskInfo.task = selectedTask;
  return selectedTask;
}

void execTask(struct Task& task) {
  // Set currentTask so that the various sleep functions can use it.
  currentTaskInfo.task = &task;

  // Save the timestamp of when it executes for later.
  uint32_t taskExecutedTimeUs = micros();

  ScheduleType scheduleType = task.schedule.type;
  // If it's not a periodic task, or a task with a bool flag, don't execute
  // it again without rescheduling.
  if (scheduleType != PERIODIC && scheduleType != WHEN && scheduleType != UNTIL) {
    task.schedule.type = NONE;
  }

  // FIXME: move the delay loop here so timing is more accurate.
  // Execute the task, passing in the task struct and it's state.
  task.func(task, task.state);

  // Calculate how long the task took.
  uint32_t taskDurationUs = micros() - taskExecutedTimeUs;
  // Record the time that the task actually ran at.
  task.lastExecutedTimeUs = taskExecutedTimeUs;

  // For some task types, record the time it had been scheduled to run at.
  // It's only safe to write to the schedule struct if the type (and hence the
  // union) didn't change, or was set to NONE above, and left unchanged.
  if ((scheduleType == ONCE ||
      scheduleType == SLEEP) &&
      (task.schedule.type == NONE ||
       task.schedule.type == ONCE ||
       task.schedule.type == SLEEP)) {
    task.schedule.once.lastScheduledTimeUs = task.schedule.once.requestedNextExecutionTimeUs;
  }
  if (scheduleType == PERIODIC &&
      (task.schedule.type == NONE ||
       task.schedule.type == PERIODIC)) {
    task.schedule.periodic.lastScheduledTimeUs = task.schedule.periodic.requestedNextExecutionTimeUs;
  }

  // Record the maximum length of time this task has taken.
  task.maxDurationUs = max(task.maxDurationUs, taskDurationUs);

  // If it's a periodic task, set up the next scheduled execution.
  if (task.schedule.type == PERIODIC) {
    uint32_t period = task.schedule.periodic.period;
    task.schedule.periodic.requestedNextExecutionTimeUs = task.schedule.periodic.lastScheduledTimeUs + period;
  }

  // Clear the current task, so that misplaced sleep calls don't mess with things.
  currentTaskInfo.task = NULL;
}

void execNextTask(struct Tasks& tasks) {
  uint32_t now = micros();
  Task* selectedTask = findNextTask(tasks, now);
  if (selectedTask) {
    int32_t scheduledTimeUs = currentTaskInfo.scheduledExecutionTime;
    serInfo("Scheduled at ");
    serInfoln(scheduledTimeUs);
    serInfo("Now ");
    serInfoln(micros());
    while (scheduledTimeUs - (int32_t)micros() > 0) {
      // tight loop
      serTrace(".");
    }
    serInfoln("Executing");
    execTask(*selectedTask);
    serInfoln("done");
  } else {
    serDebugln("No task to execute");
  }
}

/***************************************************************
 * Scheduling
 ***************************************************************/

Task* findEmptyTaskSlot(struct Tasks& tasks) {
  for (uint8_t i = 0; i < tasks.maxTasks; ++i) {
    if (tasks.tasks[i].func == NULL) {
      return &tasks.tasks[i];
    }
  }
  serWarnln("No empty task slot found");
  return NULL;
}

Task* _scheduleOnce(struct Tasks& tasks, TaskFunc func, void* state, uint32_t scheduledExecutionTimeUs) {
  Task& task = *findEmptyTaskSlot(tasks);
  task.func = func;
  task.state = state;
  scheduleTaskOnce(&task, scheduledExecutionTimeUs);
  return &task;
}

void scheduleTaskOnce(struct Task* task, uint32_t scheduledExecutionTimeUs) {
  task->schedule.type = ONCE;
  task->schedule.once.requestedNextExecutionTimeUs = scheduledExecutionTimeUs;
}

Task* _schedulePeriodic(struct Tasks& tasks, TaskFunc func, void* state, uint32_t scheduledExecutionTimeUs, uint32_t intervalUs) {
  Task& task = *findEmptyTaskSlot(tasks);
  task.func = func;
  task.state = state;
  scheduleTaskPeriodic(&task, scheduledExecutionTimeUs, intervalUs);
  return &task;
}

void scheduleTaskPeriodic(struct Task* task, int32_t scheduledExecutionTimeUs, uint32_t intervalUs) {
  task->schedule.type = PERIODIC;
  task->schedule.periodic.requestedNextExecutionTimeUs = scheduledExecutionTimeUs;
  task->schedule.periodic.period = intervalUs;
}

Task* _scheduleWhen(struct Tasks& tasks, TaskFunc func, void* state, bool& whenVar) {
  Task& task = *findEmptyTaskSlot(tasks);
  task.func = func;
  task.state = state;
  scheduleTaskWhen(&task, whenVar);
  return &task;
}

void scheduleTaskWhen(struct Task* task, bool& whenVar) {
  task->schedule.type = WHEN;
  task->schedule.when.whenVar = &whenVar;
}

Task* _scheduleUntil(struct Tasks& tasks, TaskFunc func, void* state, bool& untilVar) {
  Task& task = *findEmptyTaskSlot(tasks);
  task.func = func;
  task.state = state;
  scheduleTaskUntil(&task, untilVar);
  return &task;
}

void scheduleTaskUntil(struct Task* task, bool& untilVar) {
  task->schedule.type = UNTIL;
  task->schedule.when.whenVar = &untilVar;
}

/***************************************************************
 * Sleep 
 ***************************************************************/


void sleepMillis(uint16_t milliseconds) {
  if (currentTaskInfo.task) {
    uint32_t now = micros();
    TaskSchedule& schedule = currentTaskInfo.task->schedule;
    schedule.type = SLEEP;
    schedule.once.requestedNextExecutionTimeUs = now + (uint32_t)milliseconds*1000;
  } else {
    serWarnln("Cannot call sleepMillis outside of a task");
  }
}

void syncSleepMillis(uint16_t milliseconds) {
  if (currentTaskInfo.task) {
    TaskSchedule& schedule = currentTaskInfo.task->schedule;
    schedule.type = SLEEP;
    schedule.once.requestedNextExecutionTimeUs += (uint32_t)milliseconds*1000;
  } else {
    serWarnln("Cannot call syncSleepMillis outside of a task");
  }
}

void sleepUntil(bool& untilVar) {
  if (currentTaskInfo.task) {
    TaskSchedule& schedule = currentTaskInfo.task->schedule;
    schedule.type = WHEN;
    schedule.when.whenVar = &untilVar;
  } else {
    serWarnln("Cannot call sleepUntil outside of a task");
  }
}

void sleepWhen(bool& whenVar) {
  if (currentTaskInfo.task) {
    TaskSchedule& schedule = currentTaskInfo.task->schedule;
    schedule.type = UNTIL;
    schedule.when.whenVar = &whenVar;
  } else {
    serWarnln("Cannot call sleepWhen outside of a task");
  }
}


// vim: set tabstop=2 shiftwidth=2 softtabstop=2 smarttab expandtab:
