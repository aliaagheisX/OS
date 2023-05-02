#include "datastructure.h"
#include "headers.h"
int isProcessFinished = 0;
pid_t pid;
processIn *RunningProcess = NULL;
enum ALGO CURR_ALGO = HPF;
ALGO_mem CURR_ALGO_mem = FFT;
CpuState cpu_state;
struct PQueue *ReadyQueue=NULL; //queue store process
FILE *log_file=NULL;
int Quantum=1;
struct PQueue *waitingQueue_allocation = NULL;


void initSCH();           // inialize scheduler
void clearResources(int); //(clear resource+kill itself) when Gen. till him
int getProcessFromGen();  // wait for Gen. to send process

int msgq_id;
processIn curr_procc; // process buffer
int isGeneratorFinished = 0;
FILE *mem_log_file = NULL;


// ==== Headers
void InsertInReadyQueue(processIn proccess) ;
void Handler(int signum); // handel when child die SIGCHLD
void implementSRTN(struct PQueue *ReadyQueue);
void DeAllocation();
int Allocation();

// ==== Headers

void ReadingProcess() {
    while (!isGeneratorFinished && getProcessFromGen() != -1) {
        struct PNode *newNode = CreateNode(curr_procc);
        printf("======= Insert %i at %i ====\n", curr_procc.id, getClk());
        if (CURR_ALGO == HPF)
                enqueuProcessByPriority(newNode, ReadyQueue);
        else if (CURR_ALGO == SRTN)
                enqueueByRemaining(newNode, ReadyQueue);
        else if(CURR_ALGO == RR)
                enqueuProcess(newNode, ReadyQueue);// RR
    }    
}

void update_cpu_calc(processIn *p) {
    cpu_state.numCompleted++;
    cpu_state.totalRunningTime += p->runtime;
    cpu_state.totalTimeSchedule = p->finish_time;
    cpu_state.totalWTA += p->WTA;
    cpu_state.totalWaiting += (double)p->waiting;
    cpu_state.totalWaitingSquared += (double)p->waiting * (double)p->waiting;

}
void finish_cpu_calc() {
    printf("======= totalTimeSchedule %i ==========\n", cpu_state.totalTimeSchedule);
    cpu_state.cpu_utilization = cpu_state.totalTimeSchedule ? (double)cpu_state.totalRunningTime / (double)(cpu_state.totalTimeSchedule - 1) : 0;
    cpu_state.avg_wta = cpu_state.numCompleted ? cpu_state.totalWTA / cpu_state.numCompleted : INFINITY;
    cpu_state.avg_waiting = cpu_state.numCompleted ? cpu_state.totalWaiting / cpu_state.numCompleted : INFINITY;
    cpu_state.std_waiting = cpu_state.numCompleted ? sqrt( (cpu_state.totalWaitingSquared / cpu_state.numCompleted) - pow(cpu_state.avg_waiting, 2.0)) : INFINITY;

    FILE *perf_file = fopen("scheduler.perf", "w"); // open scheduler.perf in append mode
    if (perf_file == NULL) {
        printf("Error opening scheduler.perf file\n");
        return;
    }

    // print CPU data to scheduler.perf
    //fprintf(perf_file, "Runtime: %i\n", cpu_state.totalRunningTime);
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
        fprintf(log_file, "At time %d process %d started arr %d total %d remain %d wait %d\n", p->starttime, p->id, p->arrival_time, p->runtime, *(p->remain), p->waiting);
        fprintf(mem_log_file, "At time %d allocated %d bytes for process %d from %d to %d\n", p->starttime, p->memsize, p->id, p->start_address, p->start_address + p->memsize - 1);
        break;
    case 1:
        fprintf(log_file, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", p->lastStop, p->id, p->arrival_time, p->runtime, *(p->remain), p->waiting);
        break;
    case 2:
        fprintf(log_file, "At time %d process %d resumed arr %d total %d remain %d wait %d \n", p->laststart, p->id, p->arrival_time, p->runtime, *(p->remain), p->waiting);
        /* code */
        break;
    case 3:
        fprintf(log_file, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", 
        p->finish_time, 
        p->id, 
        p->arrival_time, 
        p->runtime,
        *(p->remain), p->waiting, p->TA, p->WTA);
        fprintf(mem_log_file, "At time %d freed %d bytes for process %d from %d to %d\n", p->finish_time, p->memsize, p->id, p->start_address, p->start_address + p->memsize - 1);
        break;
    case 4:
       // fprintf(log_file, "At time %d\tprocess %d\trunning     arr %d\ttotal %d\tremain %d\twait %d  \n", getClk(), p->id, p->arrival_time, p->runtime, p->remaining, p->waiting);

        break;
    }
}

