/*

This example demonstrates how `syncSleep` prevents drift, while using `sleep` allows for it.

This example schedules two tasks.  One uses `syncSleep`, the other uses `sleep.
Both tasks keep track of 'drift'. That is, how far off they are from running exactly every
second. This cumulative drift, in microseconds, is printed when the task runs.

*/
#include <Tasker.h>

struct TaskState {
  uint32_t prevNow;
  int32_t drift;
  uint32_t maxDrift;
};

Task task;
TaskState taskState1, taskState2;

const int MAX_TASKS = 4;
Task taskStructs[MAX_TASKS];
Tasks tasks;

void setup()
{
  Serial.begin(9600);
  initTasks(tasks, taskStructs, MAX_TASKS);

  // Schedule to run once, 1 second (1000ms) from now.
  scheduleOnce(tasks, taskFunc1, taskState1, micros()+1000*MS);

  // Schedule to run once, 1.5 seconds (1500ms) from now.
  scheduleOnce(tasks, taskFunc2, taskState2, micros()+1500*MS);
}

void loop() {
  execNextTask(tasks);
}

void updateDrift(struct TaskState& state) {
  uint32_t now = micros();
  if (state.prevNow == 0) {
    // If this is the first time running the task, just record the current time and return.
    state.prevNow = now;
    return;
  }
  // Calculate how much time has elapsed since last time the task was run.
  uint32_t interval = now - state.prevNow;
  // Calculate how much different from the expected interval the actual interval is.
  int32_t intervalDelta = ((int32_t)interval - 1000*MS);
  // Add the delta to the cumulative drift.
  state.drift += intervalDelta;
  state.maxDrift = max(state.maxDrift, abs(state.drift));
  // Record the current time we got earlier, for next time the task runs.
  state.prevNow = now;
}

void taskFunc1(struct Task& task, struct TaskState& state) {
  updateDrift(state);  
  Serial.print("sync:  drift = "); Serial.print(state.drift);
  Serial.print(" max = "); Serial.println(state.maxDrift);
  // `sleepSync` will schedule this task to be run 1000 milliseconds after the last
  // time it was scheduled to run.
  syncSleepMillis(1000);
}

void taskFunc2(struct Task& task, struct TaskState& state) {
  updateDrift(state);  
  Serial.print("sleep:  drift = "); Serial.println(state.drift);
  // `sleep` will schedule this task to be run 1000 milliseconds from now.
  sleepMillis(1000);
}

