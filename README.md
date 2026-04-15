*This project has been created as part of the 42 curriculum by khhammou*

# Codexion
Concurrency simulation in C using POSIX threads, mutexes, and resource scheduling

-pthread means to turn on threading mode for the whole build, allows you to use pthread_create and pthread_join and the mutex lock

## Context switching
cpu executing a thread A for x time then pausing A and running B then pausing B and running C and so on

## Time Slicing
Each thread gets a small time slice then the OS switches
when using usleep(1000) to make thread sleep for 1 ms it is not guaranteed exactly 1 ms sleep, its like sleep at least 1 ms, but the OS might wake it later than 1ms and take more time

## Thread States
Running:
Currently executing on CPU
Ready:
Waiting for CPU
Blocked:
Waiting for something like mutexes, sleep, or condition variables

## Mutex
When a thread is locked by a mutex, it doesn't run until unblocked

## Condition variable effect
this is better than busy waiting, the while loop
pthread_cond_wait() makes the thread blocked and sleeping efficiently
BUT it releases the mutex and waits for signal

## Race condition happen because of an abscence of mutex
Thread A and B check for dongle
Thread A takes it then thread B also takes it
This happens because of context switch between steps


# Description
Expected input format:
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler
# Instructions

# Resources

