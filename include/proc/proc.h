// include/proc/proc.h
#ifndef __PROC_H__
#define __PROC_H__

#include "riscv.h"

#define NPROC 8
#define PGSIZE 4096

enum procstate { UNUSED, EMBRYO, RUNNABLE, RUNNING, SLEEPING, ZOMBIE };

struct context {
    uint64_t ra;
    uint64_t sp;
};

struct trapframe {
    uint64_t epc;
    uint64_t ra, sp, gp, tp;
    uint64_t t0, t1, t2;
    uint64_t s0, s1;
    uint64_t a0, a1, a2, a3, a4, a5, a6, a7;
    uint64_t s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
    uint64_t t3, t4, t5, t6;
};

struct proc {
    enum procstate state;
    int pid;
    struct context context;
    pagetable_t pagetable;
    uint64_t kstack;
    void (*entry)(void);
    int exit_status;
    int parent;
    struct trapframe *trapframe;  // ✅ 必须有这一行
};

extern struct proc proc[NPROC];
extern struct proc *current_proc;

void proc_init(void);
int create_process(void (*entry)(void));
void exit_process(int status);
void scheduler(void) __attribute__((noreturn));
void swtch(struct context *old, struct context *new);

#endif
