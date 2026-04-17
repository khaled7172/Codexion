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
## Types of printf in C
int x = 4;
printf("Hello %d\n", x);
it prints directly to stdout
sprintf
char buf[100];
sprintf(buf, "Value = %d", x);
it writes formatted string into a buffer (char array)
and doesnt print anything and pretty unsafe due to buffer overflow risk
use snprintf instead
snprintf(buf, sizeof(buf), "Value = %d", x);
fprintf
same as printf but here you choose where it writes
stdout, stderr, file
fprintf(stdout, "Hello %d\n", x);
fprintf(stderr, "Error: %d\n", x);
My program works like this:
First i check if i have exactly 9 args
then i allocate space for simulation struct
then i use memset on it to zero every byte so no garbae values exist before init
then i call init_sim which should initialize everything
first i parse the args i check that they all are numbers
i check the first 7 args, not checking the last arg for the scheduler, nor the first arg which is the program name
then i check last arg is a real scheduler either fifo or edf
we initialize the stop to 0
and get the time now for the start
global ticket initialized to 0
then we initialize the mutexes:
A pthread_mutex_t is a lock that prevents multiple threads from accessing shared data at the same time
pthread_mutex_init() initializes a mutex
we give it address of mutex inside your struct (&sim->stop_lock)
check if initializing stop_lock failed
then create a seperate lock for log_lock and state_lock
Each lock protects a critical section
then we malloc space for dongle array the size of num of coders entered by users
so for n coders we have n dongles
each dongle has id same as its position in the array
initially none are at use
and none are ready_at (no cooldown since not used yet)
then we initialize a heap (priority queue)
we allocate memory for an array
Each element is t_waiter
and total size is number of coders
initially heap has 0 elements
then we store max capacity
data stores the array
waiter takes a lock a coder id and a pthread_cond_t
A min heap is a binary tree stored as an array
where smallest element is stored at the top (index 0)
example:
elements with following keys: 10, 5, 20, 2

        2
       / \
      5   20
     /
   10

Array form : [2, 5, 20, 10]
Rules for min heap are:
for every node
parent <= children
so root is the smallest
when you push waiters with a key
your heap decides who gets the dongle next
with heap finding elements is O(1)
while without heap finding smallest is O(n)
and with heap insert/remove is O(log n)
then we initialize the coders
each coder has id of its position in array i + 1 so we start with coder 1
compile_count initially 0 for all
last compile time initially equal to the start time
and each coder's left anf right dongle are his i and (i + 1) % number of coders to wrap around
so for 3 coders
coder 3 has left dongle as 3 and right dongle as 0 as opposed to 4 that doesnt exist
and every coder is given access to the sim struct
Every coder has a condition variable for coder i
pthread_cond_t cond;
is a condition variable and used to sleep and wake threads
pthread_cond_init() takes as args address of condition variable to initialize and default attribute NULL
A condition variable lets a thread:
1. sleep
pthread_cond_wait(&conde, &mutex);
Thread blocks(sleeps) and releases mutex while sleeping
2. wake up
pthread_cond_signal(&cond);
or
pthread_cond_broadcast(&cond);
wakes one or all waiting threads
initially all condition variables has null basically
if it doesnt work we free the struct entirely
After that we start the threads:
launching the monitor thread and the N coder threads
First we create the monitor thread (1 thread)
and run the function monitor_routine
This function's job is to watch the simulation and decide when to stop it
First as a rule of thumb thread fucntion always take void * as arg and you convert it to your real type
then we create an infinite loop that runs forever until a stop condition is met
then we do usleep(1000);
sleep for 1000 microseconds = 1ms
This exists because without it
cpu usage goes up to 100%
so this makes it like check every ~1ms
The core logic is there's 2 conditions checked inside it
check_burnout which checks if any
code exceeded:
last compile time + time to burnout
if yes it prints x died
And all done which checks if all coders reached
compile count >= must compile
if yes simulation should stop normally by using
set_stop(sim)
inside this we change stop flag to 1
and wakes all sleeping threads (cond_signal)
without wake threads stay blocked forever
first we take a snapshot of the current time in ms
then we loop all over the coders and compute each one's deadline
which is when he'd be dead
computing deadline is done by adding last compile time with time to burnout
meaning the coder must compile again before this timestamp or he dies
if now(current time) is greaten than deadline then its too late and coder is dead
in the case now >= deadline(death)
we cant having multiple threads detecting burnout at the same time so we use a mutex
pthread_mutex_lock(&sim->stop_lock)
this ensures only one thread sets stop and only one prints
after we lock the mutex we check if the flag wasnt already 1 to set it to 1
and lock the log_lock mutex to print x burned out then unlock the mutex which is done to prevent mixed output from multiple threads
then we unlock the mutex stop_lock which we locked when we found out this time now >= deadline so only this thread could stop and him to print he's burned
Else if nobody died the monitor keeps looping, infinite loop

