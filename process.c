#include "headers.h"

/* Modify this file as needed*/
int remainingtime;
int id;
bool isStoppedBefore = 0;

void handel_stopped (int signum) {
    printf("Enter Stoppped %i\n", id);
    isStoppedBefore = 1;
}

int main(int agrc, char * argv[])
{
    initClk();
    signal(SIGTSTP, handel_stopped);
    int id=atoi(argv[1]);
    int runtime=atoi(argv[2]);
     //TODO it needs to get the remaining time from somewhere
    //initRemaining(id);
    int * Rem=attachRemain(atoi(argv[3]),id);
    *Rem=runtime;
    //remainingtime = atoi(argv[1]);
    
    //printf("Hello I'm Process with RT = %i\n", remainingtime);
    
    int prev=getClk();
    while ((*Rem) > 0)
    {

        printf("The Rem of Proc %d is %d at %d\n",id,*Rem,getClk());
        while(prev==getClk());
        if(isStoppedBefore == 0)
          (*Rem)--;
        isStoppedBefore = 0;
        prev=getClk();
    }
    //printf("Finished Process\n");

      shmctl(atoi(argv[3]), IPC_RMID, (struct shmid_ds *)0);
    destroyClk(false);
    destroyRemain(Rem);

    raise(SIGKILL);
    
}
