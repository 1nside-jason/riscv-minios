// include/fs.h
#ifndef __FS_H__
#define __FS_H__

#include "riscv.h"

#define MAX_OPEN_FILES 16
#define MAX_FILE_SIZE  (64 * 1024)  // 64KB
#define MAX_FILENAME   28

// 文件类型
#define FT_REG  1  // 普通文件
#define FT_DIR  2  // 目录

struct inode {
    int inum;           // inode 编号
    short type;         // FT_REG 或 FT_DIR
    short nlink;        // 硬链接数
    uint size;          // 文件大小
    char data[MAX_FILE_SIZE];  // 文件内容（RAMFS 简化）
};

struct dirent {
    ushort inum;
    char name[MAX_FILENAME];
};

// 打开文件描述符
struct file {
    enum { FD_NONE, FD_INODE } type;
    int ref;            // 引用计数
    struct inode *ip;   // 指向 inode
    uint off;           // 读写偏移
};

extern struct file ftable[MAX_OPEN_FILES];
extern struct inode root_inode;
extern int next_inum;

void fs_init(void);
struct inode* ialloc(short type);
void iupdate(struct inode *ip);
struct inode* namei(char *path);
int dirlink(struct inode *dp, char *name, uint inum);
int dirlookup(struct inode *dp, char *name, uint *poff);

// 文件操作
int sys_open(void);
int sys_close(void);
int sys_read(void);
int sys_write(void);
int sys_unlink(void);

#endif
