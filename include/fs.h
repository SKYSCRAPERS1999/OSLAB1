#ifndef __FS_H__
#define __FS_H__

#include <kernel.h>

#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define T_DIR  1   // Directory
#define T_FILE 2   // File

#define ROOTINO 1  // root i-number
#define FSSIZE 64  // size of file system in blocks
#define BSIZE 512  // block size
#define NDIRECT 12
#define NFILE 100

struct superblock{
    uint size;         // Size of file system image (blocks)
    uint nblocks;      // Number of data blocks
    uint ninodes;      // Number of inodes.
    uint inodestart;   // Block number of first inode block
    uint bmapstart;    // Block number of first free map block
};
struct filesystem { 
  superblock_t sb;
  uint8_t ibitmap[BSIZE];
  uint8_t bbitmap[BSIZE];
  uint8_t disk[BSIZE * FSSIZE];
  fsops_t ops;
};
struct inode {
  uint inum;          // Inode number
  short type;         // copy of disk inode
  short nlink;
  uint size;
  uint addrs[NDIRECT+1];
};
struct file {
  char readable;
  char writable;
  struct inode *ip;
  uint off, ref;
};
// Inodes per block.
#define IPB           (BSIZE / sizeof(struct inode))
// Block containing inode i
#define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)
// Bitmap bits per block
#define BPB           (BSIZE*8)
// Block of free map containing bit for block b
#define BBLOCK(b, sb) (b/BPB + sb.bmapstart)

#endif
