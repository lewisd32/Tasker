/*

This example shows how tasks can run without explicit state. (Though this is not the recommended
way to write tasks.)

There are 3 tasks in this example.

Two that read an analog input (that could be connected to an analog distance sensor), and one that
prints the distances.

Unlike the SharedState example, the two tasks that read the sensor in this example require
separate functions because the analog pin to read cannot be passed in via the state struct.

*/
#include <Tasker.h>

uint16_t distance1, distance2;

const int MAX_TASKS = 8;
Task taskStructs[MAX_TASKS];
Tasks tasks;

void setup()
{
  Serial.begin(9600);
  initTasks(tasks, taskStructs, MAX_TASKS);
  
  // Schedule the first updateDistance task to run as soon as possible, and every 10ms after that.
  schedulePeriodic(tasks, updateDistance1, NO_STATE, 0, 10*MS);
  // Schedule the second updateDistance task to run as soon as possible, and every 16ms after that.
  schedulePeriodic(tasks, updateDistance2, NO_STATE, 0, 16*MS);
  
  // Schedule the distance to be printed as soon as possible, and every second after that.
  schedulePeriodic(tasks, printDistance, NO_STATE, 0, 1000*MS);
}

void loop() {
  execNextTask(tasks);
}

void updateDistance1(struct Task& task, void* state) {
  distance1 = analogRead(0);
}

void updateDistance2(struct Task& task, void* state) {
  distance2 = analogRead(1);
}

void printDistance(struct Task& task, void* state) {
  // This task function has no state of it's own.
  // It reads from the state used by the other two tasks.
  Serial.print("distance1 = "); Serial.print(distance1);
  Serial.print("  distance2 = "); Serial.println(distance2);
}

