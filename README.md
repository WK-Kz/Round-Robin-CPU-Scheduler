# Round Robin CPU Scheduler

## Introduction
The RR (Round-Robin CPU Scheduler) is a scheduler in which the CPU will run a certain a process for
some specific time quantum which is determined by the user. Upon arrival of the process it will be
run for that specified time quantum. This process is repeated with any new processes that are put
into the queue.

## Assignment
Create a CPU Scheduler from scratch that will read a csv and implement this algorithm.

## Executing

Change the arrival/burst time in the pid.csv file
<br>

Compile the program again if necessary.
```
g++ -o output Round_Robin.c
```

Execute

```
./output
```

