#include <Tasker.h>

const int MAX_TASKS = 10;
Task taskStructs[MAX_TASKS];
Tasks tasks;


void setup() {
  Serial.begin(9600);
  Serial.print("sizeof(Task) = ");
  Serial.println(sizeof(taskStructs[0]));
  Serial.print("sizeof(Taskss) = ");
  Serial.println(sizeof(tasks));
}

void loop() {
}
