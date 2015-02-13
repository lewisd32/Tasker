#ifndef _lewisd_TaskerInternals
#define _lewisd_TaskerInternals

#include <Tasker.h>

int32_t getTimeUntilExecution(struct Task& task, uint32_t now);

Task* findNextTask(struct Tasks& tasks, uint32_t now);

void execTask(struct Task& task);

Task* findEmptyTaskSlot(struct Tasks& tasks);

#endif
// vim: set tabstop=2 shiftwidth=2 softtabstop=2 smarttab expandtab:
