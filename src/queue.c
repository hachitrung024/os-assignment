#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        q->proc[q->size] = proc;              // Them process vao queue
        q->size++;
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
        // if(empty(q)) return NULL;
        struct pcb_t * target_proc = NULL;   // pcb can deqeueu
        // uint8_t target_index = 0;       // Vi tri cua pcb
        // uint32_t min_priority_val = q->proc[0]->priority; // gia tri priorty nho nhat
        // // Tim proc co gia tri priority nho nhat
        // for(uint8_t i = 0; i < q->size; i++){
        //         if(q->proc[i]->priority < min_priority_val) {
        //                 min_priority_val = q->proc[i]->priority;
        //                 target_index = i;
        //         }
        // }
        target_proc = q->proc[0];    // pcb tra ve
        // Xoa pcb khoi queue
        int i;
        for( i= 0; i< q->size -1; i++){
                q->proc[i] = q->proc[i+1];
        }
        q->size --;
        return target_proc;
}

