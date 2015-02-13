/*

This example demonstrates using a periodically scheduled task to blink an
LED without using `delay`.

*/
#include <debug.h>
#include <Tasker.h>

struct TaskState {
  uint8_t step;
} taskState;

const int MAX_TASKS = 4;
Task taskStructs[MAX_TASKS];
Tasks tasks;

void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  
  initTasks(tasks, taskStructs, MAX_TASKS);

  // Schedule the task to start 500ms from now, and repeat every 500ms.
  schedulePeriodic(tasks, taskFunc, taskState, micros()+500*MS, 500*MS);
}

void loop() {
  execNextTask(tasks);
}

void taskFunc(struct Task& task, struct TaskState& state) {
  if (state.step == 0) {
    digitalWrite(13, HIGH);
    state.step = 1;
  } else {
    digitalWrite(13, LOW);
    state.step = 0;
  }
}

