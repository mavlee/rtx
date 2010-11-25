#ifndef _KERN_GLOBALS_H_
#define _KERN_GLOBALS_H_

#include "rtx.h"
#include "k_structs.h"

// Processes
#define NUM_PROCESSES 3

// Process priorities
#define NUM_PRIORITIES 3
#define ADDITIONAL_NULL_PRIORITY 1
#define HIGHEST_PRIORITY 0
#define LOWEST_PRIORITY  2
#define NULL_PRIORITY    3

pcb_t * k_get_process(int32_t pid);

// Scheduling
extern proc_pq_t ready_pq;
extern proc_pq_t blocked_request_env_pq;

bool k_process_switch(enum process_state transition_to);
bool k_context_switch(pcb_t * prev, pcb_t * next);

#endif