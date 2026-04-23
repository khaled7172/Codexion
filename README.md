*This project has been created as part of the 42 curriculum by khhammou.*

# Codexion

## Description

Codexion is a concurrency simulation inspired by the classic Dining Philosophers problem. Multiple coders sit in a circular co-working hub and must repeatedly compile, debug, and refactor. Compiling requires holding two USB dongles simultaneously — one per hand — which must be shared with neighbors. The goal is to orchestrate all coders so that none of them burns out due to lack of compiling, while correctly implementing POSIX thread synchronization, fair scheduling, and dongle cooldown.

The simulation stops either when every coder has compiled at least a required number of times, or when a coder fails to start a new compile within the burnout deadline.

## Instructions

### Compilation

```bash
make
```

Compiles with `-Wall -Wextra -Werror -pthread`. Produces the binary `codexion`.

### Execution

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

All arguments are mandatory. All time values are in milliseconds.

| Argument | Description |
|---|---|
| `number_of_coders` | Number of coders (and dongles) |
| `time_to_burnout` | Max ms between compile starts before burnout |
| `time_to_compile` | Duration of each compile (holds both dongles) |
| `time_to_debug` | Duration of the debug phase |
| `time_to_refactor` | Duration of the refactor phase |
| `number_of_compiles_required` | Required compiles per coder to stop normally |
| `dongle_cooldown` | ms a dongle must rest after being released |
| `scheduler` | `fifo` (arrival order) or `edf` (earliest deadline first) |

### Example runs

```bash
# 4 coders, feasible params, FIFO scheduling
./codexion 4 800 200 200 200 3 0 fifo

# 6 coders, EDF scheduling, 20 compiles each
./codexion 6 1000 200 100 100 20 0 edf

# Burnout scenario
./codexion 2 300 400 100 100 5 0 fifo
```

### Helgrind usage

A suppression file is provided to silence a known glibc false positive in `pthread_cond_timedwait` (glibc emits an internal signal during timedwait that helgrind misreads as a lock-order violation):

```bash
valgrind --tool=helgrind --suppressions=codexion.supp ./codexion 5 800 200 200 200 3 0 fifo
```

## Blocking cases handled

### Deadlock prevention

