#include <Tasker.h>
#include "debug.h"

// TODO I'm not sure why the Arduino IDE compiler keeps warning about this.
const uint32_t MAX_LONG = 4294967295;
const int32_t MAX_SIGNED_LONG = MAX_LONG/2-1;

Task* currentTask;

void initTasks(struct Tasks& tasks, Task taskStructs[], uint8_t maxTasks) {
  tasks.tasks = taskStructs;
  tasks.maxTasks = maxTasks;
  for (uint8_t i = 0; i < tasks.maxTasks; ++i) {
    memset(&(tasks.tasks[i]), 0, sizeof(Task));
    tasks.tasks[i].schedule.type = NONE;
  }
}

void execTask(struct Task& task) {
  currentTask = &task;
  // Save the timestamp of when it executes for later.
  uint32_t taskExecutedTimeUs = micros();

  // If it's not a periodic task, don't execute it again without rescheduling.
  if (task.schedule.type != PERIODIC) {
    task.schedule.type = NONE;
  }

  // Execute the task, passing in the task struct and it's state.
  task.func(task, task.state);
  uint32_t taskDurationUs = micros() - taskExecutedTimeUs;
  // Record the time it ran, and the time it was scheduled to run.
  task.lastExecutedTimeUs = taskExecutedTimeUs;
  task.lastScheduledTimeUs = task.schedule.requestedNextExecutionTimeUs;
  task.maxDurationUs = max(task.maxDurationUs, taskDurationUs);

  // If it's a periodic task, set up the next scheduled execution.
  if (task.schedule.type == PERIODIC) {
    uint32_t period = task.schedule.period;
    task.schedule.requestedNextExecutionTimeUs = task.lastScheduledTimeUs + period;
  }
  currentTask = NULL;
}

int32_t getTimeUntilExecution(struct Task& task, uint32_t now) {
  TaskSchedule& schedule = task.schedule;
  switch (schedule.type) {
    case ONCE:
    case SLEEP:
    case PERIODIC:
      return schedule.requestedNextExecutionTimeUs - now;
      break;
    case UNTIL:
      if (*schedule.whenVar) {
        return 0;
      }
      break;
    case WHEN:
      if (!*schedule.whenVar) {
        return 0;
      }
      break;
    case NONE:
      break;
    default:
      serWarn("Unknown task schedule type ");
      serWarnln(schedule.type);
  }
  serTrace("returning ");
  serTraceln(MAX_LONG);
  return MAX_SIGNED_LONG;
}

Task* findNextTask(struct Tasks& tasks) {
  // TODO use priority
  Task* selectedTask = NULL;
  int32_t timeUntilExec = MAX_SIGNED_LONG;
  uint32_t now = micros();
  serDebug("timeUntilExec ");
  serDebugln(timeUntilExec);
  for (uint8_t i = 0; i < tasks.maxTasks; ++i) {
    Task& task = tasks.tasks[i];
    int32_t taskTimeUntilExec = getTimeUntilExecution(task, now);
    serDebug("taskTimeUntilExec ");
    serDebugln(taskTimeUntilExec);
    if (taskTimeUntilExec < timeUntilExec) {
      serDebug("selecting ");
      serDebugln(i);
      selectedTask = &task;
      timeUntilExec = taskTimeUntilExec;
    }
  }
  return selectedTask;
}

void execNextTask(struct Tasks& tasks) {
  Task* selected = findNextTask(tasks);
  if (selected) {
    int32_t scheduledTimeUs = selected->schedule.requestedNextExecutionTimeUs;
    serInfo("Scheduled at ");
    serInfoln(scheduledTimeUs);
    serInfo("Now ");
    serInfoln(micros());
    while (scheduledTimeUs - (int32_t)micros() > 0) {
      // tight loop
      serTrace(".");
    }
    serInfoln("Executing");
    execTask(*selected);
    serInfoln("done");
  } else {
    serDebugln("No task to execute");
  }
}

Task* findEmptyTaskSlot(struct Tasks& tasks) {
  for (uint8_t i = 0; i < tasks.maxTasks; ++i) {
    if (tasks.tasks[i].func == NULL) {
      return &tasks.tasks[i];
    }
  }
  serWarnln("No empty task slot found");
  return NULL;
}

/***************************************************************
 * Scheduling
 ***************************************************************/

Task* _scheduleOnce(struct Tasks& tasks, TaskFunc func, void* state, uint32_t scheduledExecutionTimeUs) {
  Task& task = *findEmptyTaskSlot(tasks);
  task.func = func;
  task.state = state;
  task.schedule.type = ONCE;
  task.schedule.requestedNextExecutionTimeUs = scheduledExecutionTimeUs;
  return &task;
}

Task* _schedulePeriodic(struct Tasks& tasks, TaskFunc func, void* state, uint32_t scheduledExecutionTimeUs, uint32_t intervalUs) {
  Task& task = *findEmptyTaskSlot(tasks);
  task.func = func;
  task.state = state;
  task.schedule.type = PERIODIC;
  task.schedule.requestedNextExecutionTimeUs = scheduledExecutionTimeUs;
  task.schedule.period = intervalUs;
  return &task;
}

Task* _scheduleWhen(struct Tasks& tasks, TaskFunc func, void* state, bool& whenVar) {
  Task& task = *findEmptyTaskSlot(tasks);
  task.func = func;
  task.state = state;
  task.schedule.type = WHEN;
  task.schedule.whenVar = &whenVar;
  return &task;
}

Task* _scheduleUntil(struct Tasks& tasks, TaskFunc func, void* state, bool& untilVar) {
  Task& task = *findEmptyTaskSlot(tasks);
  task.func = func;
  task.state = state;
  task.schedule.type = UNTIL;
  task.schedule.whenVar = &untilVar;
  return &task;
}

/***************************************************************
 * Sleep 
 ***************************************************************/


void sleepMillis(uint16_t milliseconds) {
  if (currentTask) {
    uint32_t now = micros();
    TaskSchedule& schedule = currentTask->schedule;
    schedule.type = SLEEP;
    schedule.requestedNextExecutionTimeUs = now + (uint32_t)milliseconds*1000;
  } else {
    serWarnln("Cannot call sleepMillis outside of a task");
  }
}

void syncSleepMillis(uint16_t milliseconds) {
  if (currentTask) {
    TaskSchedule& schedule = currentTask->schedule;
    schedule.type = SLEEP;
    schedule.requestedNextExecutionTimeUs += (uint32_t)milliseconds*1000;
  } else {
    serWarnln("Cannot call syncSleepMillis outside of a task");
  }
}

void sleepUntil(bool& untilVar) {
  if (currentTask) {
    TaskSchedule& schedule = currentTask->schedule;
    schedule.type = UNTIL;
    schedule.whenVar = &untilVar;
  } else {
    serWarnln("Cannot call sleepUntil outside of a task");
  }
}

void sleepWhen(bool& whenVar) {
  if (currentTask) {
    TaskSchedule& schedule = currentTask->schedule;
    schedule.type = WHEN;
    schedule.whenVar = &whenVar;
  } else {
    serWarnln("Cannot call sleepWhen outside of a task");
  }
}


// vim: set tabstop=2 shiftwidth=2 softtabstop=2 smarttab expandtab:
