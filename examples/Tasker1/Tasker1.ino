#include <Tasker.h>

struct Task1State {
  int counter;
  bool overflow;
  uint32_t now;
  int32_t drift;
};

Task task1;
Task1State task1state, task2state;

const int MAX_TASKS = 8;
Task taskStructs[MAX_TASKS];
Tasks tasks;

void setup()
{
  Serial.begin(9600);
  initTasks(tasks, taskStructs, MAX_TASKS);

  scheduleOnce(tasks, task1func, task1state, 1000000);
  schedulePeriodic(tasks, task2func, task2state, 1500000, 1000000);
  scheduleOnce(tasks, task3func, task2state, 0);
  
  Serial.println(tasks.tasks[0].schedule.requestedNextExecutionTimeUs);
  Serial.println(tasks.tasks[1].schedule.requestedNextExecutionTimeUs);
}

void loop() {
  execNextTask(tasks);
}

void task1func(struct Task& task, struct Task1State& state) {
  uint32_t now = micros();
  if (state.now == 0) {
    state.now = now - 1000000;
  }
  state.counter++;
  uint32_t interval = now - state.now;
  state.drift += ((int32_t)interval - 1000000);
  Serial.print("1: now = "); Serial.println(now);
  Serial.print("1: counter = "); Serial.println(state.counter);
  Serial.print("1: inverval = "); Serial.println(interval);
  Serial.print("1: drift = "); Serial.println(state.drift);
  Serial.println();
//  Serial.print("maxDuration = "); Serial.println(task.maxDurationUs);
  state.now = now;
  
  syncSleepMillis(1000);
}

void task2func(struct Task& task, struct Task1State& state) {
  uint32_t now = micros();
  state.counter *= 2;
  if (state.counter == 0) {
    state.overflow = true;
  }
  Serial.print("2: now = "); Serial.println(now);
  Serial.print("2: counter = "); Serial.println(state.counter);
  Serial.println();
}

void task3func(struct Task& task, struct Task1State& state) {
  uint32_t now = micros();
  state.counter = 1;
  state.overflow = false;
  Serial.print("3: now = "); Serial.println(now);
  Serial.print("3: reset counter to 1");
  Serial.println();
  sleepUntil(state.overflow);
}


