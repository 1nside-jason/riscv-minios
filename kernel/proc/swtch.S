# kernel/proc/swtch.S
    .globl swtch
swtch:
    # 保存当前上下文
    sd ra, 0(a0)
    sd sp, 8(a0)
    # 可选：保存更多 callee-saved 寄存器（s0-s11）

    # 恢复新上下文
    ld ra, 0(a1)
    ld sp, 8(a1)

    ret
