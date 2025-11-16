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

struct proc {
    enum procstate state;
    int pid;
    struct context context;
    pagetable_t pagetable;
    uint64_t kstack;
    void (*entry)(void);
    int exit_status;
    int parent;
};

extern struct proc proc[NPROC];
extern struct proc *current_proc;

void proc_init(void);
int create_process(void (*entry)(void));
void exit_process(int status);
void scheduler(void) __attribute__((noreturn));
void swtch(struct context *old, struct context *new);

#endif
