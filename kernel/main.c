void uart_puts(char *s);
// kernel/main.c
#include "printf.h"
#include "console.h"  // ✅ 必须包含！否则不认识 clear_screen, goto_xy 等
#include "uart.h"

void test_printf_basic() {
    printf("Testing integer: %d\n", 42);
    printf("Testing negative: %d\n", -123);
    printf("Testing zero: %d\n", 0);
    printf("Testing hex: 0x%x\n", 0xABC);
    printf("Testing string: %s\n", "Hello OS");
    printf("Testing char: %c\n", 'X');
    printf("Testing percent: %%\n");
    printf("Testing pointer: %p\n", (void*)0x80001234);
}

void test_printf_edge_cases() {
    printf("INT_MAX: %d\n", 2147483647);
    printf("INT_MIN: %d\n", -2147483648);
    printf("NULL string: %s\n", (char*)0);
    printf("Empty string: %s\n", "");
}

// ✅ 修改：返回 int，虽然我们不使用返回值
int main() {
    uart_init();  // 初始化 UART

    clear_screen();      // 清屏
    goto_xy(10, 5);      // 光标定位到第5行第10列
    set_color(31);       // 设置红色前景
    printf("🌟 Welcome to RISC-V MiniOS 🌟\n");
    reset_color();       // 重置颜色

    test_printf_basic();
    test_printf_edge_cases();

    while(1); // 死循环
    return 0; // 满足编译器要求
}
