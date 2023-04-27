#include "datastructure.h"
#include "headers.h"

pid_t pid;
CpuState cpu_state;
processIn *RunningProcess = NULL;
enum ALGO CURR_ALGO = HPF;
ALGO_mem CURR_ALGO_mem = FFT;

struct PQueue *ReadyQueue = NULL;              // queue store process
struct PQueue *waitingQueue_allocation = NULL; // queue store process waiting for allocation
processIn curr_procc;                          // process buffer for msg queue

FILE *log_file = NULL;
FILE *mem_log_file = NULL;
int Quantum = 1;

void implementRR(struct PQueue *ReadyQueue, int q);

void initSCH();           // inialize scheduler
void clearResources(int); //(clear resource+kill itself) when Gen. till him
int getProcessFromGen();  // wait for Gen. to send process
void DeAllocation();
int Allocation();

int msgq_id;
int isGeneratorFinished = 0;

// ==== Headers
void InsertInReadyQueue(processIn proccess);
void Handler(int signum); // handel when child die SIGCHLD
void implementSRTN(struct PQueue *ReadyQueue);

// ==== Headers

void ReadingProcess()
{
    while (!isGeneratorFinished && getProcessFromGen() != -1)
        InsertInReadyQueue(curr_procc);
}
void update_cpu_calc(processIn *p)
{
    cpu_state.numCompleted++;
    cpu_state.totalRunningTime += p->runtime;
    cpu_state.totalWTA += p->WTA;
    cpu_state.totalWaiting += (double)p->waiting;
    cpu_state.totalWaitingSquared += (double)p->waiting * (double)p->waiting;
}
void finish_cpu_calc()
{
    cpu_state.cpu_utilization = (double)cpu_state.totalRunningTime / (double)(getClk() - 1);
    cpu_state.avg_wta = cpu_state.totalWTA / cpu_state.numCompleted;
    cpu_state.avg_waiting = cpu_state.totalWaiting / cpu_state.numCompleted;
    cpu_state.std_waiting = sqrt((cpu_state.totalWaitingSquared / cpu_state.numCompleted) - pow(cpu_state.avg_waiting, 2.0));

    FILE *perf_file = fopen("scheduler.perf", "w"); // open scheduler.perf in append mode
    if (perf_file == NULL)
    {
        printf("Error opening scheduler.perf file\n");
        return;
    }

    // print CPU data to scheduler.perf
    // fprintf(perf_file, "Runtime: %i\n", cpu_state.totalRunningTime);
    fprintf(perf_file, "CPU utilization: %.2f%%\n", cpu_state.cpu_utilization * 100.0);
    fprintf(perf_file, "Average weighted turnaround time: %.2f\n", cpu_state.avg_wta);
    fprintf(perf_file, "Average waiting time: %.2f\n", cpu_state.avg_waiting);
    fprintf(perf_file, "Standard deviation of waiting time: %.2f\n", cpu_state.std_waiting);

    fclose(perf_file); // close the file
}
void printinfo(processIn *p)
{
    switch (p->curr_state)
    {

    case 0:
        /* code */
        fprintf(log_file, "At time %d\tprocess %d\tstarted    \tarr %d\ttotal %d\tremain %d\twait %d\n", p->starttime, p->id, p->arrival_time, p->runtime, p->remaining, p->waiting);
        fprintf(mem_log_file, "At time %d allocated %d bytes for process %d from %d to %d\n", p->starttime, p->memsize, p->id, p->start_address, p->start_address + p->memsize - 1);
        break;
    case 1:
        fprintf(log_file, "At time %d\tprocess %d\tstopped     arr %d\ttotal %d\tremain %d\twait %d \n", getClk(), p->id, p->arrival_time, p->runtime, p->remaining, p->waiting);
        break;
    case 2:
        fprintf(log_file, "At time %d\tprocess %d\tresumed     arr %d\ttotal %d\tremain %d\twait %d  \n", getClk(), p->id, p->arrival_time, p->runtime, p->remaining, p->waiting);
        /* code */
        break;
    case 3:
        fprintf(log_file, "At time %d\tprocess %d\tfinished\tarr %d\ttotal %d\tremain %d\twait %d\tTA %d\tWTA %.2f \n", getClk(), p->id, p->arrival_time, p->runtime, p->remaining, p->waiting, p->TA, p->WTA);
        fprintf(mem_log_file, "At time %d freed %d bytes for process %d from %d to %d\n", getClk(), p->memsize, p->id, p->start_address, p->start_address + p->memsize - 1);
        break;
    case 4:
        fprintf(log_file, "At time %d\tprocess %d\trunning     arr %d\ttotal %d\tremain %d\twait %d  \n", getClk(), p->id, p->arrival_time, p->runtime, p->remaining, p->waiting);

        break;
    }
}
void UpdateInfo(state newState, Payload *load)
{
    if (!RunningProcess)
        return;
    if (newState == started)
    {
        // printf("The Processes %i started\n", RunningProcess->id);
        RunningProcess->starttime = getClk();
        RunningProcess->systemID = load->pid;
        RunningProcess->firsttime = 1;
        RunningProcess->waiting = RunningProcess->starttime - RunningProcess->arrival_time;

        RunningProcess->curr_state = started;
        printinfo(RunningProcess);
        RunningProcess->curr_state = running;
    }
    else if (newState == finished)
    {
        int stat_loc;
        // printf("The Processes %i finished\n", RunningProcess->id);

        waitpid(RunningProcess->systemID, &stat_loc, 0);

        RunningProcess->curr_state = finished;
        RunningProcess->finish_time = getClk();

        ////////// CALCulation //////////
        RunningProcess->TA = RunningProcess->finish_time - RunningProcess->arrival_time;
        RunningProcess->WTA = RunningProcess->TA * 1.0 / RunningProcess->runtime;
        update_cpu_calc(RunningProcess);
        RunningProcess->remaining = *(RunningProcess->remain);
        ////////// CALCulation //////////
        //============ phase 2 =========
        DeAllocation();

        printinfo(RunningProcess); // show results
        if (CURR_ALGO == HPF || CURR_ALGO == SRTN)
            DeleteProcessinHPF(ReadyQueue, RunningProcess);
        else
            dequeuProcess(ReadyQueue);
        RunningProcess = NULL;

        return;
    }
    else if (newState == stoped)
    {
        // printf("The Processes %i stopped\n", RunningProcess->id);

        RunningProcess->curr_state = stoped;
        RunningProcess->lastStop = getClk();
        RunningProcess->firsttime = 1;
        printinfo(RunningProcess);

        RunningProcess = NULL;

        return;
    }
    else if (newState == resumed)
    {
        // printf("The Processes %i resumed\n", RunningProcess->id);

        RunningProcess->curr_state = resumed;
        RunningProcess->laststart = getClk();
        RunningProcess->waiting += RunningProcess->laststart - RunningProcess->lastStop;
        printinfo(RunningProcess);
        RunningProcess->curr_state = running;
    }

    else if (RunningProcess->curr_state == running)
    {
        // printf("The Processes %i running\n", RunningProcess->id);

        RunningProcess->curr_state = running;
        RunningProcess->cumlativeRunning += 1;
        RunningProcess->remaining = *(RunningProcess->remain);

        printinfo(RunningProcess);
    }
    // else
}

