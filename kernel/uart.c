#define UART0_BASE 0x10000000L
#define UART0_THR  (UART0_BASE + 0x00)
#define UART0_LSR  (UART0_BASE + 0x05)
#define LSR_THRE   (1 << 5)

void uart_putc(char c) {
    while ((*(volatile char*)UART0_LSR & LSR_THRE) == 0)
        ;
    *(volatile char*)UART0_THR = c;
}

void uart_puts(char *s) {
    while (*s) {
        uart_putc(*s);
        s++;
    }
}
