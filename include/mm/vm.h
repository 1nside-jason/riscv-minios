// include/mm/vm.h
#ifndef __VM_H__
#define __VM_H__

#include "../riscv.h"

// 页表类型已在 riscv.h 中定义

// 函数声明
void kvminit(void);
void kvminithart(void);
pagetable_t create_pagetable(void);
int map_page(pagetable_t pt, uint64_t va, uint64_t pa, int perm);
void destroy_pagetable(pagetable_t pt);
void dump_pagetable(pagetable_t pt, int level);

#endif
