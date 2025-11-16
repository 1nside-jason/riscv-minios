// kernel/proc/proc.c
#include "mm/pmm.h"
#include "printf.h"
#include "trap/trap.h"
#include "proc/proc.h"

struct proc proc[NPROC];
struct proc *current_proc = 0;

static int next_pid = 1;

// 分配内核栈（1页）
static uint64_t alloc_kstack() {
    return (uint64_t)alloc_page();
}

// 释放内核栈
static void free_kstack(uint64_t kstack) {
    free_page((void*)kstack);
}

// 初始化进程系统
void proc_init(void) {
    for (int i = 0; i < NPROC; i++) {
        proc[i].state = UNUSED;
    }
    printf("proc_init: process system initialized\n");
}

// 创建新进程
int create_process(void (*entry)(void)) {
    for (int i = 0; i < NPROC; i++) {
        if (proc[i].state == UNUSED) {
            struct proc *p = &proc[i];
            p->pid = next_pid++;
            p->entry = entry;
            p->kstack = alloc_kstack();
            if (p->kstack == 0) {
                printf("create_process: out of memory\n");
                return -1;
            }

            // 设置初始上下文：entry 是入口，kstack 是栈
            p->context.sp = p->kstack + PGSIZE;  // 栈顶
            p->context.ra = (uint64_t)entry;     // 返回地址 = 入口

            p->state = RUNNABLE;
            printf("create_process: PID %d created\n", p->pid);
            return p->pid;
        }
    }
    printf("create_process: process table full\n");
    return -1;
}

// 退出当前进程
void exit_process(int status) {
    if (current_proc) {
        current_proc->exit_status = status;
        current_proc->state = ZOMBIE;
        printf("Process %d exited with status %d\n", current_proc->pid, status);
    }
    // 触发调度
    //scheduler();
}

// 等待子进程（简化：等待任意进程）
int wait_process(int *status) {
    while (1) {
        for (int i = 0; i < NPROC; i++) {
            if (proc[i].state == ZOMBIE) {
                int pid = proc[i].pid;
                if (status) *status = proc[i].exit_status;
                free_kstack(proc[i].kstack);
                proc[i].state = UNUSED;
                return pid;
            }
        }
        // 简单轮询（实际应 sleep）
    }
}

// 调度器（轮转）
void scheduler(void) {
    while (1) {
        // 允许中断（时钟中断可能唤醒新进程）
        // 在实验4中已全局开中断

        for (int i = 0; i < NPROC; i++) {
            if (proc[i].state == RUNNABLE) {
                struct proc *p = &proc[i];
                p->state = RUNNING;
                current_proc = p;

                // 切换到进程上下文
                swtch(&proc[0].context, &p->context);

                // 返回后，进程已让出
                current_proc = 0;
                p->state = RUNNABLE;  // 下次可再调度
            }
        }
    }
}