// |||   ||   ||   ||   ||   ||   |

/* void UpdateInfoNormal() {
    UpdateInfo(running, NULL);
    alarm(1);
}  */
void UpdateInfoNormal()
{
    ReadingProcess();
    // printf("Entering Alarm %p at clk %i\n", RunningProcess, getClk());
    if (RunningProcess)
        UpdateInfo(running, NULL);
    if (CURR_ALGO == RR)
        implementRR(ReadyQueue, Quantum); // RR

    alarm(1);
}

// ========================== Utilites ===============

int min(int a, int b)
{
    return (a > b) ? b : a;
}

void stopProcess(int sysid)
{
    printf("Here is Stop %d\n", sysid);
    kill(sysid, SIGTSTP);
}

void preempt()
{
    stopProcess(RunningProcess->systemID);
    UpdateInfo(stoped, NULL);
    return;
}

void runProcess(processIn *pRun)
{
    RunningProcess = pRun;
    RunningProcess->laststart = getClk();
    if (pRun->firsttime == 0)
    {
        //========== Phase 2
        if (Allocation() == -1) {
            RunningProcess = NULL;
            return;
        }

        int shmid;
        shmid = shmget(IPC_PRIVATE, 4096, IPC_CREAT | 0644);

        if (shmid == -1)
        {
            perror("Error in create");
            exit(-1);
        }
        else
            printf("\nShared memory ID = %d\n", shmid);

        //
        int pid = fork();
        if (pid == -1)
        {
            printf("Error in Creating Process\n");
        }
        else if (pid == 0)
        {
            char proc_id[5];
            sprintf(proc_id, "%i", pRun->id);
            char proc_runtime[5];
            sprintf(proc_runtime, "%i", pRun->runtime);
            char proc_shmid[5];
            sprintf(proc_shmid, "%i", shmid);
            execl("process.out", "process.out", proc_id, proc_runtime, proc_shmid, NULL);
            return;
        }
        else
        {
            Payload load;
            load.pid = pid;
            pRun->remain = attachRemain(shmid);
            UpdateInfo(started, &load);
        }
    }
    else
    {
        kill(pRun->systemID, SIGCONT);
        UpdateInfo(resumed, NULL);
    }
    // printinfo(RunningProcess);
}

