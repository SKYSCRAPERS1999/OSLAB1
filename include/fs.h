#ifndef __FS_H__
#define __FS_H__

#include <kernel.h>

#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002

#define KVFS  0x000
#define PROCFS  0x001
#define DEVFS    0x002

#define T_FILE 0x000
#define T_DIR 0x001

#define ROOTINO 1  // root i-number
#define FSSIZE 64  // size of file system in blocks
#define BSIZE 512  // block size
#define NDIRECT 12
#define NINODES 64
#define NFILE 64
#define MAXFILENUM 128
#define MAXDATASZ 4096

struct fsops {
    void (*init)(filesystem_t *fs, const char *name, inode_t *dev);
    inode_t *(*lookup)(filesystem_t *fs, const char *path, int flags);
    int (*close)(inode_t *inode);
};

struct fileops {
    int (*open)(inode_t *inode, file_t *file, int flags);
    ssize_t (*read)(inode_t *inode, file_t *file, char *buf, size_t size);
    ssize_t (*write)(inode_t *inode, file_t *file, const char *buf, size_t size);
    off_t (*lseek)(inode_t *inode, file_t *file, off_t offset, int whence);
};

struct filesystem { 
  int type;
  fsops_t* ops;
  inode_t* inodes[NINODES];
};

struct inode {
  int ref;        // used
  int inum;      // Inode number
  int type;     
  int size;
  file_t* files[NDIRECT];
  char data[MAXDATASZ];
};

struct file {
  int fd;
  int ref; // used
  int mode;
  int off;
  inode_t* f_inode;
};

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct inode))
// Block containing inode i
#define IBLOCK(i, sb)     ((i) / IPB)

#endif

