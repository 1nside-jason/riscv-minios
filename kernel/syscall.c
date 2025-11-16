// kernel/syscall.c
#include "syscall.h"
#include "proc/proc.h"
#include "printf.h"
#include "uart.h"
#include "string.h"

// ============ RAMFS 模拟 ============
#define MAX_FILES 8
#define MAX_FILE_SIZE 4096
#define MAX_FILENAME 28

enum {
    FT_NONE = 0,
    FT_REG = 1,
};

struct file_data {
    char name[MAX_FILENAME];
    int type;
    int size;
    char data[MAX_FILE_SIZE];
};

struct open_file {
    int used;
    int fd;              // 文件描述符编号
    struct file_data *fp; // 指向文件数据
    int offset;          // 当前读写位置
};

static struct file_data files[MAX_FILES];
static struct open_file ofiles[MAX_FILES];
static int next_fd = 0;
static int initialized = 0;

static void fs_init_once(void) {
    if (initialized) return;
    for (int i = 0; i < MAX_FILES; i++) {
        files[i].type = FT_NONE;
    }
    for (int i = 0; i < MAX_FILES; i++) {
        ofiles[i].used = 0;
    }
    initialized = 1;
}

// 查找已存在的文件（按 name）
static struct file_data* find_file(const char *name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].type == FT_REG && strcmp(files[i].name, name) == 0) {
            return &files[i];
        }
    }
    return 0;
}

// 分配新文件
static struct file_data* alloc_file(const char *name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].type == FT_NONE) {
            strcpy(files[i].name, name);
            files[i].type = FT_REG;
            files[i].size = 0;
            return &files[i];
        }
    }
    return 0;
}

// 分配文件描述符
static struct open_file* alloc_fd(void) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (!ofiles[i].used) {
            ofiles[i].used = 1;
            ofiles[i].fd = next_fd++;
            return &ofiles[i];
        }
    }
    return 0;
}

// ============ 系统调用实现 ============

// 声明系统调用实现函数
int sys_getpid(void);
int sys_fork(void);
int sys_exit(void);
int sys_wait(void);
int sys_write(void);
int sys_open(void);
int sys_close(void);
int sys_read(void);
int sys_unlink(void);

// 系统调用分发表
static int (*syscalls[])(void) = {
    [SYS_getpid] = sys_getpid,
    [SYS_fork]   = sys_fork,
    [SYS_exit]   = sys_exit,
    [SYS_wait]   = sys_wait,
    [SYS_write]  = sys_write,
    [SYS_open]   = sys_open,
    [SYS_close]  = sys_close,
    [SYS_read]   = sys_read,
    [SYS_unlink] = sys_unlink,
};

// 参数提取：从 trapframe 获取 a0-a6
static int argint(int n, int *ip) {
    struct proc *p = current_proc;
    if (!p) return -1;
    switch (n) {
        case 0: *ip = p->trapframe->a0; break;
        case 1: *ip = p->trapframe->a1; break;
        case 2: *ip = p->trapframe->a2; break;
        default: return -1;
    }
    return 0;
}

static uint64_t argaddr(int n) {
    struct proc *p = current_proc;
    if (!p) return 0;
    switch (n) {
        case 0: return p->trapframe->a0;
        case 1: return p->trapframe->a1;
        case 2: return p->trapframe->a2;
        default: return 0;
    }
}

static int argstr(int n, char *buf, int max) {
    uint64_t addr = argaddr(n);
    if (addr == 0 || max <= 0) return -1;
    char *s = (char*)addr;
    int i = 0;
    while (i < max - 1 && s[i] != '\0') {
        buf[i] = s[i];
        i++;
    }
    buf[i] = '\0';
    return 0;
}

// 系统调用分发
void syscall_dispatch(void) {
    struct proc *p = current_proc;
    if (!p) return;

    int num = p->trapframe->a7;  // 系统调用号在 a7
    if (num > 0 && num < sizeof(syscalls)/sizeof(syscalls[0]) && syscalls[num]) {
        int ret = syscalls[num]();
        p->trapframe->a0 = ret;  // 返回值放 a0
    } else {
        printf("Unknown syscall %d\n", num);
        p->trapframe->a0 = -1;
    }
}

// ========== 已有实现 ==========
int sys_getpid(void) {
    return current_proc ? current_proc->pid : -1;
}

int sys_exit(void) {
    int status;
    argint(0, &status);
    if (current_proc) {
        exit_process(status);
    }
    return 0;
}

int sys_fork(void) {
    return -1; // 未实现
}

int sys_wait(void) {
    int *status;
    if (argint(0, (int*)&status) < 0) return -1;
    return wait_process(status);
}

int sys_write(void) {
    int fd, count;
    uint64_t buf;
    argint(0, &fd);
    buf = argaddr(1);
    argint(2, &count);

    if (fd != 1) return -1;  // 只支持 stdout (fd=1)
    if (buf == 0 || count < 0) return -1;

    char *s = (char*)buf;
    for (int i = 0; i < count; i++) {
        uart_putc(s[i]);
    }
    return count;
}

// ========== 新增实现 ==========
int sys_open(void) {
    fs_init_once();
    char path[64];
    int flags;
    if (argstr(0, path, sizeof(path)) < 0) return -1;
    argint(1, &flags);

    // 只支持根目录文件：/filename
    if (path[0] != '/') return -1;
    const char *name = path + 1;
    if (strchr(name, '/')) return -1; // 不支持子目录

    struct file_data *fp = find_file(name);
    if (flags & 1) { // O_CREATE
        if (fp) return -1; // 已存在
        fp = alloc_file(name);
        if (!fp) return -1;
    } else {
        if (!fp) return -1; // 文件不存在
    }

    struct open_file *of = alloc_fd();
    if (!of) return -1;
    of->fp = fp;
    of->offset = 0;
    return of->fd;
}

int sys_close(void) {
    int fd;
    if (argint(0, &fd) < 0) {  // ✅ 从 a0 提取 fd
        return -1;
    }
    if (fd < 0 || fd >= MAX_FILES) {
        return -1;
    }
    for (int i = 0; i < MAX_FILES; i++) {
        if (ofiles[i].used && ofiles[i].fd == fd) {
            ofiles[i].used = 0;
            return 0;
        }
    }
    return -1;  // fd 未打开
}

int sys_read(void) {
    int fd, count;
    uint64_t buf;
    argint(0, &fd);
    buf = argaddr(1);
    argint(2, &count);

    if (buf == 0 || count < 0) return -1;
    if (fd < 0 || fd >= MAX_FILES) return -1;

    struct open_file *of = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (ofiles[i].used && ofiles[i].fd == fd) {
            of = &ofiles[i];
            break;
        }
    }
    if (!of) return -1;

    if (of->offset >= of->fp->size) return 0; // EOF

    int n = count;
    if (of->offset + n > of->fp->size) {
        n = of->fp->size - of->offset;
    }
    memcpy((void*)buf, of->fp->data + of->offset, n);
    of->offset += n;
    return n;
}

int sys_unlink(void) {
    fs_init_once();
    char path[64];
    if (argstr(0, path, sizeof(path)) < 0) return -1;

    if (path[0] != '/') return -1;
    const char *name = path + 1;
    if (strchr(name, '/')) return -1;

    struct file_data *fp = find_file(name);
    if (!fp) return -1;

    // 标记为未使用
    fp->type = FT_NONE;
    return 0;
}
