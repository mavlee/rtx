#include "k_process.h"
#include "k_config.h"
#include "k_globals.h"
#include "k_scheduler.h"

#include <stdlib.h>
#include <setjmp.h>

pcb_t * current_process;
pcb_t   p_table[NUM_PROCESSES];

int k_release_processor()
{
    proc_pq_enqueue(ready_pq, current_process);
    k_process_switch(P_READY);
    return CODE_SUCCESS;
}

int k_request_process_status(MsgEnv *msg_env)
{
    uint32_t * data = (uint32_t *) msg_env->msg;
    int i;
    *data++ = NUM_PROCESSES;
    for (i = 0; i < NUM_PROCESSES; i++)
    {
        *data++ = p_table[i].pid;
        *data++ = p_table[i].status;
        *data++ = p_table[i].priority;
    }
    return CODE_SUCCESS;
}

int k_terminate()
{
    assert(0);
    return -1;
}

int k_change_priority(int new_priority, int target_process_id)
{
    if (new_priority < 0 || new_priority >= NUM_PRIORITIES ||
            target_process_id < 0 || target_process_id >= NUM_PROCESSES)
    {
        return ERROR_ILLEGAL_ARG;
    }

    pcb_t *pcb = &p_table[target_process_id];
    switch (pcb->status)
    {
        case P_READY:
            proc_pq_remove(ready_pq, pcb);
            pcb->priority = new_priority;
            proc_pq_enqueue(ready_pq, pcb);
            break;

        case P_BLOCKED_ON_ENV_REQUEST:
            proc_pq_remove(env_blocked_pq, pcb);
            pcb->priority = new_priority;
            proc_pq_enqueue(env_blocked_pq, pcb);
            break;

        default:
            pcb->priority = new_priority;
            break;
    }

    return CODE_SUCCESS;
}

void k_init_processes(proc_cfg_t init_table[])
{
    jmp_buf init_buf;
    int i;
    for (i = 0; i < NUM_PROCESSES; i++)
    {
        pcb_t *pcb = &p_table[i];
        proc_cfg_t *cfg = &init_table[i];
        pcb->pid = cfg->pid;
        pcb->priority = cfg->priority;
        pcb->name = cfg->name;
        pcb->is_i_process = cfg->is_i_process;
        pcb->start = cfg->start;
        pcb->recv_msgs = msg_env_queue_create();
        pcb->next = NULL;
        pcb->status = P_READY;
        pcb->stack_end = malloc(STACK_SIZE);

        // Initialize the stack and start pc
        if (setjmp(init_buf) ==  0)
        {
            char * stack_top = pcb->stack_end + STACK_SIZE - STACK_OFFSET;
            __asm__("movl %0, %%esp":"=g" (stack_top));
            if (setjmp(pcb->context) == 0)
            {
                longjmp(init_buf, 1);
            }
            else
            {
                current_process->start();
            }
        }

        // If the process is not an i process place it on the ready queue
        if (!pcb->is_i_process)
        {
            proc_pq_enqueue(ready_pq, pcb);
        }
    }
}