all_done(sim) function checks if simulation should stop because all coders finished their required work
if must compile = 0 meaning no limit is required we never stop based on completion
so must_compile = 0 leads to an infinite simulation where only burnout stops it
then we loop through the coders
If even ONE coder hasn't finished we arent done yet
If loop completes we return 1 to mean ALL coders reached compile count
when we call set_stop function
we lock mutex stop_lock
to protect access to sim->stop
then we set stop flag once to 1 if it wasnt already 1
to prevent multiple threads from resetting it
and then we unlock the mutex to allow other to be able to read sim->stop
then we call wake_all which is important because otherwise threads may stay blocked forver
it means to mark simulation as finished then wake everyone so they can exit

wake_all
first it locks state_lock mutex
which is required because condition variables must be used with a mutex to prevent races with pthread_cond_wait
then loops over the coders and signals each coder
pthread_cond_signal(&sim->coders[i].cond)
which wakes up that coder if it is sleeping
if not sleeping signal is ignored which is fine
then after we wake everyone up we unlock the mutex state_lock
Thread in your code often do:
pthread_cond_wait(&coder->cond, &sim->state_lock);
when set_stop() happens:
sim->stop = 1
wake_all() signals everyone
sleeping threads wake up
They check if sim_stopped(sim)
threads exit cleanly
you could use pthread_cond_broadcast() instead which would wake all threads instead of looping manually over them

This background thread will watch everything(burnout, stop condition)
then we loop on the num of coders
create N threads (one per coder)
and Each iteration we start a thread that runs coder routine for coder i
we store thread id in thread var
thread takes Null by default
coder_routine is the function to be run by the thread
This function returns void * because pthread expects a generic return type
arg is a raw pointer passed at thread creation
then we convert generic pointer into your struct pointer which is for the coder
we do an initial desync (even IDs)
we check if coder id is even and if so we do a sleep on them
this is done so even threads wait before starting 
and to reduce immediate lock contention
without this all threads hit dongles at the same time and we would have deadlocks or heavy blocking
Then we do a loop to make the thread run until global stop flag becomes true


sim_stopped function
it takes a ptr to the shared sim struct
creates a local copy int val as a temp variable to store value safely and needed because mutex is released before returning
first we lock the mutex stop_lock
to prevent other threads from modifying it while reading
and ensure no race condition occurs
store shared value of stop flag into local variable val
release the stop_lock mutex
so other threads can access sim->stop
keeps critical section extremely short
then return the safe snapshot of the stop flag
its only done so one thread cannot read sim-stop while another is writing it which creates a data race which leads to inconsistent behavior
so basically we check if stop flag not 1 in the bigger function then we enter the loop

sched_key returns a long as a priority value
it takes coder thread requesting resources and dongle which is resource queue being used
we have 2 local var
shared sim struct
key which is the computed priority value
we get sim = coder->sim which is a shortcut to shared config (scheduler type, timing, counters)
first we check if the scheduler is edf(earliest deadline first)
then priority would be based on urgency
key value here is like the deadline 
key = last compile time + time to burnout
last compile time is last time coder worked
and time to burnout is the max allowed idle time
sum is equal to the absolute deadline
like when will coder die
lower deadline = higher priority
then we add a tie-breaker which is key * 1000 + coder id
we multiply to ensure deadline dominates sorting and we add coder id to break ties between equal deadlines
so final priority would be earliest death time first then lower id
if its fifo on the other hand we use ticket scheduling
Each request gets a monotonically increasing number
First come first served
global ticket++ ensures ordering
so smaller ticket = higher priority
and we unlock state lock mutex inside the function
we do void(dongle) to ignore the paramter since dongle isnt used in fifo mode and at the end we return fifo key
acquire_one first puts the key from sched_key function inside the waiter and takes its current condition
and then we call a heap push which calls sift up function that does:
sift up is a heap bubble up operation used to maintain a min-heap property after inserting a new element
After you insert a new t_waiter at the end of the array, this function moves it upward until the heap rule is restored which is in a min-heap every parent <= child key
so the smallest key always ends at index 0
so it operates on a heap struct
it modifies heap in place
New element is always inserted at the end
i points to the newly inserted node
example:
index: 0 1 2 3
data:  [A B C X]  ← X is new
so i = 3
we enter a while loop which stops when reaching root(index 0)
root has no parent
we compute parent index
parent = (i - 1) / 2;
Binary heap rule:
left child = 2*i + 1
right child = 2*i + 2
parent = (i - 1) / 2
then we check if the parent is already smaller or equal to our key in which case we stop since heap is valid and it satisfies min-heap condition
else we do a swap operation
we save parent key in a variable
we make the parent value equal to the data we have
and then make the data index we have equal to the tmp value
basically swapping child and parent
and moving smaller key upward
so if child has higher priority (smaller key) it rises
then we continue checking new position
and repeat until heap property is satisfied or root is reached
priority queue is:
insert new request at bottom
repeatedly compare with parent
swap upward until correct order
this is important because the heap will decide which coder gets a dongle first, based on edf or fifo key
back to the heap push
it inserts a new element into a binary min heap priority queue
it adds a t_waiter into heap while preserving heap ordering
first we do a capacity check to prevent writing past allocated memmory
if heap is full insertion is ignored
we place new element at the next free slot
this temporarily breaks heap order
and we increase size variable of the heap to include the new element in its count
so logical size is updated before reordering
then we restore heap property by calling sift_up(h) which fixes ordering
by moving new element upward and ensures min-heap property
parent.key <= children.key
then inside the acquire one we enter through an infinite loop
first we check if stop flag is 1 and if true we unlock the state lock mutex and exit
else

