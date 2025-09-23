// include/console.h
#ifndef __CONSOLE_H__
#define __CONSOLE_H__

void console_putc(char c);
void console_puts(const char *s);

// ✅ 添加以下声明！
void clear_screen(void);
void goto_xy(int x, int y);
void set_color(int fg);
void reset_color(void);

#endif