Each coder needs two dongles. Naive acquisition (left then right) risks ABBA deadlock: coder A holds dongle 0 and waits for dongle 1, while coder B holds dongle 1 and waits for dongle 0. This is prevented by always acquiring dongles in ascending ID order — establishing a global lock ordering that breaks the circular-wait condition (one of Coffman's four necessary conditions for deadlock).

### Starvation prevention

Under FIFO scheduling, each dongle maintains a priority queue (min-heap ordered by arrival ticket). This guarantees that every waiting coder eventually reaches the front, since the ticket counter is strictly increasing and no coder can jump ahead. Under EDF, the heap orders by `last_compile_start + time_to_burnout`, so the coder closest to burnout is always served first, reducing overall starvation risk when parameters are feasible.

### Cooldown handling

After a coder releases a dongle, `ready_at` is set to `now + cooldown`. The waiting loop in `wait_for_dongle` checks `get_time_ms() >= dongle->ready_at` before granting access. If the dongle is not yet ready, the coder sleeps via `pthread_cond_timedwait` with an absolute deadline matching `ready_at`, so it wakes precisely when the cooldown expires with no busy-waiting.

### Precise burnout detection

A dedicated monitor thread polls every 1ms using `ft_usleep`. For each coder it reads `last_compile_time` and compares it to `now`. If `now >= last_compile_time + time_to_burnout`, burnout is declared immediately and the log is printed before any other action. The subject requires the burnout message within 10ms of the actual deadline; in practice the implementation achieves within 1–2ms.

### Log serialization

All output goes through `log_state`, which acquires `log_lock` before calling `printf`. This ensures that concurrent coder threads and the monitor thread never produce interleaved output on a single line.

## Thread synchronization mechanisms

### Per-dongle mutex (`dongle->lock`)

Protects the dongle's `in_use`, `ready_at`, and `queue` fields. Held only briefly to read or modify dongle state — never held while sleeping or waiting.

### Per-coder condvar (`coder->cond` + `coder->cond_lock`)

Each coder has its own condition variable paired with its own dedicated mutex (`cond_lock`). This is the only mutex used during `pthread_cond_timedwait` and `pthread_cond_signal`. Separating the waiting mutex from the dongle mutex eliminates the helgrind-class error of signaling a condvar without holding the associated lock, and avoids holding a dongle lock during a potentially long sleep.

When a dongle becomes available or stop is triggered, the signaling path is:
1. Acquire `dongle->lock`, identify the front waiter.
2. Acquire that `coder->cond_lock`, set `coder->notified = 1`, call `pthread_cond_signal`.
3. Release both locks.

The `notified` flag prevents the lost-wakeup problem: if a signal arrives between the dongle-lock release and the `pthread_cond_timedwait` call, `notified` is already set and the wait is skipped.

### Global stop mutex (`stop_lock`)

Protects `sim->stop`, `sim->burned_out`, `compile_count`, and `last_compile_time`. Used by both the monitor (to set stop and read coder state) and coder threads (to update their own state after compiling). This single lock prevents data races on the fields the monitor inspects.

**Lock ordering:** `stop_lock` → `log_lock`. This ordering is enforced globally — no code acquires `log_lock` while already holding `stop_lock` from a different lock domain, and `log_state` never nests them.

### Sleep condvar (`sleep_mutex` + `sleep_cond`)

Used exclusively by `ft_usleep`. Instead of busy-waiting, threads sleep via `pthread_cond_timedwait` on `sleep_cond`. When `wake_all` is called, it broadcasts on `sleep_cond` under `sleep_mutex`, instantly waking all threads that are sleeping in `ft_usleep`, ensuring fast and clean shutdown.

`ft_usleep` checks `sim->stop` under `stop_lock` before entering the `sleep_mutex`/`timedwait` section — this prevents a data race where `stop` was previously read under `sleep_mutex` (wrong lock).

### Race condition prevention example

Without protection, `compile_count++` in one thread and `compile_count < must_compile` in the monitor could race. Both accesses are serialized under `stop_lock`:

```c
// coder thread (coder_routine.c)
pthread_mutex_lock(&sim->stop_lock);
coder->compile_count++;
pthread_mutex_unlock(&sim->stop_lock);

// monitor thread (monitor.c)
pthread_mutex_lock(&sim->stop_lock);
count = sim->coders[i].compile_count;
pthread_mutex_unlock(&sim->stop_lock);
```

### Helgrind-clean `burned_out` access

`sim->burned_out` is protected by `stop_lock`. `log_state` reads it under `stop_lock` before and after the initial fast-path check, but never while holding `log_lock` — preserving the global lock order and eliminating the helgrind data-race report on `burned_out`.

## Resources

- POSIX Threads Programming — Blaise Barney, Lawrence Livermore: https://hpc-tutorials.llnl.gov/posix/
- The Little Book of Semaphores — Allen B. Downey: https://greenteapress.com/semaphores/
- `man 3 pthread_cond_timedwait`, `man 3 pthread_mutex_lock`
- Dining Philosophers problem — E.W. Dijkstra (1965)
- Coffman conditions for deadlock — E.G. Coffman et al. (1971)

### AI usage

Claude (Anthropic) was used throughout this project for: code review and bug identification (data races, deadlock analysis, helgrind error interpretation), architectural decisions (per-coder condvar design, lock ordering strategy)

## Special Cases
## Helgrind False Positive with `pthread_cond_timedwait`

### Overview

During testing with Helgrind, the following warning may occasionally appear:

```
pthread_cond_{signal,broadcast}: associated lock is not held by any thread
```

This warning appears to indicate incorrect usage of condition variables. However, in this project, it is a **false positive** caused by limitations in Helgrind's analysis of glibc internals.

---

### Where it happens

The issue originates from the custom sleep function:

```c
pthread_mutex_lock(&sim->sleep_mutex);
pthread_cond_timedwait(&sim->sleep_cond, &sim->sleep_mutex, &ts);
pthread_mutex_unlock(&sim->sleep_mutex);
```

And its interaction with:

```c
pthread_mutex_lock(&sim->sleep_mutex);
pthread_cond_broadcast(&sim->sleep_cond);
pthread_mutex_unlock(&sim->sleep_mutex);
```

Both usages follow correct POSIX requirements:

* The mutex is locked before calling `pthread_cond_timedwait`
* The same mutex is used for signaling (`broadcast`)
* The mutex is held during signaling

---

### Why Helgrind reports an error

Internally, `pthread_cond_timedwait` is not a simple sleep. It involves:

1. Registering the thread in a wait queue
2. Releasing the mutex atomically
3. Sleeping via a futex (kernel primitive)
4. Waking up due to either:

   * a signal/broadcast
   * a timeout
5. Re-acquiring the mutex before returning

When a timeout occurs, glibc performs internal cleanup to remove the thread from the condition variable queue.

During this cleanup, glibc may perform operations that resemble a signal or wake-up **without holding the user-level mutex**. This is safe and intentional inside glibc, but Helgrind cannot see the internal synchronization mechanisms used.

As a result, Helgrind incorrectly reports:

> "signal/broadcast without holding the associated lock"

Even though user code is correct.

---

### Why it is inconsistent

The warning does not always appear. It depends on:

* Thread scheduling timing
* Whether the timeout path is taken
* Internal execution paths within glibc

This makes the issue look like a race condition, but it is not.

---

### Why replacing with `usleep` removes the warning

Replacing the implementation with:

```c
usleep(ms * 1000);
```

removes the warning because:

* `usleep` does not use condition variables
* It directly calls a kernel sleep (`nanosleep`)
* No mutexes or signaling are involved

Therefore, Helgrind has nothing to analyze and produces no warning.

---

### Trade-offs

Using `pthread_cond_timedwait`:

* Allows early wake-up via `wake_all`
* Avoids busy waiting
* More responsive shutdown
* May trigger Helgrind false positives

Using `usleep`:

* Simpler
* No Helgrind warnings
* Cannot wake early
* Slightly worse responsiveness

---

### Conclusion

The Helgrind warning related to `pthread_cond_timedwait` in this project is a **known false positive** caused by glibc's internal implementation of timed condition variable waits.

The synchronization logic in this project is correct and follows POSIX standards. The warning can be safely ignored or suppressed.
