// kernel/mm/riscv.h
#ifndef __RISCV_H__
#define __RISCV_H__

#include <stdint.h>
#include <stddef.h>

// 页大小 4KB
#define PGSIZE 4096
#define PGSHIFT 12

// 页对齐宏
#define PGROUNDUP(sz)  (((sz) + PGSIZE - 1) & ~(PGSIZE - 1))
#define PGROUNDDOWN(a) ((a) & ~(PGSIZE - 1))

// Sv39 虚拟地址布局
#define KERNBASE 0x80000000L  // 内核起始虚拟地址
#define PHYSTOP  0x88000000L  // 物理内存结束地址（128MB）

// UART 设备物理地址（QEMU virt 平台）
#define UART0 0x10000000L

// 页表项（PTE）相关
typedef uint64_t pte_t;
typedef uint64_t* pagetable_t;

// 权限位
#define PTE_V (1L << 0)  // Valid
#define PTE_R (1L << 1)  // Read
#define PTE_W (1L << 2)  // Write
#define PTE_X (1L << 3)  // Execute
#define PTE_U (1L << 4)  // User

// 从 PTE 提取物理页号（PPN）
#define PTE2PPN(pte) (((pte) >> 10) << 12)
#define PPN2PTE(ppn) (((ppn) >> 12) << 10)

// 从虚拟地址提取 VPN[level]
#define VPN_SHIFT(level) (12 + 9 * (level))
#define VPN_MASK(va, level) (((va) >> VPN_SHIFT(level)) & 0x1FF)

// SATP 寄存器操作
static inline uint64_t r_satp() {
    uint64_t x;
    asm volatile("csrr %0, satp" : "=r" (x));
    return x;
}

static inline void w_satp(uint64_t x) {
    asm volatile("csrw satp, %0" : : "r" (x));
}

// 刷新 TLB
static inline void sfence_vma() {
    asm volatile("sfence.vma");
}

// 读取机器时间（mtime）
static inline uint64_t r_time() {
    uint64_t x;
    asm volatile("rdtime %0" : "=r" (x));
    return x;
}


// SATP 寄存器值构造宏
#define MAKE_SATP(pagetable) (((uint64_t)(pagetable) >> 12) | (8ULL << 60))



// ========== CSR 寄存器操作 ==========
// Machine Mode
static inline uint64_t r_mstatus() { uint64_t x; asm volatile("csrr %0, mstatus" : "=r" (x)); return x; }
static inline void w_mstatus(uint64_t x) { asm volatile("csrw mstatus, %0" :: "r" (x)); }
static inline uint64_t r_mie() { uint64_t x; asm volatile("csrr %0, mie" : "=r" (x)); return x; }
static inline void w_mie(uint64_t x) { asm volatile("csrw mie, %0" :: "r" (x)); }
static inline uint64_t r_mip() { uint64_t x; asm volatile("csrr %0, mip" : "=r" (x)); return x; }
static inline uint64_t r_medeleg() { uint64_t x; asm volatile("csrr %0, medeleg" : "=r" (x)); return x; }
static inline void w_medeleg(uint64_t x) { asm volatile("csrw medeleg, %0" :: "r" (x)); }
static inline uint64_t r_mideleg() { uint64_t x; asm volatile("csrr %0, mideleg" : "=r" (x)); return x; }
static inline void w_mideleg(uint64_t x) { asm volatile("csrw mideleg, %0" :: "r" (x)); }
static inline void w_mtvec(uint64_t x) { asm volatile("csrw mtvec, %0" :: "r" (x)); }
static inline uint64_t r_mcause() { uint64_t x; asm volatile("csrr %0, mcause" : "=r" (x)); return x; }
static inline uint64_t r_mepc() { uint64_t x; asm volatile("csrr %0, mepc" : "=r" (x)); return x; }
static inline void w_mepc(uint64_t x) { asm volatile("csrw mepc, %0" :: "r" (x)); }

// Supervisor Mode
static inline uint64_t r_sstatus() { uint64_t x; asm volatile("csrr %0, sstatus" : "=r" (x)); return x; }
static inline void w_sstatus(uint64_t x) { asm volatile("csrw sstatus, %0" :: "r" (x)); }
static inline uint64_t r_sie() { uint64_t x; asm volatile("csrr %0, sie" : "=r" (x)); return x; }
static inline void w_sie(uint64_t x) { asm volatile("csrw sie, %0" :: "r" (x)); }
static inline uint64_t r_sip() { uint64_t x; asm volatile("csrr %0, sip" : "=r" (x)); return x; }
static inline uint64_t r_scause() { uint64_t x; asm volatile("csrr %0, scause" : "=r" (x)); return x; }
static inline uint64_t r_sepc() { uint64_t x; asm volatile("csrr %0, sepc" : "=r" (x)); return x; }
static inline void w_sepc(uint64_t x) { asm volatile("csrw sepc, %0" :: "r" (x)); }
static inline void w_stvec(uint64_t x) { asm volatile("csrw stvec, %0" :: "r" (x)); }


#endif
