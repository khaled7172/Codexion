*This project has been created as part of the 42 curriculum by khhammou*

# Codexion

Concurrency simulation in C using POSIX threads, mutexes, and condition variables.

## Description

Codexion is a simulation of the classic Dining Philosophers problem adapted to a coding workspace. A group of coders sit in a circle around a shared quantum compiler. To compile their code, each coder needs two USB dongles simultaneously — one from their left, one from their right. Since adjacent coders share a dongle, they must coordinate access without communicating directly.

The simulation models three real concurrency challenges: deadlock prevention, starvation prevention, and race condition elimination. Each coder runs in its own POSIX thread. A separate monitor thread watches for burnout — if any coder goes too long without compiling, the simulation stops. Two scheduling policies are supported: FIFO (arrival order) and EDF (earliest deadline first).

The cycle for each coder is: acquire both dongles → compile → release both dongles → debug → refactor → repeat.

## Instructions

**Compile:**
```bash
make
```

**Run:**
```bash
./codexion <coders> <burnout_ms> <compile_ms> <debug_ms> <refactor_ms> <must_compile> <cooldown_ms> <fifo|edf>
```

**Arguments:**

| Argument | Description |
|---|---|
| `coders` | Number of coders (and dongles) |
| `burnout_ms` | Max milliseconds a coder can go without compiling |
| `compile_ms` | Time to compile in milliseconds |
| `debug_ms` | Time to debug in milliseconds |
| `refactor_ms` | Time to refactor in milliseconds |
| `must_compile` | Stop when every coder hits this compile count (0 = run until burnout) |
| `cooldown_ms` | Milliseconds a dongle must wait after being released before it can be taken again |
| `scheduler` | `fifo` for arrival order, `edf` for earliest deadline first |

**Examples:**
```bash
# 5 coders, 800ms burnout, must compile 3 times each
./codexion 5 800 100 80 80 3 0 fifo

# 3 coders running until burnout, EDF scheduling
./codexion 3 500 150 50 50 0 20 edf

# 1 coder edge case
./codexion 1 200 50 50 50 0 0 fifo
```

**Expected log format:**
```
0 1 has taken a dongle
1 1 has taken a dongle
1 1 is compiling
101 1 is debugging
181 1 is refactoring
201 2 has taken a dongle
...
800 3 burned out
```

**Clean:**
```bash
make clean   # removes object files
make fclean  # removes object files and binary
make re      # full rebuild
```

**Check for data races and memory leaks:**
```bash
# Build with debug symbols first
cc -Wall -Wextra -Werror -pthread -g *.c -o codexion_debug

# Memory leaks
valgrind --leak-check=full ./codexion_debug 3 8000 100 80 80 2 0 fifo

# Data races (use large burnout when running under valgrind — it slows execution ~20x)
valgrind --tool=helgrind ./codexion_debug 3 8000 100 80 80 2 0 fifo
valgrind --tool=drd ./codexion_debug 3 8000 100 80 80 2 0 fifo
```

## Blocking Cases Handled

**Deadlock prevention**

