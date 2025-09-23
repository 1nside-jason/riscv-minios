// kernel/printf.c
#include "console.h"
#include <stdarg.h>  // 可变参数

// 前向声明
static void printint(int xx, int base, int sign);
static void printptr(unsigned long long x);

// 输出一个字符（供 printf 内部调用）
static void putc(char c) {
    console_putc(c);
}

// 输出字符串
static void puts(const char *s) {
    console_puts(s);
}

// 核心 printf 实现
int printf(const char *fmt, ...) {
    va_list ap;
    int i, c;
    char *s;
    int num;

    va_start(ap, fmt);
    for (i = 0; fmt[i]; i++) {
        c = fmt[i];

        if (c != '%') {
            putc(c);
            continue;
        }

        c = fmt[++i]; // 取 % 后的字符
        switch (c) {
        case 'd': // 有符号十进制
            num = va_arg(ap, int);
            printint(num, 10, 1);
            break;
        case 'x': // 无符号十六进制
            num = va_arg(ap, int);
            printint(num, 16, 0);
            break;
        case 'p': // 指针（十六进制）
            printptr(va_arg(ap, unsigned long long));
            break;
        case 's': // 字符串
            s = va_arg(ap, char*);
            puts(s);
            break;
        case 'c': // 字符
            putc(va_arg(ap, int));
            break;
        case '%': // 字面 %
            putc('%');
            break;
        default:  // 未知格式符，原样输出
            putc('%');
            putc(c);
            break;
        }
    }
    va_end(ap);
    return 0; // 简化，不返回字符数
}

// 打印整数（支持负数、不同进制）
static void printint(int xx, int base, int sign) {
    static char digits[] = "0123456789abcdef";
    char buf[16];  // 足够存 int
    int i = 0;
    int neg = 0;
    unsigned int x;

    // 处理负数
    if (sign && xx < 0) {
        neg = 1;
        x = -xx;  // 注意：-INT_MIN 会溢出！见下方说明
    } else {
        x = xx;
    }

    // 特殊处理 INT_MIN
    if (sign && xx == 0x80000000) {
        puts("-2147483648");
        return;
    }

    // 数字转字符串（逆序存入 buf）
    do {
        buf[i++] = digits[x % base];
    } while ((x /= base) != 0);

    // 输出负号
    if (neg) {
        putc('-');
    }

    // 逆序输出
    while (--i >= 0) {
        putc(buf[i]);
    }
}

// 打印指针（16进制，带 0x 前缀）
static void printptr(unsigned long long x) {
    putc('0');
    putc('x');
    // 强制转为 32 位（RISC-V 32 位地址）
    printint((int)x, 16, 0);
}
