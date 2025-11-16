void uart_puts(char *s);
// kernel/main.c
#include "printf.h"
#include "console.h"  // ✅ 必须包含！否则不认识 clear_screen, goto_xy 等
#include "uart.h"
#include "mm/pmm.h"
#include "mm/vm.h"
#include <assert.h>
#include "trap/trap.h"
#include "proc/proc.h"

// 测试任务1
void task1(void) {
    int count = 0;
    while (1) {
        printf("Task1 [%d]: tick %d\n", current_proc ? current_proc->pid : -1, count++);
        if (count > 10) {
            exit_process(0);
        }
        // 模拟工作负载
        for (volatile int i = 0; i < 500000; i++);
    }
}

// 测试任务2
void task2(void) {
    int count = 0;
    while (1) {
        printf("Task2 [%d]: tick %d\n", current_proc ? current_proc->pid : -1, count++);
        if (count > 10) {
            exit_process(0);
        }
        for (volatile int i = 0; i < 600000; i++);
    }
}

// 测试任务3（可选）
void task3(void) {
    int count = 0;
    while (1) {
        printf("Task3 [%d]: tick %d\n", current_proc ? current_proc->pid : -1, count++);
        if (count > 5) {
            exit_process(0);
        }
        for (volatile int i = 0; i < 700000; i++);
    }
}

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
        while(1);
    }

    if (!(((uint64_t)page1 & 0xFFF) == 0)) {
        printf("Assertion failed: page1 not page aligned\n");
        while(1);
    }

    *(int*)page1 = 0x12345678;
    if (*(int*)page1 != 0x12345678) {
        printf("Memory write test failed\n");
        while(1);
    }

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
    dump_pagetable(pt, 2);

    destroy_pagetable(pt);
    printf("✅ Page table test passed\n");
}

int main() {
    uart_init();
    clear_screen();
    goto_xy(5, 3);
    set_color(32); // 绿色
    printf("🚀 RISC-V MiniOS - Process & Scheduling Lab\n");
    reset_color();

    // 基础测试
    test_printf_basic();
    test_printf_edge_cases();

    // 内存与页表初始化
    pmm_init();
    test_physical_memory();
    test_pagetable();

    kvminit();
    kvminithart();

    // 中断系统初始化
    trap_init();

    // ✅ 关键：初始化进程系统
    proc_init();

    printf("\n✅ Creating processes...\n");

    // ✅ 创建多个进程
    if (create_process(task1) <= 0) {
        printf("Failed to create task1\n");
    }
    if (create_process(task2) <= 0) {
        printf("Failed to create task2\n");
    }
    if (create_process(task3) <= 0) {
        printf("Failed to create task3\n");
    }

    printf("✅ All processes created. Starting scheduler...\n");

    // ✅ 启动调度器（永不返回）
    scheduler();

    // 不可达
    return 0;
}