Deadlock in the dining philosophers problem happens when every coder simultaneously picks up their left dongle and waits forever for their right — a circular dependency with no way out (Coffman's conditions: mutual exclusion, hold-and-wait, no preemption, circular wait). The fix breaks circular wait by enforcing a global dongle acquisition order: always acquire the lower-id dongle first. Since every coder follows the same order, no cycle can form.

```c
if (coder->left->id < coder->right->id)
    { first = left; second = right; }
else
    { first = right; second = left; }
```

**Starvation prevention**

Without a fair scheduling policy, a coder could be perpetually skipped while its neighbors keep acquiring dongles. This is prevented by the priority queue (min-heap) inside each dongle. Every request is given a priority key. Under FIFO the key is a global ticket — strictly arrival order, so no coder can be skipped indefinitely. Under EDF the key is the coder's burnout deadline — the coder closest to dying always goes first.

**Dongle cooldown**

After a dongle is released, it cannot be taken again until `cooldown_ms` have elapsed. This is enforced by `ready_at = get_time_ms() + cooldown` on each release. The acquisition loop checks `get_time_ms() >= dongle->ready_at` before allowing a take.

**Precise burnout detection**

The monitor thread polls every 1ms using `ft_usleep(1, sim)`. For each coder it checks `now >= last_compile_time + time_to_burnout`. When burnout is detected, `sim->stop` is set atomically under `stop_lock` and the burnout message is printed immediately. The subject requires the burnout log within 10ms of actual burnout — 1ms polling satisfies this on any reasonable system.

**Log serialization**

All output goes through `log_state` which locks `log_lock` around every `printf`. This guarantees no two state messages ever interleave on a single line, regardless of how many threads are running simultaneously.

**Single coder edge case**

With one coder, left and right dongle are the same object. The coder can never compile (needs two dongles, only one exists). The code detects `left == right`, takes the one dongle, logs it, then waits for the burnout signal rather than deadlocking trying to acquire the same dongle twice.

## Thread Synchronization Mechanisms

**Three mutexes, each with a single responsibility:**

`stop_lock` protects `sim->stop`. Every read and write of the stop flag goes through this mutex. This includes the monitor setting it, and `sim_stopped()` reading it. Nothing else shares this mutex, which prevents lock-order inversion.

`log_lock` protects `printf`. Locked for the duration of every print in `log_state` and `set_burnout`. Never held at the same time as any other mutex.

`state_lock` protects all dongle state (`in_use`, `ready_at`, the priority queues) and the per-coder fields read by the monitor (`last_compile_time`, `compile_count`, `global_ticket`). This is the main synchronization object for the acquisition and release protocol.

**Per-coder condition variables (`pthread_cond_t cond`):**

Each coder has its own condition variable. When a coder cannot acquire a dongle (it is in use, its cooldown has not expired, or it is not at the front of the queue), it calls `pthread_cond_wait(&coder->cond, &state_lock)`. This atomically releases `state_lock` and puts the thread to sleep. When a dongle is released, `pthread_cond_broadcast` wakes the specific coder at the front of that dongle's queue. Using per-coder condition variables means wake-ups are targeted — only relevant waiters are woken.

**How `pthread_cond_wait` prevents busy-waiting:**

Without condition variables the acquisition loop would spin continuously, burning CPU while waiting. `pthread_cond_wait` blocks the thread entirely — it consumes no CPU until signaled. The wait is always inside a `while(1)` loop that re-checks all conditions after waking, because the OS can wake a thread spuriously (for no reason). The pattern is: push to queue → wait → re-check → break if conditions met.

**Happens-before guarantees for `last_compile_time` and `compile_count`:**

These fields are written by coder threads in `do_compile` and read by the monitor thread in `check_burnout` and `all_done`. Both sides lock `state_lock`, establishing a happens-before relationship: any write that completed before unlock is guaranteed visible after the next lock by any thread.

**The `sim->stop` read in `acquire_one`:**

Inside `acquire_one`, `sim->stop` is read directly while `state_lock` is already held. This is safe because the monitor always sets `sim->stop` under `stop_lock` first, then calls `wake_all` which locks `state_lock` to broadcast. Since `state_lock` must be released before `wake_all` can acquire it, any read of `sim->stop` that happens after the broadcast is guaranteed to see the updated value.

## Resources

**Classic references:**

- Dijkstra, E.W. (1971). *Hierarchical ordering of sequential processes.* — original dining philosophers formulation
- Coffman, E.G. et al. (1971). *System deadlocks.* ACM Computing Surveys — the four Coffman conditions for deadlock
- POSIX Threads Programming — Lawrence Livermore National Laboratory: https://hpc-tutorials.llnl.gov/posix/
- `man 3 pthread_mutex_lock`, `man 3 pthread_cond_wait` — the actual POSIX specs
- Valgrind Helgrind manual: https://valgrind.org/docs/manual/hg-manual.html

**How AI was used:**

AI was used throughout this project as a debugging and review tool.
