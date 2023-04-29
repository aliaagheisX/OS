#ifndef DATASTCTH
#define DATASTCTH
#include "headers.h"

//===============================
enum ALGO
{
    HPF = 1,
    SRTN,
    RR
};
typedef enum State{
    started,
    stoped,
    resumed,
    finished,
    running
}state;

typedef struct Payload { //payload of state
    int pid;
} Payload;
//Phase 2
typedef enum ALGO_mem {
    FFT = 1,
    BDD
} ALGO_mem;

typedef struct CpuState {
    double cpu_utilization;
    double avg_wta;
    double avg_waiting;
    double std_waiting;
    /// @brief 
    int numCompleted;
    int totalRunningTime;
    double totalWTA ;
    double totalWaiting ;
    double totalWaitingSquared ;

} CpuState;

//===============================
// store input processes data structure
typedef struct ProcessIn
{
    int id, arrival_time, runtime, priority;
    int starttime,remaining,cumlativeRunning,systemID;
    int firsttime, laststart,lastStop;
    int * remain;
    //Added By Sara
    state curr_state;

    int finish_time,TA,waiting;
    double WTA;

    int memsize;
    int start_address;
    int shmid;
} processIn;
///////////////////////////////////////////////////////
///==============================
struct PNode
{
    processIn proccess;
    struct PNode *next;
};

struct PQueue
{
    struct PNode *head, *tail;
};

struct PQueue *createQueue()
{
    struct PQueue *queue = (struct PQueue *)malloc(sizeof(struct PQueue));
    queue->head = queue->tail = NULL;
    return queue;
}

void enqueuProcess(struct PNode *node, struct PQueue *queue)
{
    if (!queue->head)
        queue->head = queue->tail = node;
    else
        queue->tail->next = node,
        queue->tail = node;
        node->next=NULL;
}
void DequeuProcessFromQueue(struct PQueue *queue,processIn*ToDelete)
{
   struct PNode*prev=queue->head;
   if(prev->proccess.id==ToDelete->id)
   {
        queue->head=prev->next;
        free(prev);
        return;
   }
   struct PNode*temp=queue->head->next;
   while(temp!=NULL)
   {
    if(temp->proccess.id==ToDelete->id)
    {
        prev->next=temp->next;
        free(temp);
        
        return;
    
    }
    prev=prev->next;
    temp=temp->next;
   }
}
void dequeuProcess(struct PQueue *queue)
{
    if (!queue->head)
        return;

    // printf("dequeue: %d\n", queue->head->proccess.arrival_time);
    //1
    struct PNode *temp = queue->head;
    queue->head = queue->head->next;
    if (!queue->head)
        queue->tail = NULL;

    free(temp);
}

void destroyQueue(struct PQueue *queue)
{
    while (queue->head != NULL)
    {
        dequeuProcess(queue);
    }
    free(queue);
}

bool readAddProccess(FILE *fptr, struct PQueue *queue)
{
    // 
    // 
    // state curr_state;
    // int finish_time,TA,waiting;
    // double WTA;
    struct PNode *newNode = (struct PNode *)malloc(sizeof(struct PNode));

    if (fscanf(fptr, "%d%d%d%d%d",
               &newNode->proccess.id,
               &newNode->proccess.arrival_time,
               &newNode->proccess.runtime,
               &newNode->proccess.priority,
               &newNode->proccess.memsize) == EOF)
        return false;

    newNode->proccess.remaining = newNode->proccess.runtime;
    newNode->proccess.cumlativeRunning = 0;
    newNode->proccess.firsttime = 0;
    newNode->next = NULL;
    enqueuProcess(newNode, queue);

    return true;
}
void printQueue(struct PQueue *q)
{
    struct PNode *tmp = q->head;
    while (tmp != NULL)
    {
        printf("%d %d %d %d \n", tmp->proccess.id, tmp->proccess.arrival_time, tmp->proccess.runtime, tmp->proccess.priority);
        tmp = tmp->next;
    }
}

