// kernel/syscall.c
#include "syscall.h"
#include "proc/proc.h"
#include "printf.h"
#include "uart.h"


// 声明系统调用实现函数
int sys_getpid(void);
int sys_fork(void);
int sys_exit(void);
int sys_wait(void);
int sys_write(void);

// 系统调用分发表
static int (*syscalls[])(void) = {
    [SYS_getpid] = sys_getpid,
    [SYS_fork]   = sys_fork,
    [SYS_exit]   = sys_exit,
    [SYS_wait]   = sys_wait,
    [SYS_write]  = sys_write,
};

// 参数提取：从 trapframe 获取 a0-a6
static int argint(int n, int *ip) {
    struct proc *p = current_proc;
    switch (n) {
        case 0: *ip = p->trapframe->a0; break;
        case 1: *ip = p->trapframe->a1; break;
        case 2: *ip = p->trapframe->a2; break;
        default: return -1;
    }
    return 0;
}

static uint64_t argaddr(int n) {
    struct proc *p = current_proc;
    switch (n) {
        case 0: return p->trapframe->a0;
        case 1: return p->trapframe->a1;
        case 2: return p->trapframe->a2;
        default: return 0;
    }
}

// 系统调用分发
void syscall_dispatch(void) {
    struct proc *p = current_proc;
    if (!p) return;

    int num = p->trapframe->a7;  // 系统调用号在 a7
    if (num > 0 && num < sizeof(syscalls)/sizeof(syscalls[0]) && syscalls[num]) {
        int ret = syscalls[num]();
        p->trapframe->a0 = ret;  // 返回值放 a0
    } else {
        printf("Unknown syscall %d\n", num);
        p->trapframe->a0 = -1;
    }
}



int sys_getpid(void) {
    return current_proc->pid;
}

int sys_exit(void) {
    int status;
    argint(0, &status);
    exit_process(status);
    return 0; // 不可达
}

int sys_fork(void) {
    // 简化：暂不实现（返回 -1）
    return -1;
}

int sys_wait(void) {
    int *status;
    if (argint(0, (int*)&status) < 0) return -1;
    return wait_process(status);
}

int sys_write(void) {
    int fd, count;
    uint64_t buf;
    argint(0, &fd);
    buf = argaddr(1);
    argint(2, &count);

    if (fd != 1) return -1;  // 只支持 stdout (fd=1)
    if (buf == 0 || count < 0) return -1;

    // 简单：直接输出到 console
    char *s = (char*)buf;
    for (int i = 0; i < count; i++) {
        uart_putc(s[i]);
    }
    return count;
}
