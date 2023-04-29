#include "headers.h"
#include "datastructure.h"

void clearResources(int);
void readProcesses();
void readAlgo();
void createClkChild();
void createSchChild();
void sendProccessToSchd();
ALGO_mem CURR_ALGO_mem;

struct PQueue *queue;
enum ALGO CURR_ALGO;
int Quantum = 1;
int msgq_id;
pid_t clk_pid, schedule_pid;

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    readProcesses();
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    readAlgo();
    // 3. Initiate and create the scheduler and clock processes.
    createSchChild();
    createClkChild();
    //  4. Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this

    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    msgq_id = msgget(MSGQKEY, 0666 | IPC_CREAT); // create message queue for scheduler
    if (msgq_id == -1)
    {
        perror("generator: Error in create queue");
        exit(-1);
    }
    // 6. Send the information to the scheduler at the appropriate time.
    processIn process_ending ;
    process_ending.id = -1;
    process_ending.arrival_time = 0;
    struct PNode * endingNode = CreateNode(process_ending);

    enqueuProcess(endingNode, queue);

    while (true)
        sendProccessToSchd();

    raise(SIGINT);
}

void clearResources(int signum)
{
    destroyClk(true);                                // kill clock + deattach shared mem + send Int to all
    int lock;
    wait(&lock);    //wait for clock
 
    wait(&lock); // wait for scheduler

    msgctl(msgq_id, IPC_RMID, (struct msqid_ds *)0); // clear msg queue of scheduler
    //DEBUG
    raise(SIGKILL);                                  // kill himself
}

/**/
void readProcesses()
{
    queue = createQueue();

    FILE *fptr = fopen("processes.txt", "r");
    fseek(fptr, 37, SEEK_SET);

    while (readAddProccess(fptr, queue))
        ;
}

void readAlgo()
{
    int temp = 0;
    while (1)
    {
        printf("\n=============\n1. Non-preemptive Highest Priority First (HPF). \n2. Shortest Remaining time Next (SRTN). \n3. Round Robin (RR).\n=============\nEnter 1,2,3 to choose from current Algorithms: ");
        scanf("%d", &temp);
        if (temp >= 1 && temp <= 3)
        {
            CURR_ALGO = temp;
            break;
        }
    }
    while (1)
    {
        printf("\n=============\n1. First Fit (FFT). \n2. Buddy List (BBD).\n=============\nEnter 1,2 to choose from Allocation Algorithms: ");
        scanf("%d", &temp);
        if (temp >= 1 && temp <= 2)
        {
            CURR_ALGO_mem = temp;
            break;
        }
    }
    if (CURR_ALGO == RR)
    {
        printf("Enter The Quantum: ");
        scanf("%d", &Quantum);
    }
}

void createClkChild()
{
    clk_pid = fork();
    if (clk_pid == 0)
        execl("clk.out", "", NULL);

    if (clk_pid < 0)
    {
        printf("error in fork() clk\n");
        exit(-1);
    }
}

void createSchChild()
{
    schedule_pid = fork();

    if (schedule_pid == 0)
    {
        char algo[5];
        char algomem[5];
        char q[5];
        sprintf(algo, "%i", (int)CURR_ALGO);
        sprintf(algomem, "%i", (int)CURR_ALGO_mem);
        sprintf(q, "%i", Quantum);
        execl("scheduler.out", "scheduler.out", algo,algomem, q, NULL);
    }
    if (schedule_pid < 0)
    {
        printf("error in fork() scheduler\n");
        exit(-1);
    }
}

void sendProccessToSchd()
{
    if (queue->head == NULL)
        return;

    int curr_time = getClk();

    while (queue->head != NULL && queue->head->proccess.arrival_time <= curr_time)
    {
        msgsnd(msgq_id, &(queue->head->proccess), sizeof(processIn), !IPC_NOWAIT);
        dequeuProcess(queue);
    }
}