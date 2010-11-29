#include "k_primitives.h"
#include "k_structs.h"
#include "k_globals.h"

/** 5.1 Interprocess Communication **/
int k_send_message(int dest_pid, MsgEnv *msg_env)
{
    if(msg_env == NULL)
    {
        return ERROR_NULL_ARG;
    }

    if(dest_pid < 0 || dest_pid >= k_get_num_processes())
    {
        return ERROR_ILLEGAL_ARG;
    }

    msg_env->send_pid = current_process->pid;
    msg_env->dest_pid = dest_pid;

    pcb_t *dest_pcb = k_get_process(dest_pid);
    msg_env_queue_enqueue(dest_pcb ->recv_msgs, msg_env);
    if (dest_pcb->state == P_BLOCKED_ON_RECEIVE)
    {
        dest_pcb->state = P_READY;
        proc_pq_enqueue(ready_pq, dest_pcb );
    }

    //_log_msg_event(&_send_trace_buf, msg_env);

    return CODE_SUCCESS;
}

MsgEnv * k_receive_message()
{
    while (msg_env_queue_is_empty(current_process->recv_msgs))
    {
        if (current_process->is_i_process)
        {
            return NULL;
        }
        k_process_switch(P_BLOCKED_ON_RECEIVE);
    }

    MsgEnv *msg_env = msg_env_queue_dequeue(current_process->recv_msgs);
//    _log_msg_event(&_recv_trace_buf, msg_env);

    return msg_env;
}

/** 5.2 Storage Management **/
MsgEnv * k_request_msg_env()
{
    while (msg_env_queue_is_empty(free_env_q))
    {
        if (current_process->is_i_process)
        {
            return NULL;
        }
        proc_pq_enqueue(blocked_request_env_pq, current_process);
        k_process_switch(P_BLOCKED_ON_ENV_REQUEST);
    }

    MsgEnv *env = msg_env_queue_dequeue(free_env_q);
    return env;
}

int k_release_msg_env(MsgEnv * msg_env)
{
    if (msg_env == NULL)
    {
        return ERROR_NULL_ARG;
    }

    msg_env_queue_enqueue(free_env_q, msg_env);
    pcb_t * blocked_process = proc_pq_dequeue(blocked_request_env_pq);
    if (blocked_process)
    {
        blocked_process->state = P_READY;
        proc_pq_enqueue(ready_pq, blocked_process);
    }
    return CODE_SUCCESS;
}

/** 5.3 Processor Management **/
int k_release_processor()
{
    proc_pq_enqueue(ready_pq, current_process);
    k_process_switch(P_READY);
    return CODE_SUCCESS;
}

int k_request_process_status(MsgEnv *msg_env)
{
    if (msg_env == NULL)
    {
        return ERROR_NULL_ARG;
    }

    uint32_t * data = (uint32_t *) msg_env->msg;
    int pid;
    *data++ = k_get_num_processes();
    for (pid = 0; pid < k_get_num_processes(); pid++)
    {
        pcb_t * process = k_get_process(pid);
        *data++ = process->pid;
        *data++ = process->state;
        *data++ = process->priority;
    }
    return CODE_SUCCESS;
}

int k_terminate()
{
    return -1;
}

int k_change_priority(int new_priority, int target_process_id)
{
    if (new_priority < 0 || new_priority >= NUM_PRIORITIES ||
        target_process_id < 0 || target_process_id >= k_get_num_processes())
    {
        return ERROR_ILLEGAL_ARG;
    }

    pcb_t *pcb = k_get_process(target_process_id);
    if (pcb->is_i_process || pcb->pid == NULL_PID)
    {
        return ERROR_ILLEGAL_ARG;
    }

    if (pcb->priority == new_priority)
    {
        return CODE_SUCCESS;
    }

    switch (pcb->state)
    {
        case P_READY:
            proc_pq_remove(ready_pq, pcb);
            pcb->priority = new_priority;
            proc_pq_enqueue(ready_pq, pcb);
            break;

        case P_BLOCKED_ON_ENV_REQUEST:
            proc_pq_remove(blocked_request_env_pq, pcb);
            pcb->priority = new_priority;
            proc_pq_enqueue(blocked_request_env_pq, pcb);
            break;

        default:
            pcb->priority = new_priority;
            break;
    }

    return CODE_SUCCESS;
}

/** 5.4 Timing Servicies **/
int k_request_delay(int time_delay, int wakeup_code, MsgEnv *msg_env);

/** 5.5 System Console I/O **/
int k_send_console_chars(MsgEnv *msg_env);
int k_get_console_chars(MsgEnv *msg_env);

/** 5.6 Interprocess Message Trace **/
int k_get_trace_buffers(MsgEnv* msg_env);