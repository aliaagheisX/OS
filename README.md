## Table of Contents
- [Data Structures used](#data-structures)
- [Algorithm Explanation and Results](#algorithm-explanation-and-results)
  - [Highest Priority First](#highest-priority-first)
  - [Shortest Remaining Time Next](#shortest-remaining-time-next)
  - [RoundRobin](#Round-Robin)
  - [Memory Allocation Algorithm](#memory-allocation-algorithm)
    - [First Fit Algorithm](#first-fit-algorithm)
    - [Buddy Memory Allocation Algorithm](#buddy-memory-allocation-algorithm)
- [Assumptions](#assumptions)

## Data Structures

| Algorithm                      | Used DS           | Notes                             |
| :----------------------------- | :---------------- | :-------------------------------- |
| Highest Priority First (HPF)    | Priority Queue    | Priority: Priority of the process  |
| Shortest Remaining Time (SRTN) | Priority Queue    | Priority: Remaining time |
| Round Robin (RR)               | Circular Queue    | Inside the implementation, when a process arrives, it is inserted into a queue. When it is stopped, it is dequeued and enqueued again. Once it finishes, it is dequeued. |
| First Fit (FF)                 | List              |                                   |
| Buddy                          | List of lists     | The list of lists consists of 11 lists, each representing a specific size that is a power of two starting from 2^0 to 2^10. Each list is implemented as First Fit (FF). |



## Algorithm Explanation and Results

Note that we have isolated the functions for running, stopping, and continuing in separate functions.

### Highest Priority First

The code first checks if there is no currently running process (`RunningProcess` is `NULL`) and the ready queue is not empty (`!IsEmpty(ReadyQueue)`). If both conditions are met, the highest priority process at the front of the queue is retrieved using the `peek(ReadyQueue)` function, and its address is assigned to the `RunningProcess` pointer.

Then, the state of the process is updated to "started" and the `runProcess()` function is called to execute the process. It is assumed that the `runProcess()` function takes a process pointer as its parameter and executes the process.

### Shortest Remaining Time Next

The code first checks if the ready queue is empty using `IsEmpty(ReadyQueue)`. If the queue is empty, the function simply returns and does nothing.

If the ready queue is not empty, the code checks if there is a currently running process (`RunningProcess` is not `NULL`) and the process at the head of the ready queue has a shorter remaining execution time than the currently running process by comparing the ids of `RunningProcess` and the head of `ReadyQueue`. If this is the case, the function calls `preempt()` to stop the currently running process and switch to the new process with a shorter remaining execution time.

Finally, the function calls `runProcess()` to execute the process at the head of the ready queue.

### Round Robin

The code first checks if the ready queue is empty by checking if `ReadyQueue->head` is `NULL`. If the queue is empty, the function simply returns and does nothing.

If the queue is not empty, the code first retrieves the process at the head of the ready queue using `&(ReadyQueue->head->process)` and assigns it to a pointer variable `p`.

Then, the code checks if there is no currently running process (`RunningProcess` is `NULL`) and there is a process at the head of the ready queue (`p` is not `NULL`). If this is the case, the function assigns the process at the head of the ready queue to `RunningProcess` and calls `runProcess()` to execute it.

If there is a currently running process, the code checks if the time quantum has elapsed for the process by comparing the current system time (`getClk()`) with the last start time of the process (`RunningProcess->laststart`) plus the time quantum (`q`). If the time quantum has elapsed, the function stops the currently running process using `ReadingProcess()` and checks if there is another process in the ready queue to be executed.

If there is another process in the ready queue, the function dequeues the process at the head of the queue using `dequeueProcess(ReadyQueue)` and saves it in a temporary variable `temp`. Then, the function inserts `temp` back into the ready queue using `InsertInRR(temp, ReadyQueue, q)` to maintain the order of processes in the queue.

Finally, the function assigns the process at the head of the ready queue to `RunningProcess` and calls `runProcess()` to execute it.

### Memory Allocation Algorithm

We have defined a function `Allocation` that attempts to allocate the process according to the current algorithm by calling the functions below:

#### First Fit Algorithm

For allocating the process:

```c
int allocate_process(List* list, processIn* process)
```

The function first initializes two pointers to nodes in the linked list: `curr_hole` points to the head of the list, and `prev_hole` points to `NULL`. The function then iterates
