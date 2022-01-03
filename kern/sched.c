#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/env.h>
#include <kern/monitor.h>


struct Taskstate cpu_ts;
_Noreturn void sched_halt(void);

extern volatile uint64_t s_entropy[32];
extern volatile size_t s_entropy_begin, s_entropy_end;

uint64_t prev_time = 0;
int measured_tsc = 0;

extern uint64_t InternalRdtsc();

/* Choose a user environment to run and run it */
_Noreturn void
sched_yield(void) {
    /* Implement simple round-robin scheduling.
     *
     * Search through 'envs' for an ENV_RUNNABLE environment in
     * circular fashion starting just after the env was
     * last running.  Switch to the first such environment found.
     *
     * If no envs are runnable, but the environment previously
     * running is still ENV_RUNNING, it's okay to
     * choose that environment.
     *
     * If there are no runnable environments,
     * simply drop through to the code
     * below to halt the cpu */

    // LAB 3: Your code here:
    //env_run(&envs[0]);
    int begin = curenv ? ENVX(curenv->env_id) : 0;
    int index = begin;
    bool found = false;
    if (s_entropy_begin > 0 || s_entropy_end < 32) {
        uint64_t cur_tsc = InternalRdtsc();
        if (measured_tsc == 1) {
            if (s_entropy_begin > 0) {
                s_entropy[s_entropy_begin - 1] = cur_tsc - prev_time;
                s_entropy_begin--;
            } else {
                s_entropy[s_entropy_end++] = cur_tsc - prev_time;
            }
        }
        prev_time = cur_tsc;
        measured_tsc = 1;
    }
    
    for (int i = 0; i < NENV; i++) {
        index = (begin + i) % NENV;
        if (envs[index].env_status == ENV_RUNNABLE) {
            found = true;
            break;
        }
    }
    if (found) {
        env_run(&envs[index]);
    } else if (curenv && curenv->env_status == ENV_RUNNING) {
        env_run(curenv);
    } else {
        sched_halt();
    }

    /* No runnable environments,
     * so just halt the cpu */
    //sched_halt();
}

/* Halt this CPU when there is nothing to do. Wait until the
 * timer interrupt wakes it up. This function never returns */
_Noreturn void
sched_halt(void) {

    /* For debugging and testing purposes, if there are no runnable
     * environments in the system, then drop into the kernel monitor */
    int i;
    for (i = 0; i < NENV; i++)
        if (envs[i].env_status == ENV_RUNNABLE ||
            envs[i].env_status == ENV_RUNNING) break;
    if (i == NENV) {
        cprintf("No runnable environments in the system!\n");
        for (;;) monitor(NULL);
    }

    /* Mark that no environment is running on CPU */
    curenv = NULL;

    /* Reset stack pointer, enable interrupts and then halt */
    asm volatile(
            "movq $0, %%rbp\n"
            "movq %0, %%rsp\n"
            "pushq $0\n"
            "pushq $0\n"
            "sti\n"
            "hlt\n" ::"a"(cpu_ts.ts_rsp0));

    /* Unreachable */
    for (;;)
        ;
}
