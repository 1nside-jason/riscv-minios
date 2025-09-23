void uart_puts(char *s);
// kernel/main.c
#include "printf.h"
#include "console.h"  // ✅ 必须包含！否则不认识 clear_screen, goto_xy 等
#include "uart.h"
#include "mm/pmm.h"
#include "mm/vm.h"
#include <assert.h>

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

void test_physical_memory(void) {
    printf("\n=== Testing Physical Memory ===\n");
    void *page1 = alloc_page();
    void *page2 = alloc_page();

    if (!(page1 != page2)) {
    printf("Assertion failed: page1 != page2\n");
    while(1); // 死循环
    }

    if (!(((uint64_t)page1 & 0xFFF) == 0)) {
    printf("Assertion failed: page1 not page aligned\n");
    while(1);
}

    *(int*)page1 = 0x12345678;
    assert(*(int*)page1 == 0x12345678);

    free_page(page1);
    free_page(page2);
    printf("✅ Physical memory test passed\n");
}

void test_pagetable(void) {
    printf("\n=== Testing Page Table ===\n");
    pagetable_t pt = create_pagetable();

    uint64_t va = 0x1000000;
    uint64_t pa = (uint64_t)alloc_page();
 
   if (map_page(pt, va, pa, PTE_R | PTE_W) != 0) {
    printf("Assertion failed: map_page failed\n");
    while(1);
}

    printf("Page table contents:\n");
    dump_pagetable(pt, 2);  // 2 表示从根开始打印

    destroy_pagetable(pt);
    printf("✅ Page table test passed\n");
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


    // 初始化物理内存管理器
    pmm_init();

    // 测试物理内存分配器
    test_physical_memory();

    // 测试页表功能
    test_pagetable();

    // 初始化并启用内核页表
    kvminit();
    kvminithart();

    printf("\n✅ Virtual memory enabled successfully!\n");
    printf("Testing post-paging functionality...\n");

    // 测试：启用分页后仍能输出
    printf("Hello from virtual memory world!\n");



    while(1); // 死循环
    return 0; // 满足编译器要求
}
