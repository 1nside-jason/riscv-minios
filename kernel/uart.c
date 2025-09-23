
// kernel/uart.c
#include "uart.h"

#define UART0_BASE 0x10000000L
#define UART0_THR  (UART0_BASE + 0x00)
#define UART0_LSR  (UART0_BASE + 0x05)
#define LSR_THRE   (1 << 5)

// 初始化UART（目前为空，以后可扩展）
void uart_init(void) {
    // QEMU virt 平台无需初始化，硬件默认可用
}

// 发送一个字符
void uart_putc(char c) {
    // 等待发送缓冲区空
    while ((*(volatile char*)UART0_LSR & LSR_THRE) == 0)
        ;
    // 发送字符
    *(volatile char*)UART0_THR = c;

    // 处理换行：输出 \n 时自动补 \r
    if (c == '\n') {
        uart_putc('\r');
    }
}