void UpdateInfo(state newState,Payload* load)
{   
    if(!RunningProcess) return;
    if(newState == started) {
        RunningProcess->starttime = getClk();
        RunningProcess->systemID = load->pid;
        RunningProcess->firsttime = 1;
        RunningProcess->waiting = RunningProcess->starttime - RunningProcess->arrival_time;
        
        RunningProcess->curr_state = started;
        printinfo(RunningProcess);
        RunningProcess->curr_state = running;


    }
    else if(newState == finished /*|| *(RunningProcess->remain) == 0*/  || isProcessFinished) {
        printf("The Process Finished %i\n", getClk());
        int state;
        wait(&state);
        isProcessFinished = 0;

        RunningProcess->curr_state = finished;
        //RunningProcess->finish_time = getClk();

        ////////// CALCulation //////////
        RunningProcess->TA = RunningProcess->finish_time - RunningProcess->arrival_time;
        RunningProcess->WTA = RunningProcess->runtime ? RunningProcess->TA*1.0 / RunningProcess->runtime : INFINITY;
        update_cpu_calc(RunningProcess);
        
        RunningProcess->remaining=*(RunningProcess->remain);
        printinfo(RunningProcess); //show results
        
        
        
        destroyRemain(RunningProcess->remain);
      //==================Phase 2====================//
        DeAllocation();
        DequeuProcessFromQueue(ReadyQueue,RunningProcess);
        shmctl(RunningProcess->shmid, IPC_RMID, (struct shmid_ds *)0);

        RunningProcess = NULL;
        return;
    } 
   
    else if(newState == stoped) {
        RunningProcess->curr_state = stoped;
        RunningProcess->lastStop=getClk();
        RunningProcess->firsttime = 1;
        printinfo(RunningProcess);

        RunningProcess = NULL;

    }
    else if(newState == resumed) {

        RunningProcess->waiting =  RunningProcess->laststart - RunningProcess->starttime - RunningProcess->runtime + *(RunningProcess->remain);

        RunningProcess->curr_state = resumed;
        RunningProcess->laststart = getClk();
        printinfo(RunningProcess);
        RunningProcess->curr_state = running;
    }
    
   

    // else
    
}

// |||   ||   ||   ||   ||   ||   |






// ========================== Utilites ===============

int min(int a, int b)
{
    return (a > b) ? b : a;
}


void preempt()
{
    kill(RunningProcess->systemID, SIGTSTP);
    UpdateInfo(stoped, NULL);
}

void runProcess(processIn *pRun)
{
    RunningProcess = pRun;
    RunningProcess->laststart = getClk();
    if (pRun->firsttime == 0)
    {
        
////////////====================================Phase 2======================//
         if (Allocation() == -1) {
            RunningProcess = NULL;
            return;
        }
        int shmid = shmget(IPC_PRIVATE, 4096, IPC_CREAT | 0644);
        if (shmid == -1)
        {
            perror("Error in create");
            exit(-1);
        }
        RunningProcess->shmid = shmid;

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
            execl("process.out", "process.out", proc_id,proc_runtime,proc_shmid, NULL);
            return;
        }
        else {
            Payload load;
            load.pid = pid;
            pRun->remain =attachRemain(shmid);
            *(pRun->remain) = pRun->runtime;
            UpdateInfo(started, &load);
        }
    }
    else
    {
        kill(pRun->systemID, SIGCONT);
        UpdateInfo(resumed, NULL);
    }
    //printinfo(RunningProcess);
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
    //printf("==== Insert %i at %i==== \n", proccess.id, getClk());

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


void InsertInReadyQueue(processIn proccess) {
    struct PNode *newNode = CreateNode(proccess);
    //printf("==== Insert %i at %i==== \n", proccess.id, getClk());
    if (CURR_ALGO == HPF)
            enqueuProcessByPriority(newNode, ReadyQueue);
    else if (CURR_ALGO == SRTN)
            enqueueByRemaining(newNode, ReadyQueue);
    else if(CURR_ALGO == RR)
            enqueuProcess(newNode, ReadyQueue);// RR
}


void implementRR(struct PQueue *ReadyQueue, int q)
{
    if(!ReadyQueue->head) 
        return;

    processIn *p = &(ReadyQueue->head->proccess);

    if (!RunningProcess && p) {
        RunningProcess = p;
        runProcess(p);
    }
    else if (getClk() >= RunningProcess->laststart + q)
    {   
        processIn temp = ReadyQueue->head->proccess;
        if (*(temp.remain) > 0 && ReadyQueue->head->next) { // not finished yet & there's another process
            preempt();
            dequeuProcess(ReadyQueue);
            ReadingProcess();
             
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
    // 1 . deallocation
    if (CURR_ALGO_mem == FFT)
        deallocate_process(memory, RunningProcess);
    else
        deallocation_process_buddy(RunningProcess);

    int available_sz = RunningProcess->memsize;// the available on memory 
    // == 2. loop on waiting processes
    struct PNode *curr = waitingQueue_allocation->head;
    while (curr)
    {
        if(curr->proccess.memsize <= available_sz) {
            InsertInReadyQueue(curr->proccess);
            available_sz-= curr->proccess.memsize;
            DequeuProcessFromQueue(waitingQueue_allocation, curr);
        }
        curr = curr->next;
       
    }
    
}



