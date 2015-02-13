/*

This example demonstrates using a scheduled-once task to blink an
LED using `sleep` instead of delay.

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

  // Schedule the task to run once 500ms from now.
  scheduleOnce(tasks, taskFunc, taskState, micros()+500*MS);
}

void loop() {
  execNextTask(tasks);
}

void taskFunc(struct Task& task, struct TaskState& state) {
  if (state.step == 0) {
    // Turn the LED on
    digitalWrite(13, HIGH);
    state.step = 1;
    // Schedule this task to run again 700ms after it was last run.
    syncSleepMillis(700);
  } else {
    // Turn the LED off
    digitalWrite(13, LOW);
    state.step = 0;
    // Schedule this task to run again 300ms after it was last run.
    syncSleepMillis(300);
  }
}