get_time_ms returns long timestamp in miliseconds
first we create timeval struct which is defined in sys/time.h
and it stores second and microseconds
so time is split into whole seconds and fractional seconds
step 2 is to get the system time
we fill tv struct with current system time
second arg is NULL meaning no timezone info needed
then sec stores seconds since start
and tv_usec stores microseconds within that second
then we convert to miliseconds
sec to miliseconds is * 1000
microsec to milisecond is / 1000
then u add both to get total ms

heap_pop() removes root element at index 0, last element is moved to the top,
then uses sift down to push it down until heap property is valid again:
parent key <= children keys
root of heap is always index 0 this is where invalid element is placed after a pop
we start an infinite loop and the loop continues until heap property is restored
stops manually using break
first we compute children using the binary heap rule:
left child = 2*i + 1
right child = 2*i + 2
so from any node u can reach children
we start by assuming parent is smallest, then compare with children
compare left child( if it exists and its key is less than current parent key)
it becomes the new smallest
then we do the same logic for the right child
then we have the smallest among them
if heap is already valid and parent is smallest we break
else we swap with smallest child
move smallest up and push current node down
restoring partial heap correctness
then we do i = smallest and continue from new position and repeat until correct place is found
inside heap_pop() we store top value as h->data[0]
because we will overwrite it now
then we remove last element logically by reducing the size by 1 element
h->size--
then we check if the heap still has elements
because if it becomes empty there's nothing else to do
then we take last element and put it at the top
it breaks the heap property temporarily since that element might be bigger than its children and thats where we call sift down to take care of this problem
heap_pop() returns element it popped also
when we pop its for choosing a coder to get the dongle

acquire one is for one coder trying to acquire one dongle
first we create a shortcut pointer to coder->sim struct
then we prepare the waiter
we give him a key based on priority fifo or edf
we give it a coder id and a cond since it is how we wake it up later
then we call heap push which pushes this waiter into the heap
heap sorted by the keys
then we create an inf loop that blocks until u are allowed to take the dongle
if stop flag is 1 we unlock the mutex(state_lock) and exit immediatly
if not stop, we check if we can take the dongle
all these conditions must be true
the queue size must be > 0 
your coder must be at the top of heap
if not at top wait your turn
then we check if its in use
then we check if cooldown for the dongle passed
and if all are true we break from the infinite loop
If any condition fails we wait
pthread_cond_wait()
This releases state_lock 
sleeps
wakes up when someone signals your condition
Re-locks state_lock automatically
when a coder wins they are removed from heap
and we mark dongle as in use to lock the resource logically

log_state
This function is your logging gatekeeper
it ensures logs dont interleave
and nothing prints after simulation stops (except death)
it takes sim struct coder id and a msg to print
first we lock the log_lock mutex
so only one thread can print at a time
if simulation stopped (flag = 1)
allow only "burned out"
and block anything else
so burned out still prints
and is compiling and all others are suppressed
we unlock the mutex and exit
else
we compute timestamp which is current time - start time
to give a relative timestamp
then we print [timestamp] [coder id] [msg]
then we unlock the mutex

release_one function frees a dongle and wakes next eligible coder
called when a coder finishes using a dongle
first we mark dongle as free in_use = 0
then we set a cooldown for it current time + cooldown provided by user
so coders wait until curr time >= ready_at time
then we wake one waiting coder thats at the top of the heap
then create a infinite loop so even if wrong thread wakes up it checks condition and goes back to sleep
optimized to only wake the correct one

