#ifndef DATABDDY
#define DATABDDY

#include "headers.h"
#include "datastructure.h"

#define SZ 11

List* buddy_list[SZ];

// ============== Defentations ==========
int allocation_process_buddy(processIn *process);
void allocate_hole_from_parent(int index, int target) ;
void allocate_hole_buddy(List* list, int memsize, int start_address);
// ============== Defentations ==========
//

int findlog(processIn* p)
{
    return ceil(log2(p->memsize));
}

void initialize_buddy() {
    for(int i = 0;i < SZ;i++) {
        buddy_list[i] = (List*)malloc(sizeof(List));
    printf( " buddy list pointer %p\n",buddy_list[i]);
    }
    buddy_list[10]->head = buddy_list[10]->tail = CreateListNode(1024, 0);
}

int allocation_process_buddy(processIn *process) {
    int index = findlog(process);
    printf("\n index of process %d and index is %d \n",process->id,index);

    //if list of holes not empty   
    if(buddy_list[index]->head)  {
        process->start_address = buddy_list[index]->head->start_address; //get the address
        process->memsize=1<<index;
        ListNode* curr_hole = buddy_list[index]->head;
        //dequeu head of holes
        buddy_list[index]->head = curr_hole->next;         
        free(curr_hole);

        return 1;
    }

    //if there no hole in this list
    //check first list with holes ==> that larger that mine 
    int larger_index = index;
    while(larger_index < SZ && !buddy_list[larger_index]->head) {
        larger_index++;
    }

    if(larger_index == SZ)
        return -1;

    allocate_hole_from_parent(larger_index, index);

    if(!buddy_list[index]->head) //return the check
        return -1;
     process->memsize=1<<index;

    process->start_address = buddy_list[index]->head->start_address; //get the address
    ListNode* curr_hole = buddy_list[index]->head;
    //dequeu head of holes
    buddy_list[index]->head = curr_hole->next;         
    free(curr_hole);

    return 1;
}

//              
//                    512
//      256     256
//    128  128
//
void allocate_hole_from_parent(int index, int target) {
    if(index == target || !buddy_list[index]->head)
        return;
    
    ListNode* parent = buddy_list[index]->head;
    
    allocate_hole_buddy(buddy_list[index-1], parent->memsize / 2, parent->start_address);
    allocate_hole_buddy(buddy_list[index-1], parent->memsize / 2, parent->start_address + parent->memsize / 2);

    buddy_list[index]->head = parent->next; //remove parent node
    free(parent);

    allocate_hole_from_parent(index-1, target);
}

void allocate_hole_buddy(List* list, int memsize, int start_address){
    ListNode *node = CreateListNode(memsize, start_address);
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
    } else {
        previous->next = node;
        node->next = current;
    }
    // Update the tail if necessary
    if (!current) {
        list->tail = node;
    }
}


void deallocation_process_buddy(processIn *process) {
    int index = findlog(process);

    allocate_hole_buddy(buddy_list[index], process->memsize, process->start_address);
    
    merging_holes_till_up(index);

    //after allocating new hole
    //check if new
    
 
}

void merging_holes_till_up(int index) {
    if(index >= SZ) return;
    printf("Enter index %i\n", index);
    int real_size = (1 << index);
    //printf("%d",buddy_list[index]);
    ListNode* curr = buddy_list[index]->head;
    ListNode* prev = NULL;
    while(curr) {
        if(
            !curr || !curr->next 
            || curr->start_address + curr->memsize != curr->next->start_address
            || (curr->start_address / curr->memsize) % 2 == 0
        )
            mergeNext(buddy_list[index], curr); //try to merge this hole with the hole after it
        if(curr->memsize > real_size) { //if it's size change || [merged]
            //1.allocate new hole above me
            allocate_hole_buddy(buddy_list[index+1], curr->memsize, curr->start_address);
            //2.deallocate the current hole 
            deallocate_hole(buddy_list[index], prev, curr);

            merging_holes_till_up(index+1);
            return;
        }

        prev = curr;
        curr = curr->next;
    }

}

//resumed | started => running
//running => stopped  => start new => running
//finished => start new => running


void destroy_buddy() {
    for(int i = 0;i < 10;i++) {
        free( buddy_list[i]);
    }
}

#endif