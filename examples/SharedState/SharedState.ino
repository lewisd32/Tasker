/*

This example shows how a task can read the state used by another task.

There are 3 tasks in this example.

Two that read an analog input (that could be connected to an analog distance sensor), and one that
prints the distances.

The analog sensors are read much more often than the distance is printed.

*/
#include <Tasker.h>

struct DistanceState {
  uint8_t analogPin;
  uint16_t distance;
};

DistanceState distanceState1, distanceState2;

const int MAX_TASKS = 8;
Task taskStructs[MAX_TASKS];
Tasks tasks;

void setup()
{
  Serial.begin(9600);
  initTasks(tasks, taskStructs, MAX_TASKS);
  
  distanceState1.analogPin = 0;
  distanceState1.analogPin = 1;

  // Schedule the first updateDistance task to run as soon as possible, and every 10ms after that.
  schedulePeriodic(tasks, updateDistance, distanceState1, 0, 10*MS);
  // Schedule the second updateDistance task to run as soon as possible, and every 16ms after that.
  schedulePeriodic(tasks, updateDistance, distanceState2, 0, 16*MS);
  
  // Schedule the distance to be printed as soon as possible, and every second after that.
  schedulePeriodic(tasks, printDistance, NO_STATE, 0, 1000*MS);
}

void loop() {
  execNextTask(tasks);
}

void updateDistance(struct Task& task, struct DistanceState& state) {
  state.distance = analogRead(state.analogPin);
}

void printDistance(struct Task& task, void* state) {
  // This task function has no state of it's own.
  // It reads from the state used by the other two tasks.
  Serial.print("distance1 = "); Serial.print(distanceState1.distance);
  Serial.print("  distance2 = "); Serial.println(distanceState2.distance);
}

