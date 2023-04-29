#ifndef MYHEADERS
#define MYHEADERS

#include <stdio.h> //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "math.h"
#include "buddy.h"


#define SHKEY 300
#define SHKEYREMAIN 500
#define MSGQKEY 400

///==============================
// don't mess with this variable//
static int *shmaddr; //
static int *shmaddrR;
static int shmR[100];
//===============================

int getClk()
{
    return *shmaddr;
}
int * attachRemain(int shmid,int id)
{
    shmaddrR=(int *)shmat(shmid,(void *)0,0);
    if(shmaddrR == -1)
    {
        printf("Error in Getting Remain\n");
        //shmaddrR[id]=(int *)shmat(shmR[id],(void *)0,0);
    }
    return shmaddrR;
}

/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
 */
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        // Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *)shmat(shmid, (void *)0, 0);
}
void initRemaining(int id)
{
    int key=id*SHKEYREMAIN;
    shmR[id]=shmget(IPC_PRIVATE,10,IPC_CREAT|0444);
    if ((int)shmR[id] == -1)
    {
        printf("Error in getting Remain of process %d\n",id);
        exit(-1);
    }
    printf("Success in Create Remain\n");
}

/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
 */

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}
void destroyRemain(int *shmaddrR)
{
    shmdt(shmaddrR);
   
}
#endif