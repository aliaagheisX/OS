#include "headers.h"

/* Modify this file as needed*/
int id;


int main(int agrc, char * argv[])
{
    initClk();
    int id=atoi(argv[1]);
    int runtime=atoi(argv[2]);
    
    int * Rem=attachRemain(atoi(argv[3]));
    *Rem=runtime;
    
    
    int prev=getClk();
    while ((*Rem) > 0)
    {
        printf("The Rem of Proc %d is %d at %d\n",id,*Rem,getClk());
        (*Rem)-- ;
        for(int i = 0;i < 50;i++) {
          usleep(20000);//0.02
        }

    }
    //printf("Finished Process\n");

    kill(getppid(), SIGUSR1);
    destroyRemain(Rem);


    destroyClk(false);

    exit(0);
    
}