// ========================== Main ALGO ===============
void InsertInHPF(processIn proccess, struct PQueue *ReadyQueue)
{
    proccess.remaining = proccess.runtime;
    struct PNode *newNode = CreateNode(proccess);
    enqueuProcessByPriority(newNode, ReadyQueue);
}

void InsertInSRTN(processIn proccess, struct PQueue *ReadyQueue)
{
    proccess.remaining = proccess.runtime;
    struct PNode *newNode = CreateNode(proccess);

    enqueueByRemaining(newNode, ReadyQueue);
}

void InsertInRR(processIn proccess, struct PQueue *ReadyQueue, int Q)
{
    struct PNode *newNode = CreateNode(proccess);
    enqueuProcess(newNode, ReadyQueue);
}

// ========================== Main ALGO ===============
void implementHPF(struct PQueue *ReadyQueue)
{
    if (RunningProcess == NULL && !IsEmpty(ReadyQueue))
    {
        RunningProcess = &(peek(ReadyQueue)->proccess);
        RunningProcess->curr_state = started;
        runProcess(RunningProcess);
    }
}

void implementSRTN(struct PQueue *ReadyQueue)
{
    if (IsEmpty(ReadyQueue))
        return;
    if (RunningProcess && ReadyQueue->head->proccess.id != RunningProcess->id)
    {
        preempt();
    }
    runProcess(&(ReadyQueue->head->proccess));
}

void InsertInReadyQueue(processIn proccess)
{
    struct PNode *newNode = CreateNode(proccess);
    if (CURR_ALGO == HPF)
        enqueuProcessByPriority(newNode, ReadyQueue);
    else if (CURR_ALGO == SRTN)
        enqueueByRemaining(newNode, ReadyQueue);
    else if (CURR_ALGO == RR)
        enqueuProcess(newNode, ReadyQueue); // RR
}

void implementRR(struct PQueue *ReadyQueue, int q)
{
    if (!ReadyQueue->head)
        return;

    processIn *p = &(ReadyQueue->head->proccess);
    if (!RunningProcess && p)
    {
        RunningProcess = p;
        runProcess(p);
    }
    else if (getClk() == RunningProcess->laststart + q)
    {
        processIn temp = ReadyQueue->head->proccess;
        if (temp.remaining > 0)
        { // not finished yet
            preempt();
            dequeuProcess(ReadyQueue);
            InsertInRR(temp, ReadyQueue, q);
            RunningProcess = &ReadyQueue->head->proccess;
            runProcess(RunningProcess);
        }
    }
}

int Allocation()
{
    if ((CURR_ALGO_mem == FFT && allocate_process(memory, RunningProcess) == -1) || (CURR_ALGO_mem == BDD && allocation_process_buddy(RunningProcess) == -1))
    {

        // insert in waitingQueue
        struct PNode *temp = CreateNode(*RunningProcess);
        enqueuProcess(temp, waitingQueue_allocation);

        dequeuProcess(ReadyQueue);

        RunningProcess = NULL;
        return -1;
    }
    return 1;
}

void DeAllocation()
{
    if (waitingQueue_allocation->head && waitingQueue_allocation->head->proccess.memsize <= RunningProcess->memsize)
    {
        InsertInReadyQueue(waitingQueue_allocation->head->proccess);
        dequeuProcess(waitingQueue_allocation);
    }
    if (CURR_ALGO_mem == FFT)
        deallocate_process(memory, RunningProcess);
    else
        deallocation_process_buddy(RunningProcess);
}