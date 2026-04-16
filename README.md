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

First we parse only integers, then we create dongles array and initialize each mutex and create the coders array and assign left/right dongles (circularly)

Every coder needs left dongle and right dongle
A dongle is a shared resource meaning a lock prevents 2 coders using it at the same time
Every coder has an id
last_compile_time to detect burnout later
compile_count to count how mant cycles done
each coder points to shared dongles but dont own any
We create N dongles
if 5 coders then we create 5 dongles
initially each dongle gets a lock
(i + 1) % n wraps the last coder back to the first dongle
so coder 0 gets dongle 0 and dongle 1
coder 1 gets dongle 1 and dongle 2
coder 2 gets dongle 2 and dongle 3
coder 3 gets dongle 3 and dongle 0
(3 + 1) % 4 -> 4 % 4 = 0
The reason we store a sim inside coder struct is because we inside a coder thread we will need access to time_to_burnout, stop flag, and log mutex, and scheduler type and all coders/songles
but inside a thread function we only get:
void *arg
So instead of passing 10 variables, we pass:
t_coder *
and from there
coder->sim to get full access to everything
## What a coder should do
Take left dongle
Take right dongle
compile
release both dongles
debug
refactor
repeat
pthread_cond_wait() puts thread to sleep until something changes better than infinite loop
#add edge case for 1 coder

# Instructions

# Resources

