// kernel/mm/pmm.c
#include "riscv.h"
#include "printf.h"
#include "mm/pmm.h"
extern char end[];

// 空闲页链表节点
struct run {
    struct run *next;
};

// 空闲页链表头
static struct run *freelist;

// 初始化物理内存管理器
void pmm_init(void) {
    // 可用物理内存范围：从 _end 到 PHYSTOP
    extern char end[];  // 由链接脚本定义
    uint64_t start = PGROUNDUP((uint64_t)end);
    uint64_t stop = PHYSTOP;

    printf("pmm_init: memory range [0x%p - 0x%p]\n", start, stop);

    // 按页倒序插入空闲链表（简单实现）
    freelist = 0;
    for (uint64_t p = stop - PGSIZE; p >= start; p -= PGSIZE) {
        struct run *r = (struct run *)p;
        r->next = freelist;
        freelist = r;
    }
}

// 分配一页物理内存
void* alloc_page(void) {
    if (freelist == 0) {
        printf("alloc_page: out of memory\n");
        return 0;
    }
    struct run *r = freelist;
    freelist = freelist->next;
    return (void*)r;
}


// 释放一页物理内存
void free_page(void *pa) {
    extern char end[];  // ✅ 添加这一行！声明 end 符号

    if (((uint64_t)pa % PGSIZE) != 0 || (char*)pa < (char*)PGROUNDUP((uint64_t)&end)) {
        printf("free_page: invalid address 0x%p\n", pa);
        return;
    }

    struct run *r = (struct run *)pa;
    r->next = freelist;
    freelist = r;
}
