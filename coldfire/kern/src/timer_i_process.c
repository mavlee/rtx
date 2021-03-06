#include "timer_i_process.h"
#include "rtx.h"
#include "k_clock.h"
#include "k_primitives.h"
#include "timeout_queue.h"
#include "trace.h"

void timer_i_process()
{
    //Acknowledge the interrupt
    TIMER0_TER = 2;

    // Record a tick
    k_clock_incr_system_time();

    MsgEnv* msg_env = k_receive_message();
    while (msg_env != NULL)
    {
        timeout_queue_insert(msg_env);
        msg_env = k_receive_message();
    }

    msg_env = check_timeout_queue(msg_env);
    if (msg_env != NULL)
    {
        // Send the envelope back
        k_send_message(msg_env->send_pid, msg_env);
    }
}
