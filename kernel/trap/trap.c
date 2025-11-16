// kernel/trap/trap.c
#include "riscv.h"
#include "printf.h"
#include "trap/trap.h"
#include "proc/proc.h"
#include "syscall.h"


// 全局变量：记录时钟中断次数
volatile int timer_ticks = 0;

// SBI 调用：设置下次时钟中断
void sbi_set_timer(uint64_t stime_value) {
    register uint64_t a0 asm("a0") = stime_value;
    register uint64_t a7 asm("a7") = 0; // SBI_SET_TIMER
    asm volatile ("ecall"
                  : "+r"(a0)
                  : "r"(a7)
                  : "memory");
}

// 获取当前时间（简化）
uint64_t get_time(void) {
    // RISC-V 没有直接读取时间的 CSR，依赖 SBI 或 mtime
    // 这里用 timer_ticks 近似
    return timer_ticks;
}

// 内核态中断处理函数
void kerneltrap(void) {
    uint64_t scause = r_scause();
    uint64_t sepc = r_sepc();

    if (scause == 5) {
        // 时钟中断
        timer_ticks++;
        sbi_set_timer(r_time() + 1000000);
        if (timer_ticks % 10 == 0) {
            if (current_proc) {
                swtch(&current_proc->context, &proc[0].context);
            }
        }
    } else if (scause == 8) {
        // 👉 系统调用
        if (current_proc) {
            // 保存 sepc 到 trapframe
            current_proc->trapframe->epc = sepc;

            // 👉 调用系统调用分发器
            syscall_dispatch();

            // 更新 sepc：跳过 ecall 指令
            w_sepc(sepc + 4);
        }
    } else {
        printf("Unexpected trap: scause=0x%lx sepc=0x%lx\n", scause, sepc);
        while(1);
    }
}

// 初始化中断系统
void trap_init(void) {
    printf("trap_init: setting up interrupt handling...\n");

    // 1. 委托时钟中断到 S 模式
    w_mideleg(r_mideleg() | (1L << 5));  // bit 5 = supervisor timer interrupt

    // 2. 设置 S 模式中断向量
    w_stvec((uint64_t)kernelvec);

    // 3. 开启 S 模式时钟中断
    w_sie(r_sie() | (1L << 5));

    // 4. 全局开启中断（S 模式）
    w_sstatus(r_sstatus() | (1L << 1)); // SIE bit in sstatus

    // 5. 设置第一次时钟中断
    sbi_set_timer(r_time() + 1000000);

    printf("trap_init: interrupt system ready\n");
}
