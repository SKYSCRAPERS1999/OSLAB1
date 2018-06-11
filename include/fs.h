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
#define NINODES 100
#define NFILE 100

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

struct superblock{
    int type;
    int size;         // Size of file system image (blocks)
    int nblocks;      // Number of data blocks
    int ninodes;      // Number of inodes.
    int imap_start;
    int bitmap_start;
    int inode_start;
    int datablk_start;
};

struct filesystem { 
  superblock_t sb;
  uint8_t blocks[FSSIZE*BSIZE];
  fsops_t* ops;
};

struct inode {
  int ref;        // used
  int inum;      // Inode number
  int type;     
  int size;
  int addrs[NDIRECT+1];
};

struct file {
  int id;
  int ref; // used
  int readable;
  int writable;
  inode_t *ip;
  int off;
};

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct inode))
// Block containing inode i
#define IBLOCK(i, sb)     ((i) / IPB)

#endif

