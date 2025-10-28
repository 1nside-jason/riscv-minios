// kernel/trap/trap.c
#include "riscv.h"
#include "printf.h"
#include "trap/trap.h"

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

    // 判断是否为时钟中断（scause = 5）
    if (scause == 5) {
        timer_ticks++;
        // 设置下次中断：每 1000000 周期一次（QEMU 默认频率）
        sbi_set_timer(r_time() + 1000000);

        // 简单调度：每 5 次中断切换一次输出颜色
        if (timer_ticks % 5 == 0) {
            printf("\n⏰ Timer tick %d - ", timer_ticks);
            if (timer_ticks % 10 == 0) {
                printf("Switching to RED output\n");
                // 这里可以设置全局状态，影响后续输出
            } else {
                printf("Switching to GREEN output\n");
            }
        }
    } else {
        printf("Unexpected trap: scause=0x%lx sepc=0x%lx\n", scause, sepc);
        while(1); // 死循环
    }

    // 更新 sepc（如果需要）
    w_sepc(sepc);
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
