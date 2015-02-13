# Tasker
Task scheduling library for Arduino

# INCOMPLETE
## Use at your own risk.
## This code is not yet heavily tested.

## What is Tasker?

Tasker is, at it's core, a task schedule library, not a preemptive or
cooperative multitasking operating system.  This means that, unlike solutions
like
[SCoop](https://code.google.com/p/arduino-scoop-cooperative-scheduler-arm-avr/)
(which is fantastic), code needs to be written in a different way.  While SCoop
allows you to write very normal looking code, and scatter it with `yield`
and `sleep` calls, Tasker requires you to avoid using delays entirely.

Thankfully, Tasker tries to make this easy, by providing flexable scheduling
mechanisms (including psuedo-sleep functions), and an easy way of storing task
state.

## Usage

### Installation

(TBD: Write something about downloading a zip from github, and installing it in the
Arduino libraries folder.)

### Initial Setup

To start using Tasker, you'll need to:

1. Include the library
1. Define the maximum number of tasks
1. Create the `Task` struct array
1. Create the `Tasks` struct
1. Call `initTasks` in `setup()`
1. Call `execNextTask` in `loop()`

eg.
```
#include <Tasker.h>

const int MAX_TASKS = 8;
Task taskStructs[MAX_TASKS];
Tasks tasks;

void setup()
{
  // You'll need this if you want any errors or warnings to be printed.
  Serial.begin(9600);

  initTasks(tasks, taskStructs, MAX_TASKS);
}

void loop()
{
  execNextTask(tasks);
}
```

### Scheduling Tasks

#### Define a struct that holds the task's state.

Define a struct, and an instance of it:
```
struct CounterTaskState {
  int counter;
} counterTaskState;
```
or
```
struct CounterTaskState {
  int counter;
};

CounterTaskState counterTaskState;
```

#### Create the task function that uses the state.

Create the function that should be called when Tasker decides it's time to run
the task.
```
void counterTaskFunc(struct Task& task, struct CounterTaskState& state) {
  state.counter++;
  Serial.print("Counter is ");
  Serial.println(state.counter);
}
```

The declaration of this function must follow that strict format:

1. It must return `void`
1. It must take only two arguments
1. It must take a `Task&` as the first argument
1. It must take a ref (`&`) or pointer (`*`) as the second argument

This is *not* checked at compile time.  Getting this wrong *will* cause
strange problems at runtime.

Within the function, it is recommended (quite strongly) not to use `delay`, but
rather to call one of the Tasker `sleep` functions, and return immediately.
This does mean that you will need to retain state (in the `state` struct) to
allow the function to pick up at the right place when it's called again.

#### Schedule the task.

There are four ways of scheduling a task:

1. To run once, at a specific time.
1. To run periodically, at a first time, and a fixed interval thereafter.
1. When a `bool` becomes `true`.
1. When a `bool` becomes `false`. (Called 'until' in the code.)

Generally, at least one task should be scheduled by the end of the `setup()`
function, otherwise the `loop()` as written above won't have anything to
schedule.

Depending on the complexity of the programm, you will likely either.
* Schedule all tasks in `setup()`
* Schedule a 'startup' task in `setup()`, which will in turn schedule more
tasks.

Examples:
```
// Schedule to run once, 1 second (1000ms) from now.
Task* counterTask = scheduleOnce(tasks, counterTaskFunc, counterTaskState,
	micros()+1000*MS);
```
`MS` is a convenience variable set to 1000 to make scheduling in milliseconds a
bit easier.
```
// Schedule to run periodically, 1 second from now, and then every 500ms after that.
Task* counterTask = schedulePeriodic(tasks, counterTaskFunc, counterTaskState,
micros()+1000*MS, 500*MS);
```

```
// This must be global (ie. not inside any function, including setup and loop)
bool doCount = false;

// Schedule to run whenever 'doCount' becomes true.
Task* counterTask = scheduleWhen(tasks, counterTaskFunc, counterTaskState, doCount);
```

```
// This must be global (ie. not inside any function, including setup and loop)
bool waitToCount = true;

// Schedule to run whenever 'doCount' becomes true.
Task* counterTask = scheduleUntil(tasks, counterTaskFunc, counterTaskState, waitToCount);
```

**NOTE**: 'until' is a really bad name for this.  It's really 'when not'.

#### Sleeping within the task.

The usual pattern followed in simple Arduino code goes something like this:
```
void loop() {
	turnLedOn();
	delay(500);
	turnLedOff();
	delay(500);
}
```
This code would make an LED flash once per second.

Unfortunately, that `delay` ties up execution, which in a task scheduling
environment, means other tasks can't run.

One way to do this with a task scheduler, and a single task, would be:
```
void setup() {
  // all your usual setup

  // Schedule the task to start 500ms from now, and repeat every 500ms.
  schedulePeriodic(tasks, taskFunc, taskState, micros()+500*MS, 500*MS);
}

void taskFunc(struct Task& task, struct TaskState& state) {
  if (state.step == 0) {
    turnLedOn();
    state.step = 1;
  } else {
    turnLedOff();
    state.step = 0;
  }
}
```
(declaration of the state struct left out for brevity)

Or, if you want the LED to be on and off for different lengths of time:

```
void setup() {
  // all your usual setup

  // Schedule the task to run once 500ms from now.
  scheduleOnce(tasks, taskFunc, taskState, micros()+500*MS);
}

void taskFunc(struct Task& task, struct TaskState& state) {
  if (state.step == 0) {
    turnLedOn();
    state.step = 1;
    // Schedule this task to run again 700ms after it was last run.
    syncSleepMillis(700);
  } else {
    turnLedOff();
    state.step = 0;
    // Schedule this task to run again 300ms after it was last run.
    syncSleepMillis(300);
  }
}
```

This will turn the LED on for 700ms, and then turn it off for 300ms.

The difference between `sleep` and `sleepSync` is that sleep will schedule the
task to be run `x` milliseconds after the call to `sleep` is made.  `sleepSync`
will schedule it to be run `x` milliseconds after the last time it was scheduled
to run. `sleep` is perfectly fine when accuracy is not important, but when it's
important that the task run exactly every second (for example), `sleepSync`
should be used.  `sleepSync` helps avoid
[jitter](http://en.wikipedia.org/wiki/Jitter) and
[drift](http://en.wikipedia.org/wiki/Drift_%28telecommunication%29).


## Design Approach

While working on a [very simple robot](https://www.youtube.com/watch?v=91ZdHdaBwLw)
my code became quite a mess after tacking on things to deal with hardware I was
adding as I built it.  I decided to re-write it in a more task-oriented
fashion, using a finite-state-machine.  However, I couldn't find a task schedule
library that did all the things I wanted to do.  In the spirit of NIH (actually,
I just thought it would be fun to try writing my own) I decided to write one.

# Time Math

## Simple comparison
Many libraries I've seen use simple, easy to understand time math like:
```
if (now() >= taskScheduledTime) // it's time to run the task
```
This works great for a while, until variables overflow.  Let's assume both the
variable and the function return type are 32 bit unsigned integers.
If `taskScheduledTime` is 2^32-1 (4294967295), the maximum value that can be
stored, then the `if` statement above will only ever be true if `now()` is exactly
equal to `taskScheduledTime`, since it can't be any higher.
If the `if` statement doesn't run at exactly the right time, it'll miss the task.
If the `if` statement is changed to a busy loop:
```
while (now() < taskScheduledTime) {
  // It's not time yet.
}
// it's time to run the task
```
If this code isn't running at exactly the time that `now()` is equal to
`taskScheduledTime`, then the task won't run.  This could be because an
interrupt was being processed, or another task was taking too long, or any
number of other things.

The example above used exactly 2^32-1 and signed integers to demonstrate, but
the problem can still occur for if `taskScheduledTime` is high, close to 2^32-1,
it's just increasingly less likely the farther away it is.  For signed integers,
the problem occurs at 2^31-1 instead, but still occurs.

I hope to never see this code used for war, or in systems where bugs may
endanger anyone's life, but this is a similar bug to the one described here:
https://www.ima.umn.edu/~arnold/disasters/patriot.html

To avoid this, I'm using a different technique.  Subtraction instead of
comparison.

## Subtract and Compare to Zero

The approach I'm using is:
```
while (taskScheduledTime - now() > 0) {
 // It's not time yet
}
// it's time to run the task
```
This code requires `taskScheduledTime` and the return from `now()` to be 32 bit
*signed* values.

Similar to the previous example, let's say `taskScheduledTime` is the highest
value it can be, 2^31-1 (2147483647).

Walking through what happens as `now()` is below that, equal to that, and the
above that:

| now() | taskScheduledTime - now() | taskScheduledTime - now() > 0 | result |
|------:|--------------------------:|------------------------------:|--------|
| 2147483646 | 1 | true | Keep looping |
| 2147483647 | 0 | false | Exit the loop (and execute the task) |
| -2147483648 | -1 | false | Exit the loop (and execute the task) |

Note the strange value for `now()` in the last row of the table.  Since `now()`
is returning signed 32 bit integers, and 2147483647 is the highest value that
type can store, adding one more to it gives a large negative number.  Because of
the way signed integer math works, the result of the subtraction is -1.

All of this means that the task scheduler has plenty of time to recognize that
the task needs to be executed, virtually eliminating the problem described
above.

You may be asking "but how much time does it have?" and "does this affect how
far into the future tasks can be scheduled?"  Those are good questions.

### How much time does the scheduler have to notice the task?

Assuming the clock is in microseconds (since this is the worst case),
 if `taskScheduledTime` is 2147483647, then `taskScheduledTime - now() > 0` will
 return false (meaning it's time to execute the task) until `now()` returns 0.

| now() | taskScheduledTime - now() | taskScheduledTime - now() > 0 | result |
|------:|--------------------------:|------------------------------:|--------|
| -1 | -2147483648 | false | Exit the loop (and execute the task) |
| 0 | 2147483647 | true | Keep looping |

This means it has 2147483648 microseconds (about 35 minutes) to notice the
task.

### How far into the future can tasks be scheduled?

Since the positive and negative side of signed integers is almost symetrical
(it's off by 1), the answer is the same as the previous question: about 35
minutes.

If the clock is changed to use milliseconds instead, then it has 1000 times
longer, about 24 days.

## TODOs
* Add `sleep` functions for microseconds
  * Maybe also remove the millisecond versions, now that the `MS` constant is
 there.
* Add function to cancel a scheduled task
* Implement priority
  * Tasks that would otherwise be scheduled before a higher priority task, but
have a max runtime that indicate they wouldn't finish in time for the higher
priority task, should either be run early (if allowed) enough that that doesn't
happen, or shouldn't be run until after the higher priority task.
  * Tasks would have some flag that specifies that they are allowed to be run
early.
* Make clock (ie. `micros()`) configurable.
  * Using a microsecond clock and 32-bit unsigned integers limits how far in
the future tasks can be scheduled to approximately 2^31 microseconds, or a bit
shy of 35 minutes.  When only millisecond accuracy is required, using a
millisecond clock would increase this to almost 25 days.
* Allow WHEN/UNTIL bool to be updated by interrupts
  * The busy-spin in execNextTask means the bools aren't being checked, so it the
busy-spin goes on for quite a while, WHEN/UNTIl tasks won't be executed when
perhaps they should have been. (Particularly if they were high priority.)
  * This conflicts slightly with my intent to move the busy-spin loop to the
last possible moment in the code, to make the actual invocation time of task
closer to when it was scheduled.  I may be able to alleviate this by having two
stages to the busy spin:
    1. A first loop that checks all the WAIT/UNTIl tasks to see if any should run,
possibly changing the nextTask if there is. (And also tracks how long iterations
of this loop take)
    2. A second loop that is a simply short busy-spin like I have now. Once there
 isn't enough time for the first loop to run, the second loop takes over.
* Make the task scheduling algorithm overridable
  * Task scheduling is a hard problem, and finding a "one size fits all" solution
is even harder. Allowing users to implement their own alleviates this.
* Implement termination of lower priority tasks
  * This is very much a pie-in-the-sky idea still
