#include <Tasker.h>
#include <internal/TaskerInternals.h>
#include <internal/testutil.h>

void setup() {
  Serial.begin(9600);
  
  initTests();
  
  runTest(testSignedOverflow);
  runTest(testUnsignedOverflow);
  runTest(testUnderflow);
  
  printResults();
}

bool testSignedOverflow() {
  Task task;
  memset(&(task), 0, sizeof(Task));
  task.schedule.type = PERIODIC;
  task.schedule.periodic.requestedNextExecutionTimeUs = 2147483647;
  
  assertEquals(1, getTimeUntilExecution(task, 2147483646), "Test signed overflow 1");
  assertEquals(0, getTimeUntilExecution(task, 2147483647), "Test signed overflow 2");
  assertEquals(-1, getTimeUntilExecution(task, 2147483648), "Test signed overflow 3");
  
  return true;
}

bool testUnsignedOverflow() {
  Task task;
  memset(&(task), 0, sizeof(Task));
  task.schedule.type = PERIODIC;
  task.schedule.periodic.requestedNextExecutionTimeUs = 4294967295;
  
  assertEquals(1, getTimeUntilExecution(task, 4294967294), "Test unsigned overflow 3");
  assertEquals(0, getTimeUntilExecution(task, 4294967295), "Test unsigned overflow 4");
  assertEquals(-1, getTimeUntilExecution(task, 4294967296), "Test unsigned overflow 5");
  
  return true;
}

bool testUnderflow() {
  Task task;
  memset(&(task), 0, sizeof(Task));
  task.schedule.type = PERIODIC;
  task.schedule.periodic.requestedNextExecutionTimeUs = 2147483647;
  
  assertEquals(-2147483648, getTimeUntilExecution(task, -1), "Test underflow 1");
  assertEquals(2147483647, getTimeUntilExecution(task, 0), "Test underflow 2");
  assertEquals(2147483646, getTimeUntilExecution(task, 1), "Test underflow 3");
  
  return true;
}

void loop() {
}
