#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

int main(int agrc, char * argv[])
{
    initClk();
    int id=atoi(argv[1]);
    int runtime=atoi(argv[2]);
     //TODO it needs to get the remaining time from somewhere
    //initRemaining(id);
    int * Rem=attachRemain(atoi(argv[3]),id);
    printf("Here is our process %d    \n",id);
    *Rem=runtime;
    //remainingtime = atoi(argv[1]);
    
    //printf("Hello I'm Process with RT = %i\n", remainingtime);
    
    int prev=getClk();
    while ((*Rem) > 0)
    {
        printf("The Rem of Proc %d is %d at %d\n",id,*Rem,prev);
        while(prev==getClk());
        (*Rem)--;
        prev=getClk();
    }
    //printf("Finished Process\n");

      shmctl(atoi(argv[3]), IPC_RMID, (struct shmid_ds *)0);
    destroyClk(false);
    destroyRemain(Rem);

    raise(SIGKILL);
    
}
