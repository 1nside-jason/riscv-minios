# Makefile for riscv-minios
CC = riscv64-linux-gnu-gcc
LD = riscv64-linux-gnu-ld
OBJCOPY = riscv64-linux-gnu-objcopy

CFLAGS = -Wall -Werror -O2 -fno-common -fno-builtin -nostdlib -mcmodel=medany -I./include
LDFLAGS = -T kernel/kernel.ld -nostdlib

# 编译 usys.S（作为用户代码，但链接到内核）
user/usys.o: user/usys.S
	$(CC) $(CFLAGS) -c $< -o $@

OBJS = kernel/entry.o kernel/main.o kernel/uart.o kernel/printf.o kernel/console.o \
       kernel/mm/pmm.o kernel/mm/vm.o  \
       kernel/trap/trap.o kernel/trap/trapvec.o \
       kernel/proc/proc.o kernel/proc/swtch.o \
       kernel/syscall.o user/usys.o


kernel.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

kernel/entry.o: kernel/entry.S
	$(CC) $(CFLAGS) -c $< -o $@

kernel/main.o: kernel/main.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel/uart.o: kernel/uart.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel/printf.o: kernel/printf.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel/console.o: kernel/console.c
	$(CC) $(CFLAGS) -c $< -o $@
kernel/trap/trap.o: kernel/trap/trap.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel/trap/trapvec.o: kernel/trap/trapvec.S
	$(CC) $(CFLAGS) -c $< -o $@

kernel/proc/proc.o: kernel/proc/proc.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel/proc/swtch.o: kernel/proc/swtch.S
	$(CC) $(CFLAGS) -c $< -o $@

run: kernel.elf
	qemu-system-riscv64 -machine virt -bios none -kernel kernel.elf -nographic -serial mon:stdio

debug: kernel.elf
	qemu-system-riscv64 -machine virt -bios none -kernel kernel.elf -nographic -serial mon:stdio -S -gdb tcp::1234

dump-dtb:
	qemu-system-riscv64 -machine virt,dumpdtb=virt.dtb -nographic
	dtc -I dtb -O dts virt.dtb > virt.dts
	@grep -A5 -B5 -E "uart|memory" virt.dts

clean:
	rm -f kernel.elf $(OBJS) virt.dtb virt.dts

.PHONY: run debug dump-dtb clean
