// include/syscall.h
#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#define SYS_getpid 1
#define SYS_fork   2
#define SYS_exit   3
#define SYS_wait   4
#define SYS_write  5

void syscall_dispatch(void);


#endif