acquire both dongles function
it tries to acquire both left and right dongles
when num of coders we 1 we have a special case where right and left dongle are the same meaning coder has 1 resource and this can never compile
we lock shared mutex state_lock
try to grab the one dongle and if sim stopped we exit
it prints he got one
and then he wants forever until monitor signals stop
so he holds one dongle and dies
then we unlock the state_lock mutex and exit
first = smaller id(first dongle)
second = bigger id(second dongle)
it prevents deadlock since without this
coder A takes left 
coder B takes right
both wait forever and deadlock
but with ordering everyone locks resources in the same order
then we lock mutex state_lock
and try to acquire the 1st dongle
and then try to acquire the second dongle
if it fails to acquire second we release first dongle
unlock state_lock mutex and exit
if you succeed and have both you unlock the state lock mutex
then we print that the coder has both resources

do compile function
first we get simulation pointer to struct
then we print coder is compiling
this happens before sleeping so timestamp marks start of compile not the end
then we simulate compile time by doing ft_usleep to block the thread for time to compile miliseconds
so during this time coder is busy and monitor thread can still run and kill him if needed
then we update last compile time to curr time
during compile if time to burnout becomes less than time to compile coder can die while compiling
then we increment the compile count

release both dongles function
its about giving resources back safely and waking the right waiting threads
first we get sim struct
lock state lock mutex
we release left dongle
inside release one function we do dongle in use = 0 readyat = now + cooldown and if queue isnt empty we signal next waiter
back to our function
we release right dongle if diff from left dongle
it avoids bug when 1 coder has same dongle to not release twice
then we unlock the state lock mutex

do debug function
we print it is debugging and then make it sleep that amount of time simulating a debugging phase
during this time coder is not holding any dongles and can still be killed by monitor if approaching burnout

same for the do refactor function

coder routine function
it is passed to pthread_create to create a thread for each coder
and run by each coder independently
first even ids wait before starting so not all threads start at the same time and try to grab dongles
then thread runs forever infinite loop until sim->stop = 1
wait until both dongles acquired
compile
release resources (not used during compile time)
after each step debugging or refactoring 
monitor thread can kill them so we keep checking the flag
that was the start threads code

now we enter join threads function

join thread function
pthread_join means block here until this thread finishes execution
so the calling method (main) waits until this thread finishes execution
we wait for monitor thread
it blocks until monitor routine returns null
and that only happens when there's burnout or all are done
then after we join the monitor thread we loop over coders and wait until coder re=outine returns
join is blocking and sequential
if coder 3 is stuck program waits forever here
so if the program stop its because a coder never exits or a condition variable never wakes
if we dont use join threads we create zombie threads running in the background
causing memory leaks and inconsistent termination

free all function
cleanup/teardown function
the purpose is to release all allocated resources and destroy synchronization primitives
if coders arent free we loop over them and destroy the coder's conditional variables one by one
then we free the coders
pthread_cond_destroy() is only safe if
no thread is waiting on it and no thread will signal it again
then we free dongle queues
if dongles exist we loop over num of coders
and since each dongle has a priority queue min-heap we free internal heap array then free dongle array by itself
then we destory the three mutexes we use
stop lock log lock and state lock and they must be destroyed only when no thread is running and no one can lock/unlock anymore
then we free simulation struct
Each dongle has a priority queue min-heap
since each queue stores t_waiter entries which has coder if key and cond
so each dongle is a shared resource with a waiting list ordered by priority
why per-dongle heap exists
with heap 
each dongle independently decides who should get it next
based on scheduling rule
so each dongle becomes a scheduler aware mutex with ordering
the heap guarantees at top we have highest priority code
heap push inserts requester
heap pop removes the one that got the resource
when a coder requests a dongle we do heap push so hes added to queue
then sleeps till hes at top and dongle is free and cooldown finished
when hes granted access hes popped heap pop
and hes no londer in the queue
why NOT one global heap?
well because each dongle is independent and it would mix unrelated resources and just very complex to do

&sim->coders[i] is a pointer to THIS coder's data
example if num of coders is 3 we get:
1 monitor thread
and 3 coder threads
All running concurrently
main thread
   |
   |--> monitor thread (checks burnout)
   |--> coder 1 thread (loops: acquire → compile → debug → refactor)
   |--> coder 2 thread
   |--> coder 3 thread
reason for adding this header:
# define _XOPEN_SOURCE 600

Why: usleep() is a POSIX extension, not part of the C standard.
When you compile with -std=c99 or -std=c98, the compiler hides everything that is not pure ISO C unless you tell it otherwise. _XOPEN_SOURCE 600 unlocks POSIX.1-2001 extensions (which includes usleep, gettimeofday, pthread_*).
Without it you get:
error: implicit declaration of function 'usleep'
Placing it in the header means every .c file that includes codexion.h automatically gets it.



# Instructions

# Resources

