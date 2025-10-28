// include/trap.h
#ifndef __TRAP_H__
#define __TRAP_H__

//声明汇编中定义的符号
extern void kernelvec(void);

void trap_init(void);
void kerneltrap(void);

// SBI 调用（用于设置时钟）
void sbi_set_timer(uint64_t stime_value);

#endif