void enqueueByRemaining(struct PNode *node, struct PQueue *queue)
{
    if (!queue->head) {
        queue->head = queue->tail = node;
    } else {
        // Traverse the queue to find the position to insert the new node
        struct PNode *current = queue->head;
        struct PNode *previous = NULL;
        while (current && node->proccess.remaining >= current->proccess.remaining) {
            previous = current;
            current = current->next;
        }
        // Insert the new node
        if (!previous) {
            node->next = queue->head;
            queue->head = node;
        } else {
            previous->next = node;
            node->next = current;
        }
        // Update the tail if necessary
        if (!current) {
            queue->tail = node;
        }
    }
}
void enqueuProcessByPriority(struct PNode *node, struct PQueue *queue)
{
    if (!queue->head) {
        queue->head = queue->tail = node;
    } else {
        // Traverse the queue to find the position to insert the new node
        struct PNode *current = queue->head;
        struct PNode *previous = NULL;
        while (current != NULL && node->proccess.priority >= current->proccess.priority) {
            previous = current;
            current = current->next;
        }
        // Insert the new node
        if (!previous) {
            node->next = queue->head;
            queue->head = node;
        } else {
            previous->next = node;
            node->next = current;
        }
        // Update the tail if necessary
        if (!current) {
            queue->tail = node;
        }
    }
}


void dequeueByPriority(struct PQueue *queue)
{
    dequeuProcess(queue);
}
bool IsEmpty(struct PQueue *queue)
{
    return (!queue->head);
}
struct PNode *CreateNode(processIn proc)
{
    struct PNode *newNode = (struct PNode *)malloc(sizeof(struct PNode));
    newNode->proccess = proc;
    //printf("Create process %i\n", newNode->proccess.id);
    newNode->next = NULL;
    return newNode;
}
struct PNode *peek(struct PQueue *queue)
{
    return queue->head;
}
//////////////////////////////////////////////////////Added for phase 2 :) /////////////////////////////////////////////
typedef struct ListNode
{
    int memsize;
    int start_address;
    struct ListNode* next;

} ListNode;

typedef struct List
{
    ListNode *head, *tail;
} List;


List *memory;


List* CreateList() {
    List *list = (struct List *)malloc(sizeof(List));
    list->head = list->tail = NULL;
    return list;
}
ListNode* CreateListNode(int memsize,int start_address) {
    ListNode* listNode  = (ListNode*) malloc(sizeof(ListNode));
    listNode->memsize = memsize;
    listNode->start_address = start_address;
    listNode->next = NULL;
}

void destroyList(List *list)
{
    // while (list->head != NULL)
    // {
    //     dequeuListNode(list);
    // }
    free(list);
}

void deallocate_hole(List*list, ListNode * prev,ListNode * curr) {
    if(!curr) return;

    if(!prev) 
        list->head = curr->next;
    else 
        prev->next = curr->next;
    free(curr);
}
int allocate_process(List* list, processIn *process) { //deallocate hole
    ListNode* curr_hole = list->head; //loop on holes
    ListNode* prev_hole = NULL; //loop on holes

    while(curr_hole && curr_hole->memsize < process->memsize) {
        prev_hole = curr_hole;
        curr_hole = curr_hole->next;
    }
    if(!curr_hole) {
        return -1;
    }

    //1.update process
    process->start_address = curr_hole->start_address;

    //2.update list of holes depend on size left from node
    int new_size =  curr_hole->memsize - process->memsize;
    if(new_size == 0) {
        deallocate_hole(list, prev_hole, curr_hole);
    }   
    else {
        curr_hole->memsize -= process->memsize;
        curr_hole->start_address = curr_hole->start_address + process->memsize;
    }

    return 1;
        
}
void mergeNext(List* list, ListNode* node) {
    if(!node || !node->next 
    || node->start_address + node->memsize != node->next->start_address) return;
    node->memsize += node->next->memsize;
    
    deallocate_hole(list, node, node->next);
}
void deallocate_process(List *list, processIn *process) { //allocate hole
    //create new hole with same parameters of process
    ListNode *node = CreateListNode(process->memsize, process->start_address);
    if (!list->head) {
        list->head = list->tail = node;
        return;
    } 
    // Traverse the queue to find the position to insert the new node
    ListNode *current = list->head;
    ListNode*previous = NULL;
    while (current && node->start_address >= current->start_address) {
        previous = current;
        current = current->next;
    }

    // Insert the new node
    if (!previous) {
        node->next = list->head;
        list->head = node;
        mergeNext(list, node);
    } else {
        previous->next = node;
        node->next = current;

        mergeNext(list, node);
        mergeNext(list, previous);
    }
    // Update the tail if necessary
    if (!current) {
        list->tail = node;
    }
}
void printList(List *l)
{
    ListNode *tmp = l->head;
    while (tmp != NULL)
    {
        printf("%d %d \n", tmp->start_address, tmp->memsize);
        tmp = tmp->next;
    }
    printf("===================\n");
}

#endif