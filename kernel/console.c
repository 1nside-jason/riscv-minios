// kernel/console.c
#include "console.h"
#include "uart.h"  // ✅ 确保包含 uart.h，因为 console_putc 调用 uart_putc
#include "printf.h"

void console_putc(char c) {
    uart_putc(c);
}

void console_puts(const char *s) {
    if (s == 0) {
        console_putc('(');
        console_putc('n');
        console_putc('u');
        console_putc('l');
        console_putc('l');
        console_putc(')');
        return;
    }
    while (*s) {
        console_putc(*s);
        s++;
    }
}

// 清屏 + 光标归位
void clear_screen(void) {
    console_puts("\033[2J\033[H");
}

// 光标定位 (x:列, y:行，从1开始)
void goto_xy(int x, int y) {
    printf("\033[%d;%dH", y, x);
}

// 设置前景色（简化版）
void set_color(int fg) {
    printf("\033[%dm", fg);
}

// 重置颜色
void reset_color(void) {
    printf("\033[0m");
}
