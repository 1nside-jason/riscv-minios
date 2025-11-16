// kernel/proc/proc.h
#ifndef __PROC_H__
#define __PROC_H__

#include "riscv.h"

// ✅ 完整定义 enum 和 struct
enum procstate { UNUSED, EMBRYO, RUNNABLE, RUNNING, SLEEPING, ZOMBIE };

struct context {
    uint64_t ra;
    uint64_t sp;
};

struct trapframe {
    uint64_t epc;
    uint64_t ra;
    uint64_t sp;
    // ... 其他寄存器（至少要有 a0-a7）
    uint64_t a0, a1, a2, a3, a4, a5, a6, a7;
    // 其他可省略（简化实验）
};


// 进程结构体
struct proc {
    enum procstate state;      // ✅ 现在完整了
    int pid;
    struct context context;    // ✅ 现在完整了
    pagetable_t pagetable;
    uint64_t kstack;
    void (*entry)(void);
    int exit_status;
    int parent;
    struct trapframe *trapframe;  // 确保也定义了 trapframe
};




// ✅ 声明全局变量和函数
extern struct proc proc[];        // 进程表
extern struct proc *current_proc; // 当前进程

void proc_init(void);
int create_process(void (*entry)(void));
void exit_process(int status);
int wait_process(int *status);
void scheduler(void) __attribute__((noreturn));

// ✅ 声明系统调用桩函数（供用户任务使用）
int getpid(void);
int write(int fd, const void *buf, int count);
void exit(int status);  // 注意：这是用户接口，非 exit_process

#endif
