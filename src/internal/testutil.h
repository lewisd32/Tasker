#ifndef _lewisd_TaskerTestUtil
#define _lewisd_TaskerTestUtil

#define initTests() uint16_t failures = 0, passes = 0;

#define runTest(testName) (testName())?passes++:failures++

#define printResults() \
if (failures == 0) { \
  Serial.print("Pass! ("); \
  Serial.print(passes); \
  Serial.println(" tests)"); \
} else { \
  Serial.print("There were "); \
  Serial.print(failures); \
  Serial.println(" failures."); \
}

#define assertEquals(expected, actual, message) { \
  if ((expected) != (actual)) { \
    Serial.println(message); \
    Serial.print("Got '"); \
    Serial.print(actual); \
    Serial.print("' but expected '"); \
    Serial.print(expected); \
    Serial.println("'"); \
    return false; \
  } \
}

#endif
// vim: set tabstop=2 shiftwidth=2 softtabstop=2 smarttab expandtab: