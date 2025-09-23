// kernel/mm/vm.c
#include "riscv.h"
#include "mm/pmm.h"
#include "printf.h"
#include "mm/vm.h"
extern char etext[], end[];

// 创建新页表（分配根页表）
pagetable_t create_pagetable(void) {
    pagetable_t pt = (pagetable_t)alloc_page();
    if (pt == 0) {
        printf("create_pagetable: failed\n");
        return 0;
    }
    for (int i = 0; i < 512; i++) {
        pt[i] = 0;
    }
    return pt;
}

// 页表遍历（查找或创建）
static pte_t* walk(pagetable_t pt, uint64_t va, int alloc) {
    for (int level = 2; level >= 0; level--) {
        pte_t *pte = &pt[VPN_MASK(va, level)];
        if (*pte & PTE_V) {
            if (level == 0) return pte;  // 找到叶子
            pt = (pagetable_t)PTE2PPN(*pte);
        } else {
            if (!alloc) return 0;
            pagetable_t new_pt = (pagetable_t)alloc_page();
            if (new_pt == 0) return 0;
            for (int i = 0; i < 512; i++) {
                new_pt[i] = 0;
            }
            *pte = PPN2PTE((uint64_t)new_pt) | PTE_V | PTE_R | PTE_W | PTE_X | PTE_U;
            pt = new_pt;
        }
    }
    return 0;
}

// 映射一页：va → pa
int map_page(pagetable_t pt, uint64_t va, uint64_t pa, int perm) {
    if ((va % PGSIZE) != 0 || (pa % PGSIZE) != 0) {
        printf("map_page: addresses not aligned\n");
        return -1;
    }

    pte_t *pte = walk(pt, va, 1);
    if (pte == 0) {
        printf("map_page: walk failed\n");
        return -1;
    }

    if (*pte & PTE_V) {
        printf("map_page: va 0x%p already mapped\n", va);
        return -1;
    }

    *pte = PPN2PTE(pa) | perm | PTE_V;
    return 0;
}

// 销毁页表（递归释放）
void destroy_pagetable(pagetable_t pt) {
    if (pt == 0) return;
    for (int i = 0; i < 512; i++) {
        if (pt[i] & PTE_V && (pt[i] & (PTE_R | PTE_W | PTE_X)) != 0) {
            // 是中间页表，递归销毁
            destroy_pagetable((pagetable_t)PTE2PPN(pt[i]));
        }
    }
    free_page(pt);
}

// 调试：打印页表
void dump_pagetable(pagetable_t pt, int level) {
    if (pt == 0) return;
    for (int i = 0; i < 512; i++) {
        if (pt[i] & PTE_V) {
            if (level > 0 && (pt[i] & (PTE_R | PTE_W | PTE_X)) != 0) {
                // 中间页表
                dump_pagetable((pagetable_t)PTE2PPN(pt[i]), level - 1);
            } else {
                // 叶子页表项
                printf("VA: 0x%lx -> PA: 0x%lx, perm: %c%c%c%c\n",
                       ((uint64_t)level << VPN_SHIFT(2)) | (i << VPN_SHIFT(level)),
                       PTE2PPN(pt[i]),
                       (pt[i] & PTE_R) ? 'R' : '-',
                       (pt[i] & PTE_W) ? 'W' : '-',
                       (pt[i] & PTE_X) ? 'X' : '-',
                       (pt[i] & PTE_U) ? 'U' : '-');
            }
        }
    }
}
// kernel/mm/vm.c （追加在文件末尾）

// 全局内核页表
pagetable_t kernel_pagetable;

// 初始化内核页表
void kvminit(void) {
    printf("kvminit: creating kernel page table...\n");

    kernel_pagetable = create_pagetable();
    if (kernel_pagetable == 0) {
        printf("kvminit: failed to create page table\n");
        return;
    }

    extern char etext[], end[];


    // 映射内核代码段（R+X）
    for (uint64_t va = KERNBASE; va < (uint64_t)etext; va += PGSIZE) {
        uint64_t pa = va;
        if (map_page(kernel_pagetable, va, pa, PTE_R | PTE_X) < 0) {
            printf("kvminit: failed to map code at 0x%lx\n", va);
            return;
        }
    }

    // 映射内核数据段（R+W）
    for (uint64_t va = (uint64_t)etext; va < (uint64_t)end; va += PGSIZE) {
        uint64_t pa = va;
        if (map_page(kernel_pagetable, va, pa, PTE_R | PTE_W) < 0) {
            printf("kvminit: failed to map data at 0x%lx\n", va);
            return;
        }
    }

    // 映射栈空间（预留 4MB）
    for (uint64_t va = 0x87c00000; va < 0x88000000; va += PGSIZE) {
        uint64_t pa = va;
        if (map_page(kernel_pagetable, va, pa, PTE_R | PTE_W) < 0) {
            printf("kvminit: failed to map stack at 0x%lx\n", va);
            return;
        }
    }

    // 映射 UART 设备（R+W）
    if (map_page(kernel_pagetable, UART0, UART0, PTE_R | PTE_W) < 0) {
        printf("kvminit: failed to map UART\n");
        return;
    }

    printf("kvminit: kernel page table created successfully\n");
}

// 在当前 hart 上启用页表
void kvminithart(void) {
    printf("kvminithart: enabling paging...\n");
    w_satp(MAKE_SATP(kernel_pagetable));
    sfence_vma();
    printf("kvminithart: paging enabled\n");
}


