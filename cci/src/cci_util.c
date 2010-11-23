#include "cci_util.h"
#include "rtx.h"
#include "processes.h"
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

//like CCI_printf, except for the CCI. and possibly broken.
int CCI_printf (const char* format, ...);

//prints process statuses on console given the envelope message data
int CCI_printProcessStatuses (char* raw_data)
{
    if (raw_data == NULL)
        return ERROR_NULL_ARG;
    int * data = (int *) raw_data;
    int num_processes = *data++;
    int i;
    CCI_printf ("PID | STATUS                | PRIORITY\n");
    for (i=0;i<num_processes;i++)
    {
        CCI_printf("  %d   ",*data++);
        switch(*data)
        {
            case P_READY:
                CCI_printf("ready                  ");
                break;
            case P_EXECUTING:
                CCI_printf("executing              ");
                break;
            case P_BLOCKED_ON_ENV_REQUEST:
                CCI_printf("blocked on env request ");
                break;
            case P_BLOCKED_ON_RECEIVE:
                CCI_printf("blocked on receive     ");
                break;
            default :
                CCI_printf("                       ");
                break;
        }
        data++;
        CCI_printf(" %d\n",*data++);
    }
    return CODE_SUCCESS;
}

//sets the clock 
int CCI_setClock (char* timeParam, uint32_t* time)
{
    if (timeParam == NULL)
        return ERROR_NULL_ARG;
    if (timeParam[2] != ':' || timeParam[5] != ':')
        return ERROR_ILLEGAL_ARG;

    char hr_s [3];
    char min_s [3];
    char sec_s [3];
    int i, hr, min, sec;
    for (i=0;i<2;i++)
    {
        hr_s[i] =timeParam[i];
        min_s[i]=timeParam[3+i];
        sec_s[i]=timeParam[6+i];
    }
    hr_s[2]='\0';
    min_s[2]='\0';
    sec_s[2]='\0';

    hr = atoi(hr_s); 
    min = atoi(min_s);
    sec = atoi(sec_s);
    if (hr>23 || min>59 || sec > 59)
        return ERROR_ILLEGAL_ARG;
    *time = hr*3600 + min*60 + sec;
    return CODE_SUCCESS;
}

//prints trace buffers on console given the envelope message data
int CCI_printTraceBuffers (char* data)
{
    if (data == NULL)
        return ERROR_NULL_ARG;
    int i;
    ipc_trace_t *send_dump = (ipc_trace_t *) data;
    ipc_trace_t *recv_dump = send_dump + IPC_MESSAGE_TRACE_HISTORY_SIZE; 
 
    CCI_printf(    "MESSAGE TRACE BUFFERS\n\n"
                   " Send Trace                   || Receive Trace\n"
                   " Dest |Sender|Message|  Time  || Dest |Sender|Message|  Time\n"
                   " PID  |PID   |Type   |        || PID  |PID   |Type   |\n"
                   "--------------------------------------------------------------\n");
    for (i=0;i<IPC_MESSAGE_TRACE_HISTORY_SIZE;i++)
    {
        if (send_dump[i].time_stamp != 0)
        {
            CCI_printf("   %2u |   %2u |   %3d | %6lu ||",
                       send_dump[i].dest_pid, send_dump[i].send_pid,
                       send_dump[i].msg_type, send_dump[i].time_stamp);
        }
        else if (recv_dump[i].time_stamp != 0)
        {
            CCI_printf("      |      |       |       ||");
        }
        else
        {
            break;
        }
        
        if (recv_dump[i].time_stamp != 0)
        {
            CCI_printf("   %2u |   %2u |   %3d | %6lu\n",send_dump[i].dest_pid,
                       recv_dump[i].send_pid, recv_dump[i].msg_type,
                       recv_dump[i].time_stamp);
        }
        else
        {
            CCI_printf("      |      |       |       \n");
        }
    }        
    
    CCI_printf("\n");

    return CODE_SUCCESS;
}

//set process priority based on params given provided
int CCI_setNewPriority (char* param)
{
    if (param == NULL)
        return ERROR_NULL_ARG;

    char* priorityStr;
    char* pidStr;
    int priority, pid;
    priorityStr = strtok (param," \t");
    pidStr = strtok(NULL," \t");
    priority = atoi(priorityStr);
    pid = atoi(pidStr);
	
	CCI_printf("The priority submitted is %d and the pid is %d \n", priority, pid);
	
    return change_priority(priority, pid);
}

