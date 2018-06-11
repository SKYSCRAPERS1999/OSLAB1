#ifndef __FS_H__
#define __FS_H__

#include <kernel.h>

#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define T_DIR  1   // Directory
#define T_FILE 2   // File
#define T_DEV  3   // Device

#define ROOTINO 1  // root i-number
#define FSSIZE 64  // size of file system in blocks
#define BSIZE 512  // block size
#define NDIRECT 12

int nbitmap = FSSIZE/(BSIZE*8) + 1;
int ninodeblocks = NINODES / IPB + 1;
int nmeta;    // Number of meta blocks (boot, sb, nlog, inode, bitmap)
int nblocks;  // Number of data blocks

typedef struct superblock{
    uint size;         // Size of file system image (blocks)
    uint nblocks;      // Number of data blocks
    uint ninodes;      // Number of inodes.
    uint inodestart;   // Block number of first inode block
    uint bmapstart;    // Block number of first free map block
} superblock_t;

struct filesystem { 
  superblock_t sb;
  fsops_t ops;
};

struct inode {
  uint dev;           // Device number
  uint inum;          // Inode number
  struct sleeplock lock; // protects everything below here
  short type;         // copy of disk inode
  short major;
  short minor;
  short nlink;
  uint size;
  uint addrs[NDIRECT+1];
}

struct file {
  enum { FD_NONE, FD_PIPE, FD_INODE } type;
  char readable;
  char writable;
  struct inode *ip;
  uint off;
};

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct inode))
// Block containing inode i
#define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)
// Bitmap bits per block
#define BPB           (BSIZE*8)
// Block of free map containing bit for block b
#define BBLOCK(b, sb) (b/BPB + sb.bmapstart)
// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14
struct dirent {
  ushort inum;
  char name[DIRSIZ];
};

filesystem_t *create_kvfs() {
  filesystem_t *fs = (filesystem_t *)pmm->alloc(sizeof(filesystem_t));
  if (!fs) panic("fs allocation failed");
  fs->ops = &kvfs_ops; // 你为kvfs定义的fsops_t，包含函数的实现
  fs->ops->init(fs, "procfs", NULL);
  return fs;
}

#endif
