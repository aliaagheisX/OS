#include "headers.h"
#include "datastructure.h"
#include "algorithms.h"

int main(int argc, char *argv[])
{

    CURR_ALGO = atoi(argv[1]);
    CURR_ALGO_mem=atoi(argv[2]);
    Quantum = atoi(argv[3]);
    int prev_time = -1;
    int curr_time = 0;
    initSCH();
    
    

    while (!isGeneratorFinished || !IsEmpty(ReadyQueue) || RunningProcess)
    {
        curr_time = getClk();

        // =========== if 1 clk cycle happen ============
        // 1 - Read if Process come from Generator
        // 2 - if there's Running Process => update it's state
        // 3 - Call implementation of Scheduler Algorithm (may be they use it)
        UpdateInfo(running, NULL);
            ReadingProcess();
        if (CURR_ALGO==RR && !IsEmpty(ReadyQueue))
                implementRR(ReadyQueue, Quantum); 
        else if (CURR_ALGO == HPF)
            implementHPF(ReadyQueue);
        else if (CURR_ALGO == SRTN && 
                (!RunningProcess || 
                    (ReadyQueue->head && ReadyQueue->head->proccess.id != RunningProcess->id)
                ))
            implementSRTN(ReadyQueue);
            
        // if(curr_time != prev_time) {
            
        //     prev_time = curr_time;
        // }

        
    }
    raise(SIGINT);
}


void HandlerPrF(int signum) {
    RunningProcess->finish_time = getClk();
    isProcessFinished = 1;
}

Destroy_Shared_Rem(){
    struct PNode*temp=ReadyQueue->head;
    while(temp){
        shmdt(temp->proccess.remain);
        shmctl(temp->proccess.shmid, IPC_RMID, (struct shmid_ds *)0);
        temp=temp->next;
    }
}
void clearResources(int signum)
{
    Destroy_Shared_Rem();
    finish_cpu_calc();
    fclose(log_file); // close scheduler.log
    fclose(mem_log_file);
    destroyQueue(ReadyQueue);
    
    destroyClk(true); //// NOTE: transfet it true
    

    raise(SIGKILL);
}
void initalize_memory() {
    if(CURR_ALGO_mem == BDD)
        initialize_buddy();
    else {
        memory = CreateList();
        memory->head = memory->tail = CreateListNode(1024, 0);
    }
}
void initSCH()
{
    initClk();

    signal(SIGUSR1, HandlerPrF);   //when proc killed or stopped
    signal(SIGINT, clearResources);
    
   
    log_file = fopen("scheduler.log", "w"); // open scheduler.perf in append mode
    mem_log_file = fopen("memory.log", "w"); // open scheduler.perf in append mode

    fprintf(log_file,"At time x process y state arr w total z remain y wait k\n");
    fprintf(mem_log_file,"#At time x allocated y bytes for process z from I to j\n");

    ReadyQueue = createQueue();
    waitingQueue_allocation=createQueue();
    //Phase 2
    initalize_memory();

    ///
    cpu_state.totalTimeSchedule = 0;
    cpu_state.avg_waiting = 0;
    cpu_state.avg_wta = 0;
    cpu_state.cpu_utilization = 0;
    cpu_state.std_waiting = 0;
    cpu_state.numCompleted = 0;
    cpu_state.totalRunningTime = 0;
    cpu_state.totalWaiting = 0;
    cpu_state.totalWaitingSquared = 0;
    cpu_state.totalWTA = 0;
    ///

    msgq_id = msgget(MSGQKEY, 0666 | IPC_CREAT); // message queue for proccess_generator
    if (msgq_id == -1)
    {
        perror("scheduler: Error in create queue");
        exit(-1);
    }
}

int getProcessFromGen()
{
    ssize_t id = msgrcv(msgq_id, &curr_procc, sizeof(processIn), 0, IPC_NOWAIT);
    if(id != -1 && curr_procc.id == -1) {
        isGeneratorFinished = 1;
        return -1;
    }
    isGeneratorFinished = 0;
    return id;
}


/// => printinfo =>>>
/// => error code 
/// => cpu util + print it ===>
//==============
// quueue