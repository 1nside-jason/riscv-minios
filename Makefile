# Makefile for riscv-minios
CC = riscv64-linux-gnu-gcc
LD = riscv64-linux-gnu-ld
OBJCOPY = riscv64-linux-gnu-objcopy

CFLAGS = -Wall -Werror -O2 -fno-common -fno-builtin -nostdlib -mcmodel=medany
LDFLAGS = -T kernel/kernel.ld -nostdlib

OBJS = kernel/entry.o kernel/main.o kernel/uart.o

kernel.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

kernel/entry.o: kernel/entry.S
	$(CC) $(CFLAGS) -c $< -o $@

kernel/main.o: kernel/main.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel/uart.o: kernel/uart.c
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
