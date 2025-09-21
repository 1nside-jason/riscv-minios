void uart_puts(char *s);

void start() __attribute__((noreturn));

void start() {
    uart_puts("Hello OS\n");
    while(1);
}
