// include/trap.h
#ifndef __TRAP_H__
#define __TRAP_H__

#include <stdint.h>          // ✅ 必须包含！定义 uint64_t 等类型
#include "riscv.h"           // 可选，但建议保留（如果你在 trap.h 中用到 CSR）

//声明汇编中定义的符号
extern void kernelvec(void);

void trap_init(void);
void kerneltrap(void);

// SBI 调用（用于设置时钟）
void sbi_set_timer(uint64_t stime_value);

#endif
