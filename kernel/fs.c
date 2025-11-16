// kernel/fs.c
#include "fs.h"
#include "proc.h"
#include "printf.h"

#define NINODE 50
#define ROOT_INUM 1

struct inode inodes[NINODE];
struct inode root_inode;
int next_inum = 2;

struct file ftable[MAX_OPEN_FILES];

void fs_init(void) {
    // 初始化根目录
    root_inode.inum = ROOT_INUM;
    root_inode.type = FT_DIR;
    root_inode.nlink = 1;
    root_inode.size = 0;

    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        ftable[i].type = FD_NONE;
    }

    printf("fs_init: RAMFS initialized\n");
}

// 分配新 inode
struct inode* ialloc(short type) {
    for (int i = 0; i < NINODE; i++) {
        if (inodes[i].inum == 0) {
            inodes[i].inum = next_inum++;
            inodes[i].type = type;
            inodes[i].nlink = 1;
            inodes[i].size = 0;
            return &inodes[i];
        }
    }
    return 0;
}

// 查找路径对应的 inode（只支持根目录下文件）
struct inode* namei(char *path) {
    if (path[0] != '/') return 0;
    if (path[1] == 0) return &root_inode;  // 根目录

    // 简化：只支持 "/filename"，不支持子目录
    char *name = path + 1;
    if (strchr(name, '/')) return 0;  // 不支持嵌套路径

    // 在根目录中查找
    for (int i = 0; i < root_inode.size; ) {
        struct dirent *de = (struct dirent*)(root_inode.data + i);
        if (de->inum != 0 && strcmp(de->name, name) == 0) {
            // 找到对应 inode
            return &inodes[de->inum - 2];  // inum 从 2 开始
        }
        i += sizeof(struct dirent);
    }
    return 0;  // 未找到
}

// 在目录中创建链接
int dirlink(struct inode *dp, char *name, uint inum) {
    if (dp->type != FT_DIR) return -1;
    if (dp->size + sizeof(struct dirent) > MAX_FILE_SIZE) return -1;

    struct dirent de;
    de.inum = inum;
    strcpy(de.name, name);

    memcpy(dp->data + dp->size, &de, sizeof(de));
    dp->size += sizeof(struct dirent);
    return 0;
}

// 系统调用实现
int sys_open(void) {
    char path[64];
    int flags;
    argstr(0, path, sizeof(path));
    argint(1, &flags);

    // 只支持 O_CREATE | O_RDWR | O_RDONLY
    if (flags & 1) {  // O_CREATE
        struct inode *ip = namei(path);
        if (ip) return -1;  // 已存在

        ip = ialloc(FT_REG);
        if (!ip) return -1;

        // 添加到根目录
        dirlink(&root_inode, path + 1, ip->inum);
        ip->nlink = 1;
    } else {
        struct inode *ip = namei(path);
        if (!ip) return -1;
    }

    // 分配文件描述符
    for (int fd = 0; fd < MAX_OPEN_FILES; fd++) {
        if (ftable[fd].type == FD_NONE) {
            ftable[fd].type = FD_INODE;
            ftable[fd].ip = namei(path);
            ftable[fd].off = 0;
            ftable[fd].ref = 1;
            return fd;
        }
    }
    return -1;
}

int sys_close(void) {
    int fd;
    argint(0, &fd);
    if (fd < 0 || fd >= MAX_OPEN_FILES || ftable[fd].type == FD_NONE)
        return -1;
    ftable[fd].type = FD_NONE;
    ftable[fd].ref = 0;
    return 0;
}

int sys_read(void) {
    int fd, n;
    uint64_t buf;
    argint(0, &fd);
    buf = argaddr(1);
    argint(2, &n);

    if (fd < 0 || fd >= MAX_OPEN_FILES || ftable[fd].type == FD_NONE)
        return -1;

    struct file *f = &ftable[fd];
    struct inode *ip = f->ip;
    if (f->off + n > ip->size) {
        n = ip->size - f->off;
        if (n <= 0) return 0;
    }

    memcpy((void*)buf, ip->data + f->off, n);
    f->off += n;
    return n;
}

int sys_write(void) {
    int fd, n;
    uint64_t buf;
    argint(0, &fd);
    buf = argaddr(1);
    argint(2, &n);

    if (fd < 0 || fd >= MAX_OPEN_FILES || ftable[fd].type == FD_NONE)
        return -1;

    struct file *f = &ftable[fd];
    struct inode *ip = f->ip;
    if (f->off + n > MAX_FILE_SIZE) return -1;

    memcpy(ip->data + f->off, (void*)buf, n);
    if (f->off + n > ip->size) ip->size = f->off + n;
    f->off += n;
    return n;
}

int sys_unlink(void) {
    char path[64];
    argstr(0, path, sizeof(path));

    struct inode *ip = namei(path);
    if (!ip) return -1;

    // 从根目录中移除（简化：只支持根目录）
    char *name = path + 1;
    for (int i = 0; i < root_inode.size; ) {
        struct dirent *de = (struct dirent*)(root_inode.data + i);
        if (de->inum == ip->inum && strcmp(de->name, name) == 0) {
            de->inum = 0;  // 标记为空
            // 释放 inode（简化）
            ip->inum = 0;
            return 0;
        }
        i += sizeof(struct dirent);
    }
    return -1;
}
