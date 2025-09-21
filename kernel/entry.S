    .globl _entry
_entry:
    # 调试：输出 'S' 表示启动开始
    li t0, 0x10000000       # UART0 THR
    li t1, 'S'
    sb t1, 0(t0)

    # 设置栈顶
    la sp, stack_top
    li t1, 'P'
    sb t1, 0(t0)            # 输出 'P' 表示栈设置完成

    # 清零 BSS 段
    la a0, _bss_start
    la a1, _bss_end
    li a2, 0
clear_loop:
    bge a0, a1, jump_main
    sb a2, 0(a0)
    addi a0, a0, 1
    j clear_loop

jump_main:
    call start

spin:
    j spin                  # 死循环

    .section .bss.stack
    .align 4
    .globl stack_bottom
stack_bottom:
    .space 4096             # 4KB 栈空间
    .globl stack_top
stack_top:
