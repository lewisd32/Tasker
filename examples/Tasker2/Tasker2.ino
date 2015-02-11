#include <Tasker.h>

const int MAX_TASKS = 3;
Task taskStructs[MAX_TASKS];
Tasks tasks;

Task* distance1Task;
Task* distance2Task;

uint16_t distance1;
uint16_t distance2;

uint8_t counter = 0;
bool distanceReady = false;

void setup()
{
  Serial.begin(9600);
  Serial.print("sizeof(task) = ");
  Serial.println(sizeof(taskStructs[0]));
  initTasks(tasks, taskStructs, MAX_TASKS);

  distance1Task = scheduleOnce(tasks, updateDistance1, NO_STATE, 1000*MS);
  distance2Task = scheduleOnce(tasks, updateDistance2, NO_STATE, 1600*MS);
  scheduleWhen(tasks, printDistance, NO_STATE, distanceReady);
 
}

void loop() {
  execNextTask(tasks);
}

void updateDistance1(struct Task& task, void* state) {
  distance1 = readDistance1();
  incCounter();
}

uint16_t readDistance1() {
  Serial.println("Read distance 1");
  return 10;
}

void updateDistance2(struct Task& task, void* state) {
  distance2 = readDistance2();
  incCounter();
}

uint16_t readDistance2() {
  Serial.println("Read distance 2");
  return 20;
}

void incCounter() {
  counter++;
  if (counter == 2) {
    distanceReady = true;
  }
}

void printDistance(struct Task& task, void* state) {
  distanceReady = false;
  counter = 0;
  Serial.print("Distance ");
  Serial.print(distance1);
  Serial.print(" ");
  Serial.println(distance2);
//  sleepUntil(distanceReady);
  uint32_t t1 = distance1Task->schedule.once.lastScheduledTimeUs + 1000*MS;
  uint32_t t2 = distance2Task->schedule.once.lastScheduledTimeUs + 1600*MS;
  scheduleTaskOnce(distance1Task, t1);
  scheduleTaskOnce(distance2Task, t2);
}
